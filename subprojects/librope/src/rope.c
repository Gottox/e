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
	size_t root_byte_size = rope_node_size(rope->root, ROPE_BYTE);
	return rope_insert(rope, ROPE_CHAR, root_byte_size, data, byte_size);
}

int
rope_delete(struct Rope *rope, enum RopeUnit unit, size_t index, size_t count) {
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

	rv = rope_cursor_delete(&cursor, unit, count);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

int
rope_insert(
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

	rv = rope_cursor_insert_data(&cursor, data, byte_size, 0);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&cursor);
	return rv;
}

size_t
rope_size(struct Rope *rope, enum RopeUnit unit) {
	return rope_node_size(rope->root, unit);
}

int
rope_to_range(struct Rope *rope, struct RopeRange *range) {
	int rv = 0;

	rv = rope_range_init(range, rope);
	if (rv < 0) {
		goto out;
	}
	struct RopeCursor *end = rope_range_end(range);
	size_t byte_size = rope_size(rope, ROPE_BYTE);
	rv = rope_cursor_move_to(end, ROPE_BYTE, byte_size, 0);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		rope_range_cleanup(range);
	}
	return rv;
}

char *
rope_to_str(struct Rope *rope, uint64_t tags) {
	char *str = NULL;
	int rv = 0;
	struct RopeRange range = {0};
	size_t byte_size = rope_size(rope, ROPE_BYTE);

	rv = rope_range_init(&range, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_BYTE, 0, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_BYTE, byte_size, 0);

	str = rope_range_to_cstr(&range, tags);
out:
	rope_range_cleanup(&range);
	return str;
}

int
rope_to_end_cursor(struct Rope *rope, struct RopeCursor *cursor) {
	int rv = 0;
	size_t byte_size = rope_size(rope, ROPE_BYTE);
	rv = rope_cursor_init(cursor, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_move_to(cursor, ROPE_BYTE, byte_size, 0);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(cursor);
	return rv;
}

void
rope_clear(struct Rope *rope) {
	while (rope->last_cursor != NULL &&
		   rope_cursor_index(rope->last_cursor, ROPE_BYTE, 0) != 0) {
		rope_cursor_move_to(rope->last_cursor, ROPE_BYTE, 0, 0);
	}
	rope_node_free(rope->root, rope->pool);
	rope->root = rope_pool_get(rope->pool);
}

void
rope_cleanup(struct Rope *rope) {
	rope_node_free(rope->root, rope->pool);
}
