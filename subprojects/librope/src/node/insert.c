#include "rope_node.h"
#include "rope_str.h"
#include <assert.h>
#include <grapheme.h>
#include <rope.h>
#include <string.h>

static int
merge_next_char_bytes(struct RopeNode *node, struct RopePool *pool) {
	size_t merge_count = 0;
	struct RopeNode *current = node;
	uint64_t tags = rope_node_tags(current);
	for (uint_least16_t state = 0;; merge_count++) {
		struct RopeNode *next = rope_node_next(current);
		if (next == NULL) {
			break;
		}
		if (rope_node_tags(next) != tags) {
			break;
		}
		size_t should_stitch = rope_str_should_stitch(
				&current->data.leaf, &next->data.leaf, &state);
		if (should_stitch == 0) {
			break;
		}
		current = next;
	}

	return rope_node_merge(node, merge_count, pool);
}

static int
node_insert_unbalanced(
		struct RopeNode *target, struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	assert(ROPE_NODE_IS_LEAF(target));
	assert(ROPE_NODE_IS_LEAF(node));

	int rv = 0;
	struct RopeNode *new_node = NULL;

	new_node = rope_node_new(pool);
	if (new_node == NULL) {
		goto out;
	}

	rope_node_move(new_node, target);

	rope_node_cleanup(target);
	rope_node_set_type(target, ROPE_NODE_BRANCH);
	struct RopeNode **children = target->data.branch.children;
	children[which] = node;
	children[!which] = new_node;
	rope_node_update_children(target);
	
	// Immediately set depth to 1 (both children are leaves with depth 0)
	// This ensures the node can be safely used in rotations
	target->bits &= ROPE_NODE_TYPE_MASK;
	target->bits |= 1;  // depth = 1
	
	new_node = NULL;

out:
	rope_node_free(new_node, pool);
	return rv;
}

static int
node_insert_or_append(
		struct RopeNode **node, struct RopeStr *str, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	int rv = 0;
	if (rope_str_size(str, ROPE_BYTE) == 0) {
		return 0;
	}

	struct RopeStr *node_str = &(*node)->data.leaf;
	if (rope_node_tags(*node) == tags) {
		size_t position = which == ROPE_LEFT ? 0 : SIZE_MAX;
		int rv = rope_str_inline_insert_str(node_str, ROPE_BYTE, position, str);
		if (rv == 0) {
			goto out;
		}
	}

	if (rope_node_size(*node, ROPE_BYTE) == 0) {
		rope_node_set_tags(*node, tags);
		rope_str_move(node_str, str);
		goto out;
	}

	struct RopeNode *new_node = rope_node_new(pool);
	rope_node_set_tags(new_node, tags);
	if (new_node == NULL) {
		rv = -1;
		goto out;
	}

	struct RopeStr *new_str = &new_node->data.leaf;
	rope_str_move(new_str, str);
	if (rv < 0) {
		goto out;
	}
	rv = node_insert_unbalanced(*node, new_node, pool, which);
	if (rv < 0) {
		goto out;
	}
	rope_node_balance_up(new_node);
	*node = new_node;
out:
	rope_node_propagate_sizes(*node);
	return rv;
}

static int
node_try_stitch(
		struct RopeNode **node, struct RopeStr *str, struct RopePool *pool) {
	int rv = 0;

	struct RopeStr *node_str = &(*node)->data.leaf;
	const size_t seam_right = rope_str_should_stitch(node_str, str, NULL);
	if (seam_right == 0) {
		goto out;
	}

	const size_t seam_left = rope_str_last_char_index(node_str);
	struct RopeStr seam = {0};
	rv = rope_str_stitch(&seam, node_str, seam_left, str, seam_right);
	if (rv < 0) {
		goto out;
	}
	rope_node_propagate_sizes(*node);

	rv = node_insert_or_append(
			node, &seam, rope_node_tags(*node), pool, ROPE_RIGHT);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		rope_str_cleanup(&seam);
	}
	return rv;
}

int
rope_node_insert(
		struct RopeNode *node, struct RopeStr *str, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	struct RopeStr chunk = {0};
	int rv = 0;
	if (rope_node_size(node, ROPE_BYTE) > 0 && which == ROPE_LEFT) {
		struct RopeNode *prev = rope_node_prev(node);
		if (prev != NULL) {
			node = rope_node_prev(node);
			which = ROPE_RIGHT;
		}
	}

	if (rope_node_tags(node) == tags) {
		rv = node_try_stitch(&node, str, pool);
	}

	while (rope_str_size(str, ROPE_BYTE) > 0) {
		rope_str_move(&chunk, str);
		rv = rope_str_split_fast(&chunk, str);
		if (rv < 0) {
			goto out;
		}
		rv = node_insert_or_append(&node, &chunk, tags, pool, which);
		if (rv < 0) {
			goto out;
		}
		which = ROPE_RIGHT;
	}

	rv = merge_next_char_bytes(node, pool);
out:
	rope_str_cleanup(&chunk);
	return rv;
}

int
rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	struct RopeStr str = {0};
	int rv = rope_str_init(&str, data, byte_size);
	if (rv < 0) {
		return rv;
	}
	return rope_node_insert(node, &str, tags, pool, ROPE_RIGHT);
}

int
rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	struct RopeStr str = {0};
	int rv = rope_str_init(&str, data, byte_size);
	if (rv < 0) {
		return rv;
	}
	return rope_node_insert(node, &str, tags, pool, ROPE_LEFT);
}

int
rope_node_insert_heap_left(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	struct RopeStr str = {0};
	rope_str_wrap(&str, data, byte_size);
	return rope_node_insert(node, &str, tags, pool, ROPE_LEFT);
}

int
rope_node_insert_heap_right(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	struct RopeStr str = {0};
	rope_str_wrap(&str, data, byte_size);
	return rope_node_insert(node, &str, tags, pool, ROPE_RIGHT);
}
