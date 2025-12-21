#define _GNU_SOURCE
#include <assert.h>
#include <cextras/unicode.h>
#include <rope.h>
#include <string.h>

#define SEARCH(first, cond, res) \
	for (struct RopeCursor *c = (first); c && (cond); c = c->next) { \
		(res) = c; \
	}

static void
dummy_callback(struct Rope *rope, struct RopeCursor *cursor, void *user) {
	(void)rope;
	(void)cursor;
	(void)user;
}

static void
cursor_detach(struct RopeCursor *cursor) {
	assert(cursor->rope != NULL);
	struct RopeCursor *first = cursor->rope->cursors;

	struct RopeCursor *prev = NULL;
	SEARCH(first, c != cursor, prev);
	if (prev) {
		prev->next = cursor->next;
	} else if (first == cursor) {
		cursor->rope->cursors = cursor->next;
	}
}

static void
cursor_attach(struct RopeCursor *cursor) {
	struct RopeCursor *first = cursor->rope->cursors;
	struct RopeCursor *prev = NULL;
	SEARCH(first, c->index <= cursor->index, prev);
	if (prev) {
		cursor->next = prev->next;
		prev->next = cursor;
	} else {
		cursor->next = first;
		cursor->rope->cursors = cursor;
	}
}

static void
cursor_update_location(struct RopeCursor *cursor) {
	rope_char_index_t index = cursor->index;
	rope_index_t byte_index = 0;
	struct RopeNode *node = rope_node_find_char(
			cursor->rope->root, index, /*tags=*/0, &byte_index);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);

	cursor->line = 0;
	cursor->column = cx_utf8_bidx(value, size, byte_index);
	while (rope_node_prev(&node)) {
		if (cursor->line == 0) {
			value = rope_node_value(node, &size);
			const uint8_t *p = memrchr(value, ROPE_NEWLINE, size);
			if (p) {
				cursor->column += cx_utf8_bidx(value, size, p - value + 1);
			} else {
				cursor->column += rope_node_char_size(node);
			}
		}
		cursor->line += rope_node_new_lines(node);
	}
}

static int
cursor_update(struct RopeCursor *cursor) {
	cursor_detach(cursor);
	cursor_attach(cursor);
	cursor_update_location(cursor);
	return 0;
}

static void
cursor_move_back(struct RopeCursor *cursor) {
	// Reattach cursor. This makes sure, that the current cursor is the last one
	// with the index.
	cursor_detach(cursor);
	cursor_attach(cursor);
}

static void
cursor_damaged(
		struct RopeCursor *cursor, rope_char_index_t lower_bound,
		off_t offset) {
	if (cursor == NULL) {
		return;
	}

	//  First pass: Update indices and locations
	for (struct RopeCursor *c = cursor; c; c = c->next) {
		if (CX_ADD_OVERFLOW(c->index, offset, &c->index) ||
			c->index < lower_bound) {
			c->index = lower_bound;
		}
		// Note: We don't need to call update_cursor here as the order
		// is preserved. We do need to update the location though.
		cursor_update_location(c);
	}
	// Second pass: Let the cursor callback know
	for (struct RopeCursor *c = cursor; c; c = c->next) {
		c->callback(c->rope, c, c->userdata);
	}
}

int
rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope) {
	cursor->callback = dummy_callback;
	cursor->userdata = NULL;
	cursor->rope = rope;
	cursor->index = 0;
	cursor->next = NULL;
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
	while (rope_node_prev(&node)) {
		cursor->index += rope_node_char_size(node);
	}

	return cursor_update(cursor);
}

bool
rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second) {
	if (first->index < second->index) {
		return true;
	}

	while (first && first->index == second->index) {
		if (first == second) {
			return true;
		}
		first = first->next;
	}
	return false;
}

int
rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t index, uint64_t tags) {
	struct Rope *rope = cursor->rope;
	if (tags != 0) {
		rope_index_t global_index = 0;
		struct RopeNode *node = rope_first(rope);
		for (; node; rope_node_next(&node)) {
			const size_t char_size = rope_node_char_size(node);
			if (rope_node_match_tags(node, tags)) {
				if (index < char_size) {
					break;
				}
				index -= char_size;
			}
			global_index += char_size;
		}
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
cursor_insert(
		struct RopeCursor *cursor, rope_index_t index, struct RopeNode *node) {
	struct Rope *rope = cursor->rope;
	struct RopeNode *root = rope->root;
	int rv = 0;

	rope_index_t byte_index = 0;
	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;
	struct RopeNode *insert_node =
			rope_node_find_char(root, index, /*tags=*/0, &byte_index);

	rv = rope_node_split(insert_node, &rope->pool, byte_index, &left, &right);
	if (rv < 0) {
		goto out;
	}

	if (left) {
		rv = rope_node_insert_right(left, node, &rope->pool);
	} else {
		rv = rope_node_insert_left(right, node, &rope->pool);
	}
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

#ifdef ROPE_SINGLE_LINE_NODES
int
rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags) {
	int rv = 0;
	struct Rope *rope = cursor->rope;

	const uint8_t *chunk = data;
	rope_index_t cursor_index = cursor->index;
	off_t cursor_offset = 0;

	while (chunk != NULL && chunk < data + byte_size) {
		const uint8_t *chunk_end =
				memchr(chunk, ROPE_NEWLINE, byte_size - (chunk - data));
		size_t chunk_size;
		if (chunk_end) {
			chunk_end++;
			chunk_size = chunk_end - chunk;
		} else {
			chunk_size = byte_size - (chunk - data);
		}
		struct RopeNode *node = rope_node_new(&rope->pool);
		if (node == NULL) {
			rv = -1;
			goto out;
		}
		rope_node_set_tags(node, tags);

		rv = rope_node_set_value(node, chunk, chunk_size);
		if (rv < 0) {
			goto out;
		}
		size_t char_size = cx_utf8_clen(chunk, chunk_size);
		rv = cursor_insert(cursor, cursor_index + cursor_offset, node);
		if (rv < 0) {
			goto out;
		}
		chunk = chunk_end;
		cursor_offset += char_size;
	}

	cursor_damaged(cursor, cursor_offset);
out:
	return rv;
}
#else /* ROPE_SINGLE_LINE_NODES */
int
rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags) {
	int rv = 0;
	struct Rope *rope = cursor->rope;

	if (byte_size == 0) {
		return 0;
	}

	rope_index_t cursor_index = cursor->index;
	struct RopeNode *node = rope_node_new(&rope->pool);
	rope_node_set_tags(node, tags);
	if (node == NULL) {
		rv = -1;
		goto out;
	}
	rv = rope_node_set_value(node, data, byte_size);
	if (rv < 0) {
		goto out;
	}
	size_t char_count = rope_node_char_size(node);

	rv = cursor_insert(cursor, cursor_index, node);
	if (rv < 0) {
		goto out;
	}

	cursor_move_back(cursor);
	cursor_damaged(cursor, 0, char_count);
out:
	return rv;
}
#endif

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
	size_t deleted = 0;
	rope_char_index_t start_index = index;

	if (remaining == 0) {
		return 0;
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
	}
	cursor_move_back(cursor);
	cursor_damaged(cursor->next, cursor->index, -(off_t)deleted);

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
	cursor->next = NULL;
	return 0;
}
