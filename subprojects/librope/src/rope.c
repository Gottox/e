#include <rope.h>
#include <stdbool.h>
#include <string.h>

int
rope_init(struct Rope *rope, struct RopePool *pool) {
	int rv = 0;

	rope->pool = pool;

	rope->root = rope_pool_get(rope->pool);
	if (rope->root == NULL) {
		rv = -1;
		goto out;
	}

out:
	return rv;
}

int
rope_chores(struct Rope *rope) {
	rope->chores_counter += 1;
	if (rope->chores_counter % ROPE_CHORE_RUN_INTERVAL != 0) {
		return 0;
	}
	return rope_node_chores(rope->root, rope->pool);
}

int
rope_append_str(struct Rope *rope, const char *str) {
	size_t byte_size = strlen(str);
	return rope_append(rope, (const uint8_t *)str, byte_size);
}

int
rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size) {
	size_t char_count = rope_node_dim(rope->root, ROPE_CHAR);
	return rope_insert(rope, char_count, data, byte_size);
}

int
rope_delete_at(
		struct Rope *rope, enum RopeUnit unit, size_t index, size_t count) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to(&cursor, unit, index, 0);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_delete_at(&cursor, unit, count);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

int
rope_delete(
		struct Rope *rope, rope_char_index_t char_index, size_t char_count) {
	return rope_delete_at(rope, ROPE_CHAR, char_index, char_count);
}

struct RopeNode *
rope_find_at(
		struct Rope *rope, enum RopeUnit unit, size_t index,
		rope_byte_index_t *byte_index) {
	int rv = 0;
	struct RopeNode *node = NULL;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to(&cursor, unit, index, 0);
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
rope_find(
		struct Rope *rope, rope_char_index_t char_index,
		rope_byte_index_t *byte_index) {
	return rope_find_at(rope, ROPE_CHAR, char_index, byte_index);
}

struct RopeNode *
rope_first(struct Rope *rope) {
	return rope_node_first(rope->root);
}

int
rope_insert_at(
		struct Rope *rope, enum RopeUnit unit, size_t index,
		const uint8_t *data, size_t byte_size) {
	int rv = 0;
	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_move_to(&cursor, unit, index, 0);
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
rope_insert(
		struct Rope *rope, size_t char_index, const uint8_t *data,
		size_t byte_size) {
	return rope_insert_at(rope, ROPE_CHAR, char_index, data, byte_size);
}

size_t
rope_size(struct Rope *rope, enum RopeUnit unit) {
	return rope_node_dim(rope->root, unit);
}

int
rope_char_size(struct Rope *rope) {
	return rope_size(rope, ROPE_CHAR);
}

int
rope_byte_size(struct Rope *rope) {
	return rope_size(rope, ROPE_BYTE);
}

char *
rope_to_str(struct Rope *rope, uint64_t tags) {
	char *str = NULL;
	int rv = 0;
	struct RopeRange range = {0};
	rope_char_index_t char_size = rope_char_size(rope);

	rv = rope_range_init(&range, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_range_start_move_to_index(&range, 0, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_range_end_move_to_index(&range, char_size, 0);

	str = rope_range_to_str(&range, tags);
out:
	rope_range_cleanup(&range);
	return str;
}

void
rope_cleanup(struct Rope *rope) {
	rope_node_free(rope->root, rope->pool);
}
