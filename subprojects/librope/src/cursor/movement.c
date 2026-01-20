#include "cursor_internal.h"

#include "rope_node.h"
#include <rope.h>

int
rope_cursor_move_to(
		struct RopeCursor *cursor, enum RopeUnit unit, size_t index,
		uint64_t tags) {
	rope_byte_index_t node_byte_index = 0;
	rope_byte_index_t local_byte_index = 0;
	struct RopeNode *node = rope_cursor_find_node(
			cursor, NULL, unit, index, tags, &node_byte_index,
			&local_byte_index);
	if (node == NULL) {
		return -1;
	}
	cursor->index = node_byte_index + local_byte_index;
	//old:
	//  cursor->index = local_byte_index;
	//  while ((node = rope_node_prev(node))) {
	//  	cursor->index += rope_node_dim(node, ROPE_BYTE);
	//  }
	return cursor_update(cursor);
}

int
rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t char_index,
		uint64_t tags) {
	return rope_cursor_move_to(cursor, ROPE_CHAR, char_index, tags);
}

int
rope_cursor_move_to_line_col(
		struct RopeCursor *cursor, rope_index_t line,
		rope_char_index_t column) {
	int rv = rope_cursor_move_to(cursor, ROPE_LINE, line, 0);
	if (rv < 0) {
		return rv;
	}
	return rope_cursor_move_by(cursor, ROPE_CHAR, column);
}

int
rope_cursor_move_by(
		struct RopeCursor *cursor, enum RopeUnit unit, off_t offset) {
	struct Rope *rope = cursor->rope;

	// Handle empty rope
	if (rope->root == NULL) {
		cursor->index = 0;
		return cursor_update(cursor);
	}

	// For ROPE_BYTE, use direct arithmetic (no tree traversal needed)
	if (unit == ROPE_BYTE) {
		if (offset < 0 && (size_t)-offset > cursor->index) {
			cursor->index = 0;
		} else {
			size_t new_index = cursor->index + offset;
			size_t max_byte = rope_node_dim(rope->root, ROPE_BYTE);
			cursor->index = new_index > max_byte ? max_byte : new_index;
		}
		return cursor_update(cursor);
	}

	// Step 1: Find current leaf and local byte position
	rope_byte_index_t local_byte = 0;
	struct RopeNode *leaf = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor->index, 0, NULL, &local_byte);
	if (leaf == NULL) {
		cursor->index = 0;
		return cursor_update(cursor);
	}

	// Step 2: Get current position in target unit within the leaf
	size_t local_unit =
			rope_str_unit_from_byte(&leaf->data.leaf, unit, local_byte);
	size_t leaf_dim = rope_node_dim(leaf, unit);

	// Step 3: Check if target is within current leaf (optimization)
	if (offset >= 0 && local_unit + (size_t)offset < leaf_dim) {
		// Target is in same leaf - simple case
		size_t new_local_byte = rope_str_unit_to_byte(
				&leaf->data.leaf, unit, local_unit + offset);
		cursor->index = cursor->index - local_byte + new_local_byte;
		return cursor_update(cursor);
	}
	if (offset < 0 && (size_t)-offset <= local_unit) {
		// Target is in same leaf, moving backward
		size_t new_local_byte = rope_str_unit_to_byte(
				&leaf->data.leaf, unit, local_unit + offset);
		cursor->index = cursor->index - local_byte + new_local_byte;
		return cursor_update(cursor);
	}

	enum RopeDirection direction;
	size_t remaining;
	if (offset > 0) {
		direction = ROPE_RIGHT;
		remaining = (size_t)offset - (leaf_dim - local_unit);
	} else {
		direction = ROPE_LEFT;
		remaining = (size_t)-offset - local_unit;
	}

	struct RopeNode *node = leaf;

	while (!ROPE_NODE_IS_ROOT(node)) {
		struct RopeNode *parent = rope_node_parent(node);
		if (rope_node_which(node) != direction) {
			// Coming from opposite child, check sibling in direction
			struct RopeNode *sibling = rope_node_child(parent, direction);
			size_t sibling_dim = rope_node_dim(sibling, unit);

			if (remaining <= sibling_dim) {
				size_t target_index;
				if (direction == ROPE_RIGHT) {
					target_index = remaining;
				} else {
					target_index = sibling_dim - remaining;
				}
				rope_byte_index_t target_local_byte = 0;
				rope_byte_index_t target_node_byte = 0;
				struct RopeNode *target_leaf = rope_cursor_find_node(
						cursor, sibling, unit, target_index, 0,
						&target_node_byte, &target_local_byte);
				if (target_leaf == NULL) {
					break;
				}
				cursor->index = target_node_byte + target_local_byte;
				return cursor_update(cursor);
			}
			remaining -= sibling_dim;
		}
		node = parent;
	}

	// Reached root without finding target - clamp
	if (direction == ROPE_RIGHT) {
		cursor->index = rope_node_dim(rope->root, ROPE_BYTE);
	} else {
		cursor->index = 0;
	}
	return cursor_update(cursor);
}

int
rope_cursor_move(struct RopeCursor *cursor, off_t offset) {
	return rope_cursor_move_by(cursor, ROPE_CHAR, offset);
}
