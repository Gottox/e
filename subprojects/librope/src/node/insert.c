#include <assert.h>
#include <rope.h>
#include <string.h>

static int
node_insert(
		struct RopeNode *target, struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	int rv = 0;
	struct RopeNode *new_node = NULL;

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
	struct RopeNode *new_node = rope_node_new(pool);
	if (new_node == NULL) {
		rv = -1;
		goto out;
	}

	rv = rope_str_init(&new_node->data.leaf.value, data, byte_size);
	if (rv < 0) {
		goto out;
	}
	rope_node_set_tags(new_node, tags);

	struct RopeNode *neighbour = rope_node_neighbour(node, which);
	if (neighbour && rope_node_depth(neighbour) < rope_node_depth(node)) {
		node = neighbour;
		which = !which;
	}

	rv = node_insert(node, new_node, pool, which);
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
		rv = rope_str_init(&node->data.leaf.value, data, byte_size);
	} else if (rope_node_tags(node) == tags) {
		rv = rope_str_inline_append(&node->data.leaf.value, data, byte_size);
	}
	inserted = rv == 0;

	rv = 0;
	if (!inserted) {
		rv = rope_node_insert_new_node(
				node, data, byte_size, tags, pool, ROPE_RIGHT);
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
