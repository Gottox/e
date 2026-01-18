#include <assert.h>
#include <rope.h>
#include <string.h>

size_t
rope_node_depth(struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->tags & ~ROPE_NODE_TYPE_MASK;
	} else {
		return 0;
	}
}

const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size) {
	assert(ROPE_NODE_IS_LEAF(node));
	return rope_str_data(&node->data.leaf, size);
}

static size_t
leaf_newline_count(const struct RopeNode *node) {
	size_t byte_size;
	const uint8_t *data = rope_node_value(node, &byte_size);
	size_t new_lines = 0;
	const uint8_t *p = data;
	while (1) {
		p = memchr(p, '\n', byte_size - (size_t)(p - data));
		if (p) {
			new_lines++;
			p++;
		} else {
			break;
		}
	}
	return new_lines;
}

size_t
rope_node_new_lines(const struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.newline_count;
	} else if (ROPE_NODE_IS_LEAF(node)) {
		return leaf_newline_count(node);
	}
	ROPE_UNREACHABLE();
}

size_t
rope_node_char_size(const struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.char_count;
	} else if (ROPE_NODE_IS_LEAF(node)) {
		return rope_str_chars(&node->data.leaf);
	}
	ROPE_UNREACHABLE();
}

size_t
rope_node_byte_size(const struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.byte_count;
	} else if (ROPE_NODE_IS_LEAF(node)) {
		return rope_str_bytes(&node->data.leaf);
	}
	ROPE_UNREACHABLE();
}

size_t
rope_node_cp_size(const struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.cp_count;
	} else if (ROPE_NODE_IS_LEAF(node)) {
		return rope_str_codepoints(&node->data.leaf);
	}
	ROPE_UNREACHABLE();
}

size_t
rope_node_utf16_size(const struct RopeNode *node) {
	if (ROPE_NODE_IS_BRANCH(node)) {
		return node->data.branch.dim.utf16_count;
	} else if (ROPE_NODE_IS_LEAF(node)) {
		return rope_str_utf16_cps(&node->data.leaf);
	}
	ROPE_UNREACHABLE();
}

rope_node_type_t
rope_node_type(const struct RopeNode *node) {
	return (node->tags >> 63) & 0x1;
}
