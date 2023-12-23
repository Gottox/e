#include <rope.h>
#include <string.h>
#include <utf8util.h>

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

static int
cursor_update(struct RopeCursor *cursor) {
	cursor_detach(cursor);
	cursor_attach(cursor);
	return 0;
}

static void
cursor_damaged(
		struct RopeCursor *cursor, rope_char_index_t index, off_t offset) {
	return;
	for (; cursor; cursor = cursor->next) {
		if (cursor->index + offset < index) {
			cursor->index = index;
		} else {
			cursor->index += offset;
		}
		if (cursor->callback) {
			cursor->callback(cursor->rope, cursor, cursor->userdata);
		}
		// We don't need to update the cursor as the order is preserved.
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
	return utf8_clen(value, p - value);
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
rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *root = rope->root;
	rv = rope_node_insert(root, rope, cursor->index, data, byte_size);
	if (rv < 0) {
		goto out;
	}

	size_t char_size = utf8_clen(data, byte_size);
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
	*byte_index = utf8_bidx(value, size, char_index);

	return node;
}

int
rope_cursor_cleanup(struct RopeCursor *cursor) {
	cursor_detach(cursor);
	cursor->next = NULL;
	return 0;
}
