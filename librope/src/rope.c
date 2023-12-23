#include <assert.h>
#include <rope.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int
rope_init(struct Rope *rope) {
	int rv = 0;
	rv = rope_pool_init(&rope->pool);
	if (rv < 0) {
		goto out;
	}

	rope->root = rope_pool_get(&rope->pool);
	if (rope->root == NULL) {
		rv = -1;
		goto out;
	}

	rope->bias = ROPE_BIAS_LEFT;
	rope->cursors = NULL;

out:
	return rv;
}

int
rope_append_str(struct Rope *rope, const char *str) {
	size_t byte_size = strlen(str);
	return rope_append(rope, (const uint8_t *)str, byte_size);
}

int
rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size) {
	size_t char_count = rope->root->char_size;
	return rope_insert(rope, char_count, data, byte_size);
}

int
rope_delete(struct Rope *rope, rope_char_index_t index, size_t char_count) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope, index, NULL, NULL);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_delete(&cursor, char_count);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

struct RopeNode *
rope_line(struct Rope *rope, rope_index_t line, rope_byte_index_t *index) {
	struct RopeNode *node = rope_node_find_line(rope->root, &line);
	if (node == NULL) {
		return NULL;
	}
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	const uint8_t *p;

	for (p = value; line; line--) {
		// We are sure, that there is a new line, so we can skip the check
		// for overflows and NULL pointers.
		p = memchr(p, '\n', size);
		p++;
	}
	*index = p - value;

	return node;
}

struct RopeNode *
rope_find(
		struct Rope *rope, rope_char_index_t char_index,
		rope_byte_index_t *byte_index) {
	int rv = 0;
	struct RopeNode *node = NULL;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope, char_index, NULL, NULL);
	if (rv < 0) {
		goto out;
	}
	node = rope_cursor_node(&cursor, byte_index);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return node;
}

int
rope_insert(
		struct Rope *rope, size_t index, const uint8_t *data,
		size_t byte_size) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope, index, NULL, NULL);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_insert(&cursor, data, byte_size);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

int
rope_cleanup(struct Rope *rope) {
	return rope_pool_cleanup(&rope->pool);
}
