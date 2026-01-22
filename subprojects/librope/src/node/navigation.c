#include <assert.h>
#include <rope.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum RopeDirection
rope_node_which(const struct RopeNode *node) {
	assert(!ROPE_NODE_IS_ROOT(node));

	if (rope_node_left(rope_node_parent(node)) == node) {
		return ROPE_LEFT;
	} else {
		return ROPE_RIGHT;
	}
}

struct RopeNode *
rope_node_leaf(struct RopeNode *node, enum RopeDirection which) {
	while (ROPE_NODE_IS_BRANCH(node)) {
		node = rope_node_child(node, which);
	}
	return node;
}

struct RopeNode *
rope_node_child(const struct RopeNode *node, enum RopeDirection which) {
	assert(ROPE_NODE_IS_BRANCH(node));

	return node->data.branch.children[which];
}

struct RopeNode *
rope_node_parent(const struct RopeNode *node) {
	return node->parent;
}

struct RopeNode *
rope_node_neighbour(const struct RopeNode *node, enum RopeDirection which) {
	while (!ROPE_NODE_IS_ROOT(node)) {
		if (rope_node_which(node) != which) {
			struct RopeNode *parent = rope_node_parent(node);
			struct RopeNode *child = rope_node_child(parent, which);

			return rope_node_leaf(child, !which);
		}
		node = rope_node_parent(node);
	}
	return NULL;
}
