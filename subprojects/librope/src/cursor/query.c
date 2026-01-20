#define _GNU_SOURCE

#include "cursor_internal.h"

#include "rope_node.h"
#include <cextras/unicode.h>
#include <rope.h>
#include <string.h>

struct RopeNode *
rope_cursor_find_node(
		struct RopeCursor *cursor, struct RopeNode *node, enum RopeUnit unit,
		size_t index, uint64_t tags, rope_byte_index_t *node_byte_index,
		rope_byte_index_t *local_byte_index) {
	struct Rope *rope = cursor->rope;
	if (node == NULL) {
		node = rope->root;
	}
	*local_byte_index = 0;
	size_t byte_index = 0;
	if (tags == 0) {
		while (ROPE_NODE_IS_BRANCH(node)) {
			struct RopeNode *left = rope_node_left(node);
			const size_t left_size = rope_node_dim(left, unit);
			if (index < left_size) {
				node = left;
			} else {
				node = rope_node_right(node);
				index -= left_size;
				byte_index += rope_node_dim(left, ROPE_BYTE);
			}
		}
	} else {
		node = rope_node_first(node);
		do {
			const size_t size = rope_node_dim(node, unit);
			if (rope_node_match_tags(node, tags)) {
				if (index < size) {
					break;
				}
				index -= size;
			}
			byte_index += rope_node_dim(node, ROPE_BYTE);
		} while ((node = rope_node_next(node)));
	}

	if (node) {
		if (node_byte_index) {
			*node_byte_index = byte_index;
		}

		*local_byte_index =
				rope_str_unit_to_byte(&node->data.leaf, unit, index);
	}
	return node;
}

size_t
rope_node_byte_to_index(
		struct RopeNode *node, rope_byte_index_t byte_idx, enum RopeUnit unit) {
	if (unit == ROPE_BYTE) {
		return byte_idx;
	}
	if (node == NULL) {
		return 0;
	}

	size_t prefix = 0;

	// O(log n) descent: navigate by BYTE, accumulate in target unit
	while (ROPE_NODE_IS_BRANCH(node)) {
		struct RopeNode *left = rope_node_left(node);
		const size_t left_byte_size = rope_node_dim(left, ROPE_BYTE);

		if (byte_idx < left_byte_size) {
			node = left;
		} else {
			prefix += rope_node_dim(left, unit);
			byte_idx -= left_byte_size;
			node = rope_node_right(node);
		}
	}

	return prefix + rope_str_unit_from_byte(&node->data.leaf, unit, byte_idx);
}

/*
 * Cursor query functions
 */

size_t
byte_index_to_index(
		struct Rope *rope, rope_byte_index_t byte_idx, enum RopeUnit unit) {
	return rope_node_byte_to_index(rope->root, byte_idx, unit);
}

size_t
rope_cursor_index(struct RopeCursor *cursor, enum RopeUnit unit) {
	return byte_index_to_index(cursor->rope, cursor->index, unit);
}

rope_char_index_t
rope_cursor_char_index(struct RopeCursor *cursor) {
	return rope_cursor_index(cursor, ROPE_CHAR);
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

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_byte_index_t *byte_index) {
	rope_byte_index_t cursor_byte_index = cursor->index;

	struct RopeNode *node = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor_byte_index, /*tags=*/0, NULL,
			byte_index);
	if (node == NULL) {
		return NULL;
	}

	return node;
}
