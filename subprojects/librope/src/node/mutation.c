#include <assert.h>
#include <rope.h>
#include <stdbool.h>
#include <string.h>

static void
node_set_depth(struct RopeNode *node, size_t depth) {
	assert(ROPE_NODE_IS_BRANCH(node));
	assert(depth <= ROPE_NODE_TYPE_MASK);

	node->tags &= ROPE_NODE_TYPE_MASK;
	node->tags |= depth;
}

void
rope_node_update_depth(struct RopeNode *node) {
	if (!ROPE_NODE_IS_BRANCH(node)) {
		return;
	}

	struct RopeNode *left = rope_node_left(node);
	struct RopeNode *right = rope_node_right(node);

	const size_t left_depth = rope_node_depth(left);
	const size_t right_depth = rope_node_depth(right);
	const size_t depth = CX_MAX(left_depth, right_depth) + 1;
	node_set_depth(node, depth);
}

void
rope_node_update_dim(struct RopeNode *node) {
	if (!ROPE_NODE_IS_BRANCH(node)) {
		return;
	}
	struct RopeNode *left = rope_node_left(node);
	struct RopeNode *right = rope_node_right(node);
	struct RopeDim *dim = &node->data.branch.dim;

	dim->byte_count = rope_node_byte_size(left) + rope_node_byte_size(right);
	dim->char_count = rope_node_char_size(left) + rope_node_char_size(right);
	dim->cp_count = rope_node_cp_size(left) + rope_node_cp_size(right);
	dim->newline_count = rope_node_new_lines(left) + rope_node_new_lines(right);
	dim->utf16_count = rope_node_utf16_size(left) + rope_node_utf16_size(right);
}

void
rope_node_propagate_dim(struct RopeNode *node) {
	while ((node = rope_node_parent(node))) {
		rope_node_update_dim(node);
	}
}

void
rope_node_update_children(struct RopeNode *node) {
	if (!ROPE_NODE_IS_BRANCH(node)) {
		return;
	}

	struct RopeNode *left = rope_node_left(node);
	struct RopeNode *right = rope_node_right(node);

	left->parent = right->parent = node;
}

void
rope_node_move(struct RopeNode *target, struct RopeNode *node) {
	struct RopeNode *target_parent = rope_node_parent(target);
	struct RopeNode *node_parent = rope_node_parent(node);
	memcpy(target, node, sizeof(struct RopeNode));
	memset(node, 0, sizeof(struct RopeNode));
	target->parent = target_parent;
	node->parent = node_parent;
}

void
rope_node_set_type(struct RopeNode *node, rope_node_type_t type) {
	node->tags &= (UINT64_MAX) >> 1;
	node->tags |= ((uint64_t)type) << 63;
}

int
rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr) {
	int rv = 0;
	struct RopeNode *dummy;
	if (left_ptr == NULL) {
		left_ptr = &dummy;
	}
	if (right_ptr == NULL) {
		right_ptr = &dummy;
	}

	while (ROPE_NODE_IS_BRANCH(node)) {
		struct RopeNode *left = rope_node_left(node);
		const size_t left_size = rope_node_byte_size(left);

		if (byte_index < left_size) {
			node = left;
		} else {
			node = rope_node_right(node);
			byte_index -= left_size;
		}
	}

	const size_t byte_size = rope_node_byte_size(node);
	if (byte_index == byte_size) {
		*left_ptr = node;
		*right_ptr = NULL;
		return 0;
	} else if (byte_index == 0) {
		*left_ptr = NULL;
		*right_ptr = node;
		return 0;
	}

	struct RopeNode *left = rope_node_new(pool);
	struct RopeNode *right = rope_node_new(pool);
	if (left == NULL || right == NULL) {
		rv = -ROPE_ERROR_OOM;
		goto out;
	}
	rope_node_set_type(left, rope_node_type(node));
	rope_node_set_type(right, rope_node_type(node));

	rope_node_move(left, node);
	rope_str_byte_split(&left->data.leaf, &right->data.leaf, byte_index);

	rope_node_cleanup(node);

	rope_node_set_type(node, ROPE_NODE_BRANCH);
	struct RopeNode **children = node->data.branch.children;
	children[0] = left;
	children[1] = right;

	rope_node_update_children(node);
	node_set_depth(node, 1);
	rope_node_update_dim(node);
	rope_node_balance_up(node);

	*left_ptr = left;
	*right_ptr = right;
	left = NULL;
	right = NULL;
out:
	rope_node_free(left, pool);
	rope_node_free(right, pool);
	return rv;
}

static struct RopeNode *
node_delete(struct RopeNode *node, struct RopePool *pool) {
	if (ROPE_NODE_IS_ROOT(node)) {
		rope_node_cleanup(node);
		memset(node, 0, sizeof(*node));
		return node;
	} else {
		struct RopeNode *parent = rope_node_parent(node);
		enum RopeDirection which = rope_node_which(node);

		rope_node_delete_child(parent, pool, which);
		return parent;
	}
}

void
rope_node_delete(struct RopeNode *node, struct RopePool *pool) {
	rope_node_balance_up(node_delete(node, pool));
}

static void
node_delete_child(
		struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	assert(ROPE_NODE_IS_BRANCH(node));

	struct RopeNode *child = rope_node_child(node, which);
	struct RopeNode *sibling = rope_node_child(node, !which);
	rope_node_move(node, sibling);
	rope_node_update_children(node);
	rope_node_update_depth(node);
	rope_node_update_dim(node);
	rope_node_free(sibling, pool);
	rope_node_free(child, pool);
}

void
rope_node_delete_child(
		struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	node_delete_child(node, pool, which);
	rope_node_balance_up(node);
}

struct RopeNode *
rope_node_delete_and_next(struct RopeNode *node, struct RopePool *pool) {
	struct RopeNode *next = rope_node_next(node);
	if (next != NULL) {
		struct RopeNode *parent = rope_node_parent(next);
		if (rope_node_left(parent) == node) {
			// If next and node are siblings, node will be collapsed into parent
			// on deletion of next. So we need to continue from parent
			next = parent;
		}
	}
	node_delete(node, pool);
	// TODO: balance up.
	return next;
}

int
rope_node_merge(struct RopeNode *node, size_t count, struct RopePool *pool) {
	int rv = 0;
	if (count < 1) {
		return 0;
	}
	struct RopeNode *start_node = node;

	size_t total_size = rope_node_byte_size(node);
	for (; count; count--) {
		node = rope_node_next(node);
		total_size += rope_node_byte_size(node);
	}

	uint8_t *target;
	struct RopeStr new_value;
	rv = rope_str(&new_value, total_size, &target);
	if (rv != 0) {
		goto out;
	}

	for (node = start_node;;) {
		size_t byte_size;
		const uint8_t *data = rope_str_data(&node->data.leaf, &byte_size);
		memcpy(target, data, byte_size);
		total_size -= byte_size;
		target += byte_size;
		if (total_size == 0) {
			break;
		} else {
			node = rope_node_delete_and_next(node, pool);
		}
	}
	rope_str_truncate(&new_value, SIZE_MAX);
	rope_str_cleanup(&node->data.leaf);
	rope_str_move(&node->data.leaf, &new_value);
	rope_node_balance_up(node);
out:
	return rv;
}

struct NodeMergeContext {
	uint8_t *new_data;
	size_t total_size;
};

void
rope_node_rotate(struct RopeNode *node, enum RopeDirection which) {
	assert(ROPE_NODE_IS_BRANCH(node));

	struct RopeNode **node_children = node->data.branch.children;

	struct RopeNode *pivot = node_children[!which];

	assert(ROPE_NODE_IS_BRANCH(pivot));

	struct RopeNode **pivot_children = pivot->data.branch.children;

	node_children[!which] = pivot_children[!which];
	pivot_children[!which] = pivot_children[which];
	pivot_children[which] = node_children[which];
	node_children[which] = pivot;

	rope_node_update_children(pivot);
	rope_node_update_children(node);
	rope_node_update_depth(pivot);
	rope_node_update_depth(node);
	rope_node_update_dim(pivot);
	rope_node_update_dim(node);
}

void
rope_node_balance_up(struct RopeNode *node) {
	while ((node = rope_node_parent(node))) {
		// Perform rotations as necessary
		const size_t old_depth = rope_node_depth(node);
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);
		size_t left_depth = rope_node_depth(left);
		size_t right_depth = rope_node_depth(right);

		if (left_depth > right_depth + 1) {
			rope_node_rotate(node, ROPE_RIGHT);
		} else if (right_depth > left_depth + 1) {
			rope_node_rotate(node, ROPE_LEFT);
		} else {
			rope_node_update_depth(node);
			rope_node_update_dim(node);
		}

		// Recalculate depth
		const size_t new_depth = rope_node_depth(node);

		// If the depth didn't change, we consider the tree balanced.
		// Note that this only results in a balanced tree if the tree
		// balance was only damaged by a single operation on the node.
		// Continue propagating dimensions to the root.
		if (new_depth == old_depth) {
			rope_node_propagate_dim(node);
			break;
		}
	}
}
