#include "cursor_internal.h"

#include "rope_node.h"
#include <rope.h>

int
rope_cursor_move_to(
		struct RopeCursor *cursor, enum RopeUnit unit, size_t index,
		uint64_t tags) {
	size_t node_byte_index = 0;
	size_t local_byte_index = 0;
	struct RopeNode *node = rope_cursor_find_node(
			cursor, NULL, unit, index, tags, &node_byte_index,
			&local_byte_index);
	if (node == NULL) {
		return -1;
	}
	cursor->byte_index = node_byte_index + local_byte_index;
	cursor_update(cursor);
	// TODO: Make void
	return 0;
}

int
rope_cursor_move_by(
		struct RopeCursor *cursor, enum RopeUnit unit, off_t offset) {
	struct Rope *rope = cursor->rope;

	if (offset == 0) {
		cursor_bubble_up(cursor);
		return 0;
	}

	if (unit == ROPE_BYTE) {
		if (offset < 0 && (size_t)-offset > cursor->byte_index) {
			cursor->byte_index = 0;
		} else {
			size_t new_index = cursor->byte_index + offset;
			size_t max_byte = rope_node_size(rope->root, ROPE_BYTE);
			cursor->byte_index = new_index > max_byte ? max_byte : new_index;
		}
		cursor_update(cursor);
		return 0;
	}

	size_t local_byte = 0;
	struct RopeNode *leaf = rope_cursor_find_node(
			cursor, NULL, ROPE_BYTE, cursor->byte_index, 0, NULL, &local_byte);
	if (leaf == NULL) {
		cursor->byte_index = 0;
		cursor_update(cursor);
		return 0;
	}

	const struct RopeStr *str = &leaf->data.leaf;
	const size_t local_unit = rope_str_unit_from_byte(str, unit, local_byte);
	const size_t leaf_size = rope_node_size(leaf, unit);

	const size_t abs_offset = CX_MAX(offset, -offset);
	if ((offset >= 0 && !rope_str_is_end(str, unit, local_unit + abs_offset)) ||
		(offset < 0 && abs_offset <= local_unit)) {
		size_t new_local_byte = rope_str_unit_to_byte(
				&leaf->data.leaf, unit, local_unit + offset);
		cursor->byte_index -= local_byte - new_local_byte;
		cursor_update(cursor);
		return 0;
	}

	enum RopeDirection direction;
	size_t remaining;
	if (offset > 0) {
		direction = ROPE_RIGHT;
		remaining = (size_t)offset - (leaf_size - local_unit);
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
			size_t sibling_size = rope_node_size(sibling, unit);

			if (remaining <= sibling_size) {
				size_t target_index;
				if (direction == ROPE_RIGHT) {
					target_index = remaining;
				} else {
					target_index = sibling_size - remaining;
				}
				size_t target_local_byte = 0;
				size_t target_node_byte = 0;
				struct RopeNode *target_leaf = rope_cursor_find_node(
						cursor, sibling, unit, target_index, 0,
						&target_node_byte, &target_local_byte);
				if (target_leaf == NULL) {
					break;
				}
				cursor->byte_index = target_node_byte + target_local_byte;
				cursor_update(cursor);
				return 0;
			}
			remaining -= sibling_size;
		}
		node = parent;
	}

	// Reached root without finding target - clamp
	if (direction == ROPE_RIGHT) {
		cursor->byte_index = rope_node_size(rope->root, ROPE_BYTE);
	} else {
		cursor->byte_index = 0;
	}
	cursor_update(cursor);
	return 0;
}

int
rope_cursor_move(struct RopeCursor *cursor, off_t offset) {
	return rope_cursor_move_by(cursor, ROPE_CHAR, offset);
}
