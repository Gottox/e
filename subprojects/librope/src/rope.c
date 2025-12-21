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
	size_t char_count = rope_node_char_size(rope->root);
	return rope_insert(rope, char_count, data, byte_size);
}

int
rope_delete(
		struct Rope *rope, rope_char_index_t char_index, size_t char_count) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to_index(&cursor, char_index, 0);
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
rope_find(
		struct Rope *rope, rope_char_index_t char_index,
		rope_byte_index_t *byte_index) {
	int rv = 0;
	struct RopeNode *node = NULL;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to_index(&cursor, char_index, 0);
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

struct RopeNode *
rope_first(struct Rope *rope) {
	return rope_node_first(rope->root);
}

int
rope_insert(
		struct Rope *rope, size_t char_index, const uint8_t *data,
		size_t byte_size) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to_index(&cursor, char_index, 0);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_insert(&cursor, data, byte_size, 0);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

int
rope_char_size(struct Rope *rope) {
	return rope_node_char_size(rope->root);
}

int
rope_byte_size(struct Rope *rope) {
	return rope_node_byte_size(rope->root);
}

int
rope_cleanup(struct Rope *rope) {
	return rope_pool_cleanup(&rope->pool);
}
