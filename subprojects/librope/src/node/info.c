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
	if (size != NULL) {
		*size = node->data.leaf.byte_size;
	}
	switch(rope_node_type(node)) {
		case ROPE_NODE_LEAF:
			return node->data.leaf.data;
		case ROPE_NODE_INLINE_LEAF:
#ifdef ROPE_ENABLE_INLINE_LEAVES
			return node->data.inline_leaf.data;
#else
			assert(ROPE_NODE_IS_ROOT(node));
			assert(*size == 0);
			return (const uint8_t *)"";
#endif
		default:
			ROPE_UNREACHABLE();
	}
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
	size_t byte_size;
	const uint8_t *data = rope_node_value(node, &byte_size);
	return cx_utf8_clen(data, byte_size);
})

ROPE_NODE_AGGREGATE(size_t, rope_node_byte_size, {
	return node->data.leaf.byte_size;
})

enum RopeNodeType
rope_node_type(const struct RopeNode *node) {
	return node->type;
}
