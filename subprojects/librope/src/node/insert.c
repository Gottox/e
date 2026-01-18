#include <assert.h>
#include <rope.h>
#include <string.h>

static bool
node_should_merge(struct RopeNode *a, struct RopeNode *b) {
	struct RopeStr *a_str = &a->data.leaf;
	struct RopeStr *b_str = &b->data.leaf;

	if (rope_node_tags(a) != rope_node_tags(b)) {
		return false;
	}
	return rope_str_should_merge(a_str, b_str);
}

static int
node_stitch(struct RopeNode *node, struct RopePool *pool) {
	// TODO: allow stitching of branch nodes.
	assert(ROPE_NODE_IS_LEAF(node));

	struct RopeNode *prev = rope_node_prev(node);
	struct RopeNode *next = rope_node_next(node);
	size_t merge = 0;
	if (prev) {
		bool should_merge = node_should_merge(prev, node);
		if (should_merge) {
			merge += 1;
			node = prev;
		}
	}
	if (next) {
		bool should_merge = node_should_merge(node, next);
		if (should_merge) {
			merge += 1;
		}
	}
	return rope_node_merge(node, merge, pool);
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

int
rope_node_insert_new_node(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool, enum RopeDirection which) {
	int rv = 0;
	struct RopeNode *new_node = NULL;
	while (byte_size > 0) {
		size_t chunk_size = CX_MIN(byte_size, ROPE_STR_MAX_SIZE);

		struct RopeNode *new_node = rope_node_new(pool);
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}

		rv = rope_str_init(&new_node->data.leaf, data, chunk_size);
		if (rv < 0) {
			goto out;
		}
		rope_node_set_tags(new_node, tags);

		rv = node_insert(node, new_node, pool, which);
		if (rv < 0) {
			goto out;
		}
		data += chunk_size;
		byte_size -= chunk_size;
		node = new_node;
		which = ROPE_RIGHT;
	}

	rv = node_stitch(node, pool);
	if (rv < 0) {
		goto out;
	}

	new_node = NULL;
out:
	rope_node_free(new_node, pool);
	return rv;
}

int
rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	if (byte_size == 0) {
		return 0;
	}

	int rv = -1;
	bool inserted = false;

	if (rope_node_byte_size(node) == 0) {
		rope_node_set_tags(node, tags);
		rv = rope_str_init(&node->data.leaf, data, byte_size);
	} else if (rope_node_tags(node) == tags) {
		rv = rope_str_inline_append(&node->data.leaf, data, byte_size);
	}
	inserted = rv == 0;

	rv = 0;
	if (!inserted) {
		rv = rope_node_insert_new_node(
				node, data, byte_size, tags, pool, ROPE_RIGHT);
	} else {
		rope_node_propagate_dim(node);
	}

	return rv;
}

int
rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	if (byte_size == 0) {
		return 0;
	}

	struct RopeNode *neighbour = rope_node_prev(node);
	if (neighbour) {
		return rope_node_insert_right(neighbour, data, byte_size, tags, pool);
	} else {
		return rope_node_insert_new_node(
				node, data, byte_size, tags, pool, ROPE_LEFT);
	}
}

static int
node_insert_heap(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	if (byte_size == 0) {
		return 0;
	}

	int rv = 0;
	struct RopeNode *new_node;
	if (rope_node_byte_size(node) == 0) {
		new_node = node;
	} else {
		new_node = rope_node_new(pool);
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}
	}

	rv = rope_str_freeable(&new_node->data.leaf, data, byte_size);
	if (rv < 0) {
		goto out;
	}
	rope_node_set_tags(new_node, tags);

	if (new_node != node) {
		rv = node_insert(node, new_node, pool, which);
		if (rv < 0) {
			goto out;
		}
	}

	rv = node_stitch(node, pool);
	if (rv < 0) {
		goto out;
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
	return node_insert_heap(node, data, byte_size, tags, pool, ROPE_LEFT);
}

int
rope_node_insert_heap_right(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	return node_insert_heap(node, data, byte_size, tags, pool, ROPE_RIGHT);
}
