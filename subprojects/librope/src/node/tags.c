#include "rope_node.h"
#include <assert.h>

#define ROPE_NODE_TRAVERSAL(func, arg_type, arg_name, ...) \
	void func(struct RopeNode *node, arg_type arg_name) { \
		if (ROPE_NODE_IS_BRANCH(node)) { \
			struct RopeNode *left = rope_node_left(node); \
			struct RopeNode *right = rope_node_right(node); \
			func(left, arg_name); \
			func(right, arg_name); \
		} else if (ROPE_NODE_IS_LEAF(node)) \
			__VA_ARGS__ else { \
				ROPE_UNREACHABLE(); \
			} \
	}

static uint64_t *
node_tags(struct RopeNode *node) {
	switch (rope_node_type(node)) {
	case ROPE_NODE_BRANCH:
		return NULL;
	case ROPE_NODE_LEAF:
		return &node->data.leaf.tags;
	default:
		ROPE_UNREACHABLE();
	}
}
uint64_t
rope_node_tags(struct RopeNode *node) {
	if (ROPE_NODE_IS_LEAF(node)) {
		return *node_tags(node);
	} else {
		return 0;
	}
}

bool
rope_node_match_tags(struct RopeNode *node, uint64_t tags) {
	assert(ROPE_NODE_IS_LEAF(node));

	uint64_t node_tags = rope_node_tags(node);
	return (node_tags & tags) == tags;
}

ROPE_NODE_TRAVERSAL(rope_node_set_tags, uint64_t, tags, {
	assert(ROPE_NODE_IS_LEAF(node));
	*node_tags(node) |= tags;
})

ROPE_NODE_TRAVERSAL(rope_node_remove_tags, uint64_t, tags, {
	assert(ROPE_NODE_IS_LEAF(node));
	*node_tags(node) &= ~tags;
})

ROPE_NODE_TRAVERSAL(rope_node_add_tags, uint64_t, tags, {
	assert(ROPE_NODE_IS_LEAF(node));
	*node_tags(node) |= tags;
})
