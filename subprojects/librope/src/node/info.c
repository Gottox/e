#include <assert.h>
#include <rope.h>
#include <string.h>

size_t
rope_node_dim(const struct RopeNode *node, enum RopeUnit unit) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.dim[unit];
	} else {
		return rope_str_dim(&node->data.leaf, unit);
	}
}

size_t
rope_node_depth(struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->bits & ~ROPE_NODE_TYPE_MASK;
	} else {
		return 0;
	}
}

const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size) {
	assert(ROPE_NODE_IS_LEAF(node));
	return rope_str_data(&node->data.leaf, size);
}

rope_node_type_t
rope_node_type(const struct RopeNode *node) {
	return (node->bits >> 63) & 0x1;
}
