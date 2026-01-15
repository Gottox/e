#include <assert.h>
#include <rope.h>
#include <string.h>

#define ROPE_NODE_AGGREGATE(type, func, ...) \
	type func(const struct RopeNode *node) { \
		if (ROPE_NODE_IS_BRANCH(node)) { \
			struct RopeNode *left = rope_node_left(node); \
			struct RopeNode *right = rope_node_right(node); \
			return func(left) + func(right); \
		} else if (ROPE_NODE_IS_LEAF(node)) \
			__VA_ARGS__ else { \
				ROPE_UNREACHABLE(); \
			} \
	}

size_t
rope_node_depth(struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.depth;
	} else {
		return 0;
	}
}

const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size) {
	assert(ROPE_NODE_IS_LEAF(node));
	return rope_str_data(&node->data.leaf.value, size);
}

ROPE_NODE_AGGREGATE(size_t, rope_node_new_lines, {
	size_t byte_size;
	const uint8_t *data = rope_node_value(node, &byte_size);
	size_t new_lines = 0;
	const uint8_t *p = data;
	while (1) {
		p = memchr(p, '\n', byte_size - (p - data));
		if (p) {
			new_lines++;
			p++;
		} else {
			break;
		}
	}
	return new_lines;
})

ROPE_NODE_AGGREGATE(size_t, rope_node_char_size, {
	return rope_str_char_count(&node->data.leaf.value);
})

ROPE_NODE_AGGREGATE(size_t, rope_node_byte_size, {
	return rope_str_byte_count(&node->data.leaf.value);
})

enum RopeNodeType
rope_node_type(const struct RopeNode *node) {
	return node->type;
}
