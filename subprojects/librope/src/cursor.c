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

		if (byte_index < size) {
			size = byte_index;
			byte_index = SIZE_MAX;
		}

		const uint8_t *new_line = memrchr(value, ROPE_NEWLINE, size);
		if (new_line) {
			cursor->column +=
					cx_utf8_csize(&new_line[1], size - (new_line - value) - 1);
			break;
		} else {
			cursor->column += rope_node_char_size(node);
		}
	} while ((node = rope_node_prev(node)));

	if (!node) {
		return;
	}

	do {
		cursor->line += rope_node_new_lines(node);
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

static int
cursor_insert_node(
		struct RopeCursor *cursor, struct RopeNode *insert_at,
		const uint8_t *data, size_t byte_size, uint64_t tags,
		enum RopeDirection which) {
	int rv = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *node = rope_pool_get(&rope->pool);
	if (node == NULL) {
		rv = -1;
		goto out;
	}

	rv = rope_node_set_value(node, data, byte_size);
	if (rv < 0) {
		goto out;
	}
	rope_node_set_tags(node, tags);

	struct RopeNode *neighbour = rope_node_neighbour(insert_at, which);
	if (neighbour && rope_node_depth(neighbour) < rope_node_depth(insert_at)) {
		insert_at = neighbour;
		which = !which;
	}

	rv = rope_node_insert(insert_at, node, &rope->pool, which);
	if (rv < 0) {
		goto out;
	}

	node = NULL;
out:
	rope_node_free(node, &rope->pool);
	return rv;
}

static int
cursor_insert_right(
		struct RopeCursor *cursor, struct RopeNode *insert_at,
		const uint8_t *data, size_t byte_size, uint64_t tags) {
	int rv = -1;
	bool inserted = false;

	if (rope_node_byte_size(insert_at) == 0) {
		rope_node_set_tags(insert_at, tags);
		rv = rope_node_set_value(insert_at, data, byte_size);
	} else if (rope_node_tags(insert_at) == tags) {
		rv = rope_node_append_value(insert_at, data, byte_size);
	}
	inserted = rv == 0;

	rv = 0;
	if (!inserted) {
		rv = cursor_insert_node(
				cursor, insert_at, data, byte_size, tags, ROPE_RIGHT);
	}

	return rv;
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
		rv = cursor_insert_right(cursor, left, data, byte_size, tags);
	} else {
		struct RopeNode *neighbour = rope_node_prev(right);
		if (neighbour) {
			rv = cursor_insert_right(cursor, neighbour, data, byte_size, tags);
		} else {
			rv = cursor_insert_node(
					cursor, right, data, byte_size, tags, ROPE_LEFT);
		}
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

#if 1
static bool
cursor_while_delete_cb(const struct RopeNode *node, void *userdata) {
	size_t *remaining = userdata;
	size_t char_size = rope_node_char_size(node);
	if (*remaining >= char_size) {
		*remaining -= char_size;
		return true;
	} else {
		return false;
	}
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

	node = rope_node_delete_while(
			node, &rope->pool, cursor_while_delete_cb, &remaining);

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
#else
int
rope_cursor_delete(struct RopeCursor *cursor, size_t char_count) {
	rope_char_index_t index = cursor->index;
	rope_index_t byte_index = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *node =
			rope_node_find_char(rope->root, index, /*tags=*/0, &byte_index);
	size_t remaining = char_count;
	size_t deleted = 0;
	rope_char_index_t start_index = index;

	if (remaining == 0) {
		return 0;
	}

	if (rope_node_byte_size(node) == byte_index) {
		node = rope_node_next(node);
		start_index = index = 0;
		assert(node != NULL);
	}

	if (index != 0) {
		rope_node_split(node, &rope->pool, byte_index, NULL, &node);
	}
	while (node && remaining > 0 && rope_node_char_size(node) <= remaining) {
		size_t node_size = rope_node_char_size(node);
		remaining -= node_size;
		deleted += node_size;
		rope_node_delete(node, &rope->pool);
		if (remaining == 0) {
			node = NULL;
		} else {
			rope_byte_index_t refreshed = 0;
			node = rope_node_find_char(
					rope->root, start_index, /*tags=*/0, &refreshed);
		}
	}
	if (node && remaining > 0) {
		size_t size = 0;
		const uint8_t *value = rope_node_value(node, &size);
		rope_index_t byte_index = cx_utf8_bidx(value, size, remaining);
		rope_node_split(node, &rope->pool, byte_index, &node, NULL);
		rope_node_delete(node, &rope->pool);
		deleted += remaining;
		remaining = 0;
	}
	assert(remaining == 0);
	cursor_bubble_up(cursor);
	cursor_damaged(cursor, cursor->index, -(off_t)deleted);

	return 0;
}
#endif

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

int32_t
rope_cursor_codepoint(struct RopeCursor *cursor) {
	rope_char_index_t byte_index = 0;
	struct RopeNode *node = rope_cursor_node(cursor, &byte_index);
	if (node == NULL) {
		return -1;
	}

	size_t byte_size = 0;
	const uint8_t *value = rope_node_value(node, &byte_size);

	return cx_utf8_cp(&value[byte_index], byte_size - byte_index);
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
