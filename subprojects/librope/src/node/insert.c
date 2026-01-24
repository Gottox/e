#include "rope_error.h"
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
	for (uint_least16_t state = 0;;) {
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
		merge_count++;
	}

	return rope_node_merge(node, merge_count, pool);
}

static int
node_insert(
		struct RopeNode *target, struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	// TODO: allow insertion into branch nodes.
	assert(ROPE_NODE_IS_LEAF(target));
	assert(ROPE_NODE_IS_LEAF(node));

	int rv = 0;
	struct RopeNode *new_node = NULL;

	struct RopeNode *neighbour = rope_node_neighbour(node, which);
	if (neighbour && rope_node_depth(neighbour) < rope_node_depth(node)) {
		node = neighbour;
		which = !which;
	}

	new_node = rope_node_new(pool);
	if (new_node == NULL) {
		goto out;
	}

	rope_node_move(new_node, target);

	rope_node_set_type(target, ROPE_NODE_BRANCH);
	struct RopeNode **children = target->data.branch.children;
	children[which] = node;
	children[!which] = new_node;
	rope_node_update_children(target);
	rope_node_balance_up(new_node);
	new_node = NULL;

out:
	rope_node_free(new_node, pool);
	return rv;
}

static int
node_try_stitch(
		struct RopeNode **node, struct RopeStr *str, struct RopePool *pool) {
	int rv = 0;
	// assert(rope_node_size(*node, ROPE_BYTE) > 0);

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

	struct RopeNode *new_node = *node;
	if (rope_node_size(*node, ROPE_BYTE) > 0) {
		new_node = rope_node_new(pool);
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}
	}
	rope_str_move(&new_node->data.leaf, &seam);
	rope_node_set_tags(new_node, rope_node_tags(*node));
	if (new_node != *node) {
		rv = node_insert(*node, new_node, pool, ROPE_RIGHT);
		if (rv < 0) {
			goto out;
		}
	}

	*node = new_node;
out:
	if (rv < 0) {
		rope_str_cleanup(&seam);
		if (new_node != *node) {
			rope_node_free(new_node, pool);
		}
	}
	return rv;
}

static int
node_insert_str(
		struct RopeNode *node, struct RopeStr *str, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	int rv = 0;
	struct RopeNode *new_node = NULL;
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
		new_node = node;
		if (rope_node_size(node, ROPE_BYTE) > 0) {
			new_node = rope_node_new(pool);
			rope_node_set_tags(new_node, tags);
			if (new_node == NULL) {
				rv = -1;
				goto out;
			}
		}
		struct RopeStr *new_str = &new_node->data.leaf;
		rope_str_move(new_str, str);
		rv = rope_str_split_fast(new_str, str);
		if (rv < 0) {
			goto out;
		}
		if (new_node != node) {
			rv = node_insert(node, new_node, pool, which);
			if (rv < 0) {
				goto out;
			}
		}
		which = ROPE_RIGHT;
		node = new_node;
		new_node = NULL;
	}

	rv = merge_next_char_bytes(node, pool);
out:
	if (new_node != node) {
		rope_node_free(new_node, pool);
	}
	return rv;
}

static int
node_insert_data(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool, enum RopeDirection which) {
	struct RopeStr str = {0};
	int rv = rope_str_init(&str, data, byte_size);
	if (rv < 0) {
		return rv;
	}
	return node_insert_str(node, &str, tags, pool, which);
}

int
rope_node_insert(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool, enum RopeDirection which) {
	if (byte_size == 0) {
		return 0;
	}

	if (rope_node_tags(node) == tags) {
		size_t position = which == ROPE_LEFT ? 0 : SIZE_MAX;
		int rv = rope_str_inline_insert(
				&node->data.leaf, ROPE_BYTE, position, data, byte_size);
		if (rv == 0) {
			rope_node_propagate_sizes(node);
			return merge_next_char_bytes(node, pool);
		}
	}

	return node_insert_data(node, data, byte_size, tags, pool, which);
}

int
rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	return rope_node_insert(node, data, byte_size, tags, pool, ROPE_RIGHT);
}

int
rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	return rope_node_insert(node, data, byte_size, tags, pool, ROPE_LEFT);
}

static int
node_insert_wrap(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	if (byte_size == 0) {
		return 0;
	}

	int rv = 0;
	struct RopeNode *new_node;
	if (rope_node_size(node, ROPE_BYTE) == 0) {
		new_node = node;
	} else {
		new_node = rope_node_new(pool);
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}
	}

	rope_str_wrap(&new_node->data.leaf, data, byte_size);
	rope_node_set_tags(new_node, tags);

	if (new_node != node) {
		rv = node_insert(node, new_node, pool, which);
		if (rv < 0) {
			goto out;
		}
	}

	new_node = NULL;
out:
	rope_node_free(new_node, pool);
	return rv;
}

int
rope_node_insert_heap_left(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	return node_insert_wrap(node, data, byte_size, tags, pool, ROPE_LEFT);
}

int
rope_node_insert_heap_right(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	return node_insert_wrap(node, data, byte_size, tags, pool, ROPE_RIGHT);
}
