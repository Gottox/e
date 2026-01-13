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

int
rope_node_append_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	assert(ROPE_NODE_IS_LEAF(node));

	if (rope_node_type(node) != ROPE_NODE_INLINE_LEAF) {
		rv = -ROPE_ERROR_INVALID_TYPE;
		goto out;
	}
	size_t size = rope_node_byte_size(node);
	size_t available = ROPE_INLINE_LEAF_SIZE - size;

	if (byte_size > available) {
		rv = -ROPE_ERROR_OOB;
		goto out;
	}
	memcpy(&node->data.inline_leaf.data[size], data, byte_size);
	node->data.inline_leaf.byte_size += byte_size;

out:
	return rv;
}

int
rope_node_set_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	if (byte_size <= ROPE_INLINE_LEAF_SIZE) {
		rope_node_set_type(node, ROPE_NODE_INLINE_LEAF);
		node->data.inline_leaf.byte_size = byte_size;
		memcpy(node->data.inline_leaf.data, data, byte_size);
	} else {
		struct RopeRcString *rc_str = rope_rc_string_new(data, byte_size);
		if (rc_str == NULL) {
			rv = -ROPE_ERROR_OOM;
			goto out;
		}
		rope_node_set_rc_string(node, rc_str, 0, byte_size);
		rope_rc_string_release(rc_str);
	}

out:
	return rv;
}

void
rope_node_set_rc_string(
		struct RopeNode *node, struct RopeRcString *rc_str, size_t byte_index,
		size_t size) {
	assert(byte_index + size <= rc_str->size);
	rope_node_set_type(node, ROPE_NODE_LEAF);
	node->data.leaf.byte_size = size;
	node->data.leaf.owned = rope_rc_string_retain(rc_str);
	node->data.leaf.data = &rc_str->data[byte_index];
}

void
rope_node_cleanup(struct RopeNode *node) {
	if (node == NULL) {
		return;
	}

	if (rope_node_type(node) == ROPE_NODE_LEAF) {
		rope_rc_string_release(node->data.leaf.owned);
		node->data.leaf.owned = NULL;
		node->data.leaf.data = NULL;
	}
}

void
rope_node_free(struct RopeNode *node, struct RopePool *pool) {
	if (node == NULL) {
		return;
	}
	switch (rope_node_type(node)) {
	case ROPE_NODE_LEAF:
		rope_rc_string_release(node->data.leaf.owned);
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
