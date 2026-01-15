#define _GNU_SOURCE

#include "rope_node.h"
#include <assert.h>
#include <cextras/unicode.h>
#include <rope.h>
#include <string.h>

static void
dummy_callback(struct Rope *rope, struct RopeCursor *cursor, void *user) {
	(void)rope;
	(void)cursor;
	(void)user;
}

static struct RopeCursor **
cursor_detach(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = &cursor->rope->last_cursor;
	struct RopeCursor *c = *ptr;

	for (; c->index > cursor->index; c = c->prev) {
		ptr = &c->prev;
	}
	struct RopeCursor **insert_ptr = ptr;
	for (; c != cursor; c = c->prev) {
		ptr = &c->prev;
	}
	*ptr = cursor->prev;
	return insert_ptr;
}

static void
cursor_attach_at(struct RopeCursor *cursor, struct RopeCursor **ptr) {
	cursor->prev = *ptr;
	*ptr = cursor;
}

static void
cursor_attach(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = &cursor->rope->last_cursor;
	struct RopeCursor *c = *ptr;

	for (; c && c->index > cursor->index; c = c->prev) {
		ptr = &c->prev;
	}
	cursor_attach_at(cursor, ptr);
}

static void
cursor_update_location(struct RopeCursor *cursor) {
	rope_char_index_t index = cursor->index;
	rope_index_t byte_index = 0;
	struct RopeNode *node = rope_node_find_char(
			cursor->rope->root, index, /*tags=*/0, &byte_index);
	size_t size = 0;
	const uint8_t *value;

	cursor->line = 0;
	cursor->column = 0;
	do {
		value = rope_node_value(node, &size);
		size = CX_MIN(size, byte_index);
		byte_index = SIZE_MAX;

		const uint8_t *p = value;
		while ((p = memchr(p, '\n', size))) {
			cursor->line++;
			p++;
			size -= (p - value);
			value = p;
		}

		if (cursor->line == 0) {
			cursor->column += cx_utf8_clen(value, size);
		}
	} while ((node = rope_node_prev(node)));
}

static void
cursor_bubble_up(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = cursor_detach(cursor);
	cursor_attach_at(cursor, ptr);
}

static int
cursor_update(struct RopeCursor *cursor) {
	if (cursor->prev && cursor->prev->index <= cursor->index) {
		cursor_bubble_up(cursor);
	} else {
		cursor_detach(cursor);
		cursor_attach(cursor);
	}
	cursor_update_location(cursor);
	return 0;
}

static void
cursor_damaged(
		struct RopeCursor *cursor, rope_char_index_t lower_bound,
		off_t offset) {
	struct RopeCursor *last = cursor->rope->last_cursor;
	struct RopeCursor *barrier = cursor->prev;
	struct RopeCursor *c = last;

	for (; c != barrier; c = c->prev) {
		if (offset < 0 && c->index < (size_t)-offset) {
			// underflow
			break;
		}
		c->index += offset;
		if (c->index < lower_bound) {
			break;
		}
	}
	for (; c != barrier; c = c->prev) {
		c->index = lower_bound;
	}
	for (c = last; c != barrier; c = c->prev) {
		c->callback(c->rope, c, c->userdata);
	}
}

int
rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope) {
	cursor->callback = dummy_callback;
	cursor->userdata = NULL;
	cursor->rope = rope;
	cursor->index = 0;
	cursor->prev = NULL;
	cursor_attach(cursor);
	return 0;
}

int
rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata) {
	if (callback == NULL) {
		callback = dummy_callback;
	}
	cursor->callback = callback;
	cursor->userdata = userdata;
	return 0;
}

int
rope_cursor_move_to(
		struct RopeCursor *cursor, rope_index_t line,
		rope_char_index_t column) {
	struct Rope *rope = cursor->rope;
	rope_index_t byte_index = 0;
	struct RopeNode *node =
			rope_node_find(rope->root, line, column, /*tags=*/0, &byte_index);
	if (node == NULL) {
		return -1;
	}
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	// TODO: make sure column doesn't wrap lines.
	cursor->index = cx_utf8_clen(value, byte_index);
	while ((node = rope_node_prev(node))) {
		cursor->index += rope_node_char_size(node);
	}

	return cursor_update(cursor);
}

bool
rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second) {
	if (first->index < second->index) {
		return true;
	}

	do {
		if (first->index != second->index) {
			return false;
		} else if (first == second) {
			return true;
		}
	} while ((second = second->prev));

	return false;
}

int
rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t index, uint64_t tags) {
	struct Rope *rope = cursor->rope;
	if (tags != 0) {
		rope_index_t global_index = 0;
		struct RopeNode *node = rope_first(rope);
		do {
			const size_t char_size = rope_node_char_size(node);
			if (rope_node_match_tags(node, tags)) {
				if (index < char_size) {
					break;
				}
				index -= char_size;
			}
			global_index += char_size;
		} while ((node = rope_node_next(node)));
		index += global_index;
	}
	if (index > rope_node_char_size(rope->root)) {
		return -1;
	}
	cursor->index = index;
	return cursor_update(cursor);
}

int
rope_cursor_move(struct RopeCursor *cursor, off_t offset) {
	// TODO: instead of this shenanigans, just check for addition
	// overflow.
	if (offset < 0 && (rope_char_index_t)-offset > cursor->index) {
		cursor->index = 0;
	} else {
		cursor->index = cursor->index + offset;
	}
	return cursor_update(cursor);
}

int
rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags) {
	int rv = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;

	if (byte_size == 0) {
		return 0;
	}
	size_t char_count = cx_utf8_clen(data, byte_size);

	rope_index_t cursor_index = cursor->index;

	rope_byte_index_t insert_at_byte = 0;
	struct RopeNode *insert_at = rope_node_find_char(
			rope->root, cursor_index, /*tags=*/0, &insert_at_byte);

	rv = rope_node_split(insert_at, &rope->pool, insert_at_byte, &left, &right);

	if (left) {
		rv = rope_node_insert_right(left, data, byte_size, tags, &rope->pool);
	} else {
		rv = rope_node_insert_left(right, data, byte_size, tags, &rope->pool);
	}
	if (rv < 0) {
		goto out;
	}

	cursor_bubble_up(cursor);
	cursor_damaged(cursor, 0, char_count);
out:
	return rv;
}

int
rope_cursor_insert_str(
		struct RopeCursor *cursor, const char *str, uint64_t tags) {
	return rope_cursor_insert(cursor, (const uint8_t *)str, strlen(str), tags);
}


int
rope_cursor_delete(struct RopeCursor *cursor, size_t char_count) {
	rope_char_index_t index = cursor->index;
	rope_index_t byte_index = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *node =
			rope_node_find_char(rope->root, index, /*tags=*/0, &byte_index);
	size_t remaining = char_count;

	if (remaining == 0) {
		return 0;
	}

	if (rope_node_byte_size(node) == byte_index) {
		node = rope_node_next(node);
		byte_index = 0;
	} else if (byte_index != 0) {
		rope_node_split(node, &rope->pool, byte_index, NULL, &node);
		byte_index = 0;
	}
	assert(node != NULL);

	while(node) {
		size_t char_size = rope_node_char_size(node);
		if (char_size > remaining) {
			break;
		}
		remaining -= char_size;
		node = rope_node_delete_and_next(node, &rope->pool);
	}
	//node = rope_node_delete_while(
	//		node, &rope->pool, cursor_while_delete_cb, &remaining);

	if (node && remaining > 0) {
		size_t size = 0;
		const uint8_t *value = rope_node_value(node, &size);
		byte_index = cx_utf8_bidx(value, size, remaining);
		rope_node_split(node, &rope->pool, byte_index, &node, NULL);
		rope_node_delete(node, &rope->pool);
		remaining = 0;
	}
	assert(remaining == 0);
	cursor_bubble_up(cursor);
	cursor_damaged(cursor, cursor->index, -(off_t)char_count);

	return 0;
}

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index) {
	rope_char_index_t char_index = cursor->index;

	struct RopeNode *node = rope_node_find_char(
			cursor->rope->root, char_index, /*tags=*/0, byte_index);
	if (node == NULL) {
		return NULL;
	}

	return node;
}

int
rope_cursor_cleanup(struct RopeCursor *cursor) {
	if (cursor->rope == NULL) {
		return 0;
	}
	cursor_detach(cursor);
	cursor->prev = NULL;
	return 0;
}
