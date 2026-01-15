#include <assert.h>
#include <rope.h>
#include <string.h>

struct RopeNode *
rope_node_new(struct RopePool *pool) {
	struct RopeNode *new_node = rope_pool_get(pool);
	if (new_node) {
		memset(new_node, 0, sizeof(struct RopeNode));
	}
	return new_node;
}

void
rope_node_cleanup(struct RopeNode *node) {
	if (node == NULL) {
		return;
	}

	if (rope_node_type(node) == ROPE_NODE_LEAF) {
		rope_str_cleanup(&node->data.leaf.value);
	}
}

void
rope_node_free(struct RopeNode *node, struct RopePool *pool) {
	if (node == NULL) {
		return;
	}
	switch (rope_node_type(node)) {
	case ROPE_NODE_LEAF:
		rope_str_cleanup(&node->data.leaf.value);
		break;
	case ROPE_NODE_BRANCH:
		rope_node_free(rope_node_left(node), pool);
		rope_node_free(rope_node_right(node), pool);
		break;
	default:
		break;
	}
	rope_pool_recycle(pool, node);
}
