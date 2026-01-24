#include "cursor_internal.h"

#include "rope_node.h"
#include <assert.h>
#include <rope.h>
#include <string.h>

int
rope_cursor_insert(
		struct RopeCursor *cursor, struct RopeStr *str, uint64_t tags) {
	int rv = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;

	size_t byte_size = rope_str_size(str, ROPE_BYTE);
	if (byte_size == 0) {
		return 0;
	}

	size_t cursor_byte_index = cursor->byte_index;

	size_t insert_at_byte = 0;
	struct RopeNode *insert_at = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor_byte_index, 0, NULL,
			&insert_at_byte);

	rv = rope_node_split(
			insert_at, rope->pool, insert_at_byte, ROPE_BYTE, &left, &right);

	if (left) {
		rv = rope_node_insert(left, str, tags, rope->pool, ROPE_RIGHT);
	} else {
		rv = rope_node_insert(right, str, tags, rope->pool, ROPE_LEFT);
	}
	if (rv < 0) {
		goto out;
	}

	cursor_bubble_up(cursor);
	cursor_damaged(cursor, 0, (off_t)byte_size);

	rv = rope_chores(rope);
out:
	return rv;
}

int
rope_cursor_insert_data(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags) {
	struct RopeStr str = {0};
	int rv = rope_str_init(&str, data, byte_size);
	if (rv < 0) {
		return rv;
	}
	return rope_cursor_insert(cursor, &str, tags);
}

int
rope_cursor_insert_str(
		struct RopeCursor *cursor, const char *str, uint64_t tags) {
	return rope_cursor_insert_data(
			cursor, (const uint8_t *)str, strlen(str), tags);
}

int
rope_cursor_delete(
		struct RopeCursor *cursor, enum RopeUnit unit, size_t count) {
	int rv = 0;

	size_t cursor_byte_index = cursor->byte_index;
	size_t local_byte_index = 0;
	struct Rope *rope = cursor->rope;
	struct RopeNode *node = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor_byte_index, 0, NULL,
			&local_byte_index);
	size_t remaining = count;
	size_t bytes_deleted = 0;

	if (remaining == 0) {
		return 0;
	}

	if (rope_node_size(node, ROPE_BYTE) == local_byte_index) {
		node = rope_node_next(node);
		local_byte_index = 0;
	} else if (local_byte_index != 0) {
		rv = rope_node_split(
				node, rope->pool, local_byte_index, ROPE_BYTE, NULL, &node);
		if (rv < 0) {
			goto out;
		}
		local_byte_index = 0;
	}
	assert(node != NULL);

	while (node) {
		size_t node_size = rope_node_size(node, unit);
		if (node_size >= remaining) {
			break;
		}
		remaining -= node_size;
		bytes_deleted += rope_node_size(node, ROPE_BYTE);
		node = rope_node_delete_and_next(node, rope->pool);
	}

	if (node && remaining > 0) {
		local_byte_index = rope_node_size(node, ROPE_BYTE);
		if (rope_str_is_end(&node->data.leaf, unit, remaining)) {
			rope_node_delete(node, rope->pool);
		} else {
			rv = rope_node_skip(node, unit, remaining);
			if (rv < 0) {
				goto out;
			}
			local_byte_index -= rope_node_size(node, ROPE_BYTE);
		}
		bytes_deleted += local_byte_index;
		remaining = 0;
	}
	assert(remaining == 0);
	cursor_bubble_up(cursor);
	cursor_damaged(cursor, cursor->byte_index, -(off_t)bytes_deleted);

	rv = rope_chores(rope);
out:
	return rv;
}
