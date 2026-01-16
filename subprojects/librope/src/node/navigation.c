#include <alloca.h>
#include <assert.h>
#include <cextras/memory.h>
#include <cextras/unicode.h>
#include <rope.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>

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
rope_node_sibling(const struct RopeNode *node) {
	assert(!ROPE_NODE_IS_ROOT(node));

	return rope_node_child(rope_node_parent(node), !rope_node_which(node));
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
			return rope_node_leaf(rope_node_child(node->parent, which), !which);
		}
		node = rope_node_parent(node);
	}
	return NULL;
}

struct RopeNode *
rope_node_find_char(
		struct RopeNode *node, rope_char_index_t char_index, uint64_t tags,
		rope_byte_index_t *byte_index) {
	*byte_index = 0;
	node = rope_node_first(node);
	do {
		if (rope_node_match_tags(node, tags) == false) {
			continue;
		}
		const size_t char_size = rope_node_char_size(node);
		if (char_size >= char_index) {
			break;
		}
		char_index -= char_size;
	} while ((node = rope_node_next(node)));

	if (node) {
		size_t size = 0;
		const uint8_t *value = rope_node_value(node, &size);
		size_t trailing_bytes =
				rope_str_char_to_byte_index(value, size, char_index);
		*byte_index = trailing_bytes;
	}
	return node;
}

static struct RopeNode *
find_line(
		struct RopeNode *node, rope_index_t line, rope_byte_index_t *char_index,
		uint64_t tags) {
	node = rope_node_first(node);
	do {
		if (rope_node_match_tags(node, tags) == false) {
			continue;
		}
		size_t node_new_lines = rope_node_new_lines(node);
		if (line <= node_new_lines) {
			break;
		}
		line -= node_new_lines;
	} while ((node = rope_node_next(node)));

	size_t byte_size = 0;
	const uint8_t *value = rope_node_value(node, &byte_size);
	// Fastpath for new lines at the end of the node.
	size_t node_new_lines = rope_node_new_lines(node);
	if (line == node_new_lines && value[byte_size - 1] == ROPE_NEWLINE) {
		struct RopeNode *next_node = rope_node_next(node);
		if (next_node) {
			*char_index = 0;
			return next_node;
		} else {
			*char_index = node_new_lines;
			return node;
		}
	}

	size_t remaining_size = byte_size;
	const uint8_t *remaining = value;
	for (; line;) {
		remaining = memchr(remaining, ROPE_NEWLINE, remaining_size);
		assert(remaining != NULL);
		line--;
		remaining++; // Skip the newline character.
		remaining_size = byte_size - (remaining - value);
	}
	*char_index = cx_utf8_bidx(value, byte_size, remaining - value);
	return node;
}

struct RopeNode *
rope_node_find(
		struct RopeNode *node, rope_index_t line, rope_index_t column,
		uint64_t tags, rope_byte_index_t *byte_index) {
	rope_index_t drop_chars = 0;
	node = find_line(node, line, &drop_chars, tags);
	column += drop_chars;

	while (1) {
		if (!rope_node_match_tags(node, tags)) {
			continue;
		}
		const size_t char_size = rope_node_char_size(node);
		if (char_size > column) {
			break;
		}
		column -= char_size;
		node = rope_node_next(node);
	}

	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);

	*byte_index = cx_utf8_bidx(value, size, column);
	return node;
}
