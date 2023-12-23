#define _GNU_SOURCE

#include <cextras/unicode.h>
#include <rope.h>
#include <string.h>

#define SEARCH(first, cond, res) \
	for (struct RopeCursor *c = (first); c && (cond); c = c->next) { \
		(res) = c; \
	}

static void
cursor_detach(struct RopeCursor *cursor) {
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
	SEARCH(first, c->index < cursor->index, prev);
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
	rope_char_index_t column = 0;
	struct RopeNode *node = rope_node_find(cursor->rope->root, &index);

	cursor->line = rope_node_line_number(node, index);

	const uint8_t *value, *p;
	size_t size = 0;
	value = rope_node_value(node, &size);

	p = memrchr(value, '\n', cx_utf8_bidx(value, size, index));
	if (p) {
		column = cx_utf8_clen(value, p - value) + 1;
	} else {
		column = index;
		for (; !p && rope_node_prev(&node);) {
			value = rope_node_value(node, &size);
			p = memrchr(value, '\n', size);
			if (p) {
				column += cx_utf8_clen(value, p - value) + 1;
			} else {
				column += cx_utf8_clen(value, size);
			}
		}
	}
	cursor->column = column;
}
static int
cursor_update(struct RopeCursor *cursor) {
	cursor_detach(cursor);
	cursor_attach(cursor);
	cursor_update_location(cursor);
	return 0;
}

static void
cursor_damaged(
		struct RopeCursor *cursor, rope_char_index_t index, off_t offset) {
	// First pass: Update indices and locations
	for (struct RopeCursor *c = cursor; c; c = c->next) {
		if (c->index + offset < index) {
			c->index = index;
		} else {
			c->index += offset;
		}
		// Note: We don't need to call update_cursor here as the order is
		// preserved. We do need to update the location though.
		cursor_update_location(c);
	}
	// Second pass: Let the cursor callback know
	for (struct RopeCursor *c = cursor; c; c = c->next) {
		if (c->callback) {
			c->callback(c->rope, c, c->userdata);
		}
	}
}

int
rope_cursor_init(
		struct RopeCursor *cursor, struct Rope *rope, rope_char_index_t index,
		rope_cursor_callback_t callback, void *userdata) {
	if (index > rope->root->char_size) {
		return -1;
	}
	cursor->callback = callback;
	cursor->userdata = userdata;
	cursor->rope = rope;
	cursor->index = index;
	cursor->next = NULL;
	cursor_attach(cursor);
	return 0;
}

rope_char_index_t
find_line_index(const uint8_t *value, size_t size, rope_index_t line) {
	const uint8_t *p = value;
	for (; line; line--) {
		p = memchr(p, '\n', size);
		p++;
	}
	return cx_utf8_clen(value, p - value);
}

int
rope_cursor_set(
		struct RopeCursor *cursor, rope_index_t line,
		rope_char_index_t column) {
	struct Rope *rope = cursor->rope;
	struct RopeNode *node = rope_node_find_line(rope->root, &line);
	if (node == NULL) {
		return -1;
	}
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	// TODO: make sure column doesn't wrap lines.
	cursor->index = find_line_index(value, size, line);
	cursor->index += column;

	return cursor_update(cursor);
}

int
rope_cursor_move(struct RopeCursor *cursor, off_t offset) {
	// TODO: instead of this shenanigans, just check for addition overflow.
	if (offset < 0 && (rope_char_index_t)-offset > cursor->index) {
		cursor->index = 0;
	} else {
		cursor->index = cursor->index + offset;
	}
	return cursor_update(cursor);
}

int
rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *root = rope->root;
	rv = rope_node_insert(root, rope, cursor->index, data, byte_size);
	if (rv < 0) {
		goto out;
	}

	size_t char_size = cx_utf8_clen(data, byte_size);
	cursor_damaged(cursor, 0, char_size);
out:
	return rv;
}

int
rope_cursor_insert_str(struct RopeCursor *cursor, const char *str) {
	return rope_cursor_insert(cursor, (const uint8_t *)str, strlen(str));
}

int
rope_cursor_delete(struct RopeCursor *cursor, size_t char_count) {
	rope_char_index_t index = cursor->index;
	struct Rope *rope = cursor->rope;
	struct RopeNode *node = rope_node_find(rope->root, &index);
	struct RopeNode *next = NULL;

	if (index != 0) {
		rope_node_split(node, rope, index);
		node = rope_node_right(node);
	}
	while (node && node->char_size <= char_count) {
		next = node;
		rope_node_next(&next);
		char_count -= node->char_size;
		rope_node_delete(node, rope);
		node = next;
	}
	if (node && node->char_size > char_count) {
		rope_node_split(node, rope, char_count);
		node = rope_node_left(node);
		rope_node_delete(node, rope);
	}
	cursor_damaged(cursor->next, cursor->index, -char_count);

	return 0;
}

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index) {
	rope_char_index_t char_index = cursor->index;

	struct RopeNode *node = rope_node_find(cursor->rope->root, &char_index);
	if (node == NULL) {
		return NULL;
	}

	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	*byte_index = cx_utf8_bidx(value, size, char_index);

	return node;
}

int
rope_cursor_cleanup(struct RopeCursor *cursor) {
	cursor_detach(cursor);
	cursor->next = NULL;
	return 0;
}
