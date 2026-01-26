#include <stdbool.h>
#include <stdint.h>
#define _GNU_SOURCE

#include "cursor_internal.h"

#include "rope_node.h"
#include <cextras/unicode.h>
#include <rope.h>
#include <string.h>
#include <grapheme.h>

struct RopeNode *
rope_cursor_find_node(
		struct RopeCursor *cursor, struct RopeNode *node, enum RopeUnit unit,
		size_t index, uint64_t tags, size_t *node_byte_index,
		size_t *local_byte_index) {
	struct Rope *rope = cursor->rope;
	if (node == NULL) {
		node = rope->root;
	}
	*local_byte_index = 0;
	size_t byte_index = 0;
	if (tags == 0) {
		while (ROPE_NODE_IS_BRANCH(node)) {
			struct RopeNode *left = rope_node_left(node);
			const size_t left_size = rope_node_size(left, unit);
			if (index < left_size) {
				node = left;
			} else {
				node = rope_node_right(node);
				index -= left_size;
				byte_index += rope_node_size(left, ROPE_BYTE);
			}
		}
	} else {
		node = rope_node_first(node);
		do {
			const size_t size = rope_node_size(node, unit);
			if (rope_node_match_tags(node, tags)) {
				if (index < size) {
					break;
				}
				index -= size;
			}
			byte_index += rope_node_size(node, ROPE_BYTE);
		} while ((node = rope_node_next(node)));
	}

	if (node) {
		size_t leaf_size = rope_node_size(node, unit);
		if (index > leaf_size) {
			return NULL;
		}

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
		struct RopeNode *node, size_t byte_index, enum RopeUnit unit,
		uint64_t tags) {
	if (unit == ROPE_BYTE) {
		return byte_index;
	}
	if (node == NULL) {
		return 0;
	}

	size_t prefix = 0;

	if (tags != 0) {
		node = rope_node_first(node);
		do {
			const size_t left_byte_size = rope_node_size(node, ROPE_BYTE);
			if (rope_node_match_tags(node, tags)) {
				if (byte_index < left_byte_size) {
					break;
				}
				prefix += rope_node_size(node, unit);
				byte_index -= left_byte_size;
			}
		} while ((node = rope_node_next(node)));

		if (node == NULL) {
			return prefix;
		}
	} else {
		do {
			struct RopeNode *left = rope_node_left(node);
			const size_t left_byte_size = rope_node_size(left, ROPE_BYTE);

			if (byte_index < left_byte_size) {
				node = left;
			} else {
				prefix += rope_node_size(left, unit);
				byte_index -= left_byte_size;
				node = rope_node_right(node);
			}
		} while (ROPE_NODE_IS_BRANCH(node));
	}

	return prefix + rope_str_unit_from_byte(&node->data.leaf, unit, byte_index);
}

/*
 * Cursor query functions
 */

size_t
rope_cursor_index(
		struct RopeCursor *cursor, enum RopeUnit unit, uint64_t tags) {
	return rope_node_byte_to_index(
			cursor->rope->root, cursor->byte_index, unit, tags);
}

bool
rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second) {
	if (first->byte_index < second->byte_index) {
		return true;
	}

	do {
		if (first->byte_index != second->byte_index) {
			return false;
		} else if (first == second) {
			return true;
		}
	} while ((second = second->prev));

	return false;
}

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, size_t *byte_index) {
	size_t cursor_byte_index = cursor->byte_index;

	struct RopeNode *node = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor_byte_index, /*tags=*/0, NULL,
			byte_index);
	if (node == NULL) {
		return NULL;
	}

	return node;
}

uint_least32_t
rope_cursor_cp(struct RopeCursor *cursor) {
	struct RopeNode *node;
	size_t local_byte_index;

	node = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor->byte_index, /*tags=*/0, NULL,
			&local_byte_index);
	if (node == NULL) {
		return 0;
	}

	size_t byte_size = 0;
	const uint8_t *data = rope_node_value(node, &byte_size);
	data += local_byte_index;
	byte_size -= local_byte_index;
	uint32_t cp;
	grapheme_decode_utf8((const char *)data, byte_size, &cp);
	return cp;
}

bool
rope_cursor_is_eof(struct RopeCursor *cursor) {
	return cursor->byte_index >= rope_size(cursor->rope, ROPE_BYTE);
}
