#include <assert.h>
#include <cextras/memory.h>
#include <cextras/unicode.h>
#include <rope.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#define NEWLINE '\n'

static struct RopeNode **
node_child(struct RopeNode *node, enum RopeNodeDirection direction) {
	assert(node->type == ROPE_NODE_BRANCH);
	return &node->data.branch.children[direction];
}

static struct RopeNode **
node_left(struct RopeNode *node) {
	return node_child(node, ROPE_NODE_LEFT);
}

static struct RopeNode **
node_right(struct RopeNode *node) {
	return node_child(node, ROPE_NODE_RIGHT);
}

struct RopeNode *
node_outer(struct RopeNode *node, enum RopeNodeDirection direction) {
	while (node->type == ROPE_NODE_BRANCH) {
		node = *node_child(node, direction);
	}
	return node;
}

static size_t
node_leafs(struct RopeNode *node) {
	if (node->type == ROPE_NODE_BRANCH) {
		return node->data.branch.leafs;
	} else {
		return 1;
	}
}

static size_t
node_depth(struct RopeNode *node) {
	size_t depth = 0;
	while (node->parent != NULL) {
		depth++;
		node = node->parent;
	}
	return depth;
}

enum RopeNodeDirection
rope_node_which(struct RopeNode *node) {
	if (*node_left(node->parent) == node) {
		return ROPE_NODE_LEFT;
	} else {
		return ROPE_NODE_RIGHT;
	}
}

static struct RopeNode **
node_sibling(struct RopeNode *node) {
	if (node->parent == NULL) {
		return NULL;
	}
	return node_child(node->parent, !rope_node_which(node));
}

static struct RopeNode *
node_neighbour(struct RopeNode *node, enum RopeNodeDirection direction) {
	while (node->parent != NULL) {
		if (rope_node_which(node) == !direction) {
			node = *node_sibling(node);
			break;
		}
		node = node->parent;
	}
	if (node->parent == NULL) {
		return NULL;
	}
	while (node->type == ROPE_NODE_BRANCH) {
		node = *node_child(node, !direction);
	}
	return node;
}

static size_t
count_lines(const uint8_t *data, size_t byte_size) {
	const uint8_t *ptr = data;
	size_t count = 0;
	while (true) {
		ptr = memchr(ptr, NEWLINE, byte_size - (ptr - data));
		if (ptr == NULL) {
			return count;
		}
		count++;
		ptr++;
	}
}

static void
node_update_leaf(struct RopeNode *node) {
	size_t byte_size;
	const uint8_t *data = rope_node_value(node, &byte_size);
	node->char_size = cx_utf8_clen(data, byte_size);
	node->utf16_size = cx_utf8_16len(data, byte_size);

	node->new_lines = count_lines(data, byte_size);
}

static void
node_update(struct RopeNode *node) {
	if (node->type == ROPE_NODE_BRANCH) {
		(*node_left(node))->parent = node;
		(*node_right(node))->parent = node;
	} else {
		node_update_leaf(node);
		node = node->parent;
	}

	for (; node; node = node->parent) {
		struct RopeNode *left = *node_left(node);
		struct RopeNode *right = *node_right(node);
		node->byte_size = left->byte_size + right->byte_size;
		node->char_size = left->char_size + right->char_size;
		node->utf16_size = left->utf16_size + right->utf16_size;
		node->new_lines = left->new_lines + right->new_lines;
		node->data.branch.leafs = node_leafs(left) + node_leafs(right);
	}
}

static struct RopeNode *
node_find_insertion_point(
		struct RopeNode *node, enum RopeNodeDirection *which) {
	struct RopeNode *neighbour = node_neighbour(node, *which);
	if (neighbour != NULL && node_depth(neighbour) < node_depth(node)) {
		node = neighbour;
		*which = !*which;
	}

	for (; node->parent; node = node->parent) {
		if (rope_node_which(node) != *which) {
			break;
		}
	}

	for (; node->type == ROPE_NODE_BRANCH;) {
		struct RopeNode *bad_child = *node_child(node, !*which);
		struct RopeNode *good_child = *node_child(node, *which);
		if (node_leafs(good_child) >= node_leafs(bad_child)) {
			break;
		}
		node = good_child;
	}
	return node;
}

struct RopeNode *
rope_node_new(struct RopePool *pool) {
	struct RopeNode *node = rope_pool_get(pool);
	if (!node) {
		return NULL;
	}
	memset(node, 0, sizeof(struct RopeNode));
	return node;
}

static int
node_set_inline(struct RopeNode *node, const uint8_t *data, size_t byte_size) {
	node->type = ROPE_NODE_INLINE_LEAF;
	memcpy(node->data.inline_leaf.data, data, byte_size);
	node->byte_size = byte_size;
	node_update_leaf(node);
	return 0;
}

int
rope_node_set_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	assert(node->type != ROPE_NODE_BRANCH);
	assert(node->byte_size == 0);

	if (byte_size <= ROPE_INLINE_LEAF_SIZE) {
		rv = node_set_inline(node, data, byte_size);
	} else {
		struct RopeRcString *owned = rope_rc_string_new(data, byte_size);
		if (!owned) {
			return -1;
		}
		rv = rope_node_set_rc_string(node, owned, 0);
		rope_rc_string_release(owned);
	}
	return rv;
}

int
rope_node_set_rc_string(
		struct RopeNode *node, struct RopeRcString *str, size_t byte_index) {
	assert(node->type != ROPE_NODE_BRANCH);
	assert(node->byte_size == 0);

	node->type = ROPE_NODE_LEAF;
	node->data.leaf.owned = rope_rc_string_retain(str);
	size_t byte_size;
	const uint8_t *data = rope_rc_string(str, &byte_size);
	node->data.leaf.data = &data[byte_index];

	if (ROPE_OVERFLOW_SUB(byte_size, byte_index, &node->byte_size)) {
		return -1;
	}

	node_update_leaf(node);
	return 0;
}

static void
node_move(struct RopeNode *target, struct RopeNode *source) {
	memcpy(target, source, sizeof(struct RopeNode));
	memset(&source->data, 0, sizeof(source->data));
	node_update(target);
}

static int
node_insert(
		struct RopeNode *parent, struct RopeNode *new_node,
		struct RopePool *pool, enum RopeNodeDirection which) {
	if (parent->byte_size == 0) {
		new_node->parent = parent->parent;
		node_move(parent, new_node);
		rope_node_free(new_node, pool);
	} else {
		parent = node_find_insertion_point(parent, &which);

		struct RopeNode *sibling = rope_pool_get(pool);
		if (!sibling) {
			return -1;
		}
		node_move(sibling, parent);

		new_node->parent = sibling->parent = parent;
		parent->type = ROPE_NODE_BRANCH;
		*node_child(parent, !which) = sibling;
		*node_child(parent, which) = new_node;

		node_update(parent);
	}
	rope_node_print(parent, "/tmp/node_insert.dot");

	return 0;
}

int
rope_node_insert_left(
		struct RopeNode *parent, struct RopeNode *new_node,
		struct RopePool *pool) {
	return node_insert(parent, new_node, pool, ROPE_NODE_LEFT);
}

int
rope_node_insert_right(
		struct RopeNode *parent, struct RopeNode *new_node,
		struct RopePool *pool) {
	return node_insert(parent, new_node, pool, ROPE_NODE_RIGHT);
}

int
rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr) {
	int rv = 0;
	struct RopeNode *left = NULL, *right = NULL;

	assert(node->type != ROPE_NODE_BRANCH);
	left = rope_pool_get(pool);
	if (rv < 0) {
		goto out;
	}
	right = rope_pool_get(pool);
	if (rv < 0) {
		goto out;
	}

	size_t size;
	const uint8_t *value = rope_node_value(node, &size);
	size_t char_index = cx_utf8_clen(value, byte_index);
	size_t utf16_size = cx_utf8_16len(value, byte_index);

	left->type = node->type;
	left->byte_size = byte_index;
	left->char_size = char_index;
	left->utf16_size = utf16_size;

	right->type = node->type;
	right->byte_size = node->byte_size - byte_index;
	right->char_size = node->char_size - char_index;
	right->utf16_size = node->utf16_size - utf16_size;

	if (node->type == ROPE_NODE_LEAF) {
		left->data.leaf.data = value;
		left->data.leaf.owned = node->data.leaf.owned;
		right->data.leaf.data = &value[byte_index];
		right->data.leaf.owned = node->data.leaf.owned;
		rope_rc_string_retain(node->data.leaf.owned);
		node->data.leaf.owned = NULL;
	} else {
		memcpy(left->data.inline_leaf.data, value, ROPE_INLINE_LEAF_SIZE);
		memcpy(right->data.inline_leaf.data, &value[byte_index],
			   right->byte_size);
	}

	node->type = ROPE_NODE_BRANCH;
	node->data.branch.leafs = 0;
	*node_left(node) = left;
	*node_right(node) = right;
	node_update(node);

	if (left_ptr) {
		*left_ptr = left;
	}
	if (right_ptr) {
		*right_ptr = right;
	}
	left = NULL;
	right = NULL;

out:
	rope_node_free(left, pool);
	rope_node_free(right, pool);
	return rv;
}

int
rope_node_merge(
		struct RopeNode *target, struct RopeNode *extra,
		struct RopePool *pool) {
	assert(target->type == ROPE_NODE_INLINE_LEAF);
	assert(extra->type == ROPE_NODE_INLINE_LEAF);
	assert(target->byte_size + extra->byte_size <= ROPE_INLINE_LEAF_SIZE);

	memcpy(&target->data.inline_leaf.data[target->byte_size],
		   extra->data.inline_leaf.data, extra->byte_size);
	target->byte_size += extra->byte_size;
	node_update(target);
	// TODO: maybe detach extra too?
	rope_node_free(extra, pool);
	return 0;
}

static struct RopeNode *
find_line(
		struct RopeNode *node, rope_index_t line,
		rope_byte_index_t *char_index) {
	// Quickpath for the first line.
	if (line == 0) {
		*char_index = 0;
		return rope_node_first(node);
	}

	for (; node && node->type == ROPE_NODE_BRANCH;) {
		struct RopeNode *left = *node_left(node);
		if (left->new_lines < line) {
			line -= left->new_lines;
			node = *node_right(node);
		} else {
			node = left;
		}
	}

	size_t byte_size = 0;
	const uint8_t *value = rope_node_value(node, &byte_size);
	size_t remaining_size = byte_size;
	const uint8_t *remaining = value;
	for (; line;) {
		remaining = memchr(remaining, NEWLINE, remaining_size);
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
		rope_byte_index_t *byte_index) {
	rope_index_t drop_chars = 0;
	node = find_line(node, line, &drop_chars);
	column += drop_chars;

	while (node->char_size <= column) {
		column -= node->char_size;
		bool has_next = rope_node_next(&node);
		assert(has_next);
	}

	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);

	*byte_index = cx_utf8_bidx(value, size, column);
	return node;
}

struct RopeNode *
rope_node_find_char(
		struct RopeNode *node, rope_index_t char_index,
		rope_byte_index_t *byte_index) {
	*byte_index = 0;
	for (; node && node->type == ROPE_NODE_BRANCH;) {
		struct RopeNode *left = *node_left(node);
		if (left->char_size >= char_index) {
			node = left;
		} else {
			char_index -= left->char_size;
			node = *node_right(node);
		}
	}
	if (node) {
		size_t size = 0;
		const uint8_t *value = rope_node_value(node, &size);
		ssize_t trailing_bytes = cx_utf8_bidx(value, size, char_index);
		assert(trailing_bytes >= 0);
		*byte_index = trailing_bytes;
	}
	return node;
}

struct RopeNode *
rope_node_last(struct RopeNode *node) {
	return node_outer(node, ROPE_NODE_RIGHT);
}

struct RopeNode *
rope_node_first(struct RopeNode *node) {
	return node_outer(node, ROPE_NODE_LEFT);
}

struct RopeNode *
rope_node_sibling(struct RopeNode *node) {
	return *node_sibling(node);
}

enum RopeNodeType
rope_node_type(struct RopeNode *node) {
	return node->type;
}

struct RopeNode *
rope_node_parent(struct RopeNode *node) {
	return node->parent;
}

bool
rope_node_next(struct RopeNode **node) {
	*node = node_neighbour(*node, ROPE_NODE_RIGHT);
	return *node != NULL;
}

bool
rope_node_prev(struct RopeNode **node) {
	*node = node_neighbour(*node, ROPE_NODE_LEFT);
	return *node != NULL;
}

bool
rope_node_up(struct RopeNode **node) {
	*node = (*node)->parent;
	return *node != NULL;
}

struct RopeNode *
rope_node_left(struct RopeNode *node) {
	return *node_left(node);
}

struct RopeNode *
rope_node_right(struct RopeNode *node) {
	return *node_right(node);
}

const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size) {
	switch (node->type) {
	case ROPE_NODE_LEAF:
		*size = node->byte_size;
		return node->data.leaf.data;
	case ROPE_NODE_INLINE_LEAF:
		*size = node->byte_size;
		return node->data.inline_leaf.data;
	case ROPE_NODE_BRANCH:
		*size = 0;
		return NULL;
	}
	assert(false);
	return NULL;
}

int
rope_node_delete(struct RopeNode *node, struct RopePool *pool) {
	struct RopeNode *parent = node->parent;

	if (parent == NULL) {
		memset(node, 0, sizeof(struct RopeNode));
	} else {
		struct RopeNode *sibling = *node_sibling(node);
		sibling->parent = parent->parent;
		memcpy(parent, sibling, sizeof(struct RopeNode));
		memset(sibling, 0, sizeof(struct RopeNode));
		rope_node_free(node, pool);
		rope_node_free(sibling, pool);
		node_update(parent);
	}
	return 0;
}

uint64_t
rope_node_tags(struct RopeNode *node) {
	while (node->type == ROPE_NODE_BRANCH) {
		node = *node_left(node);
	}
	return node->data.leaf.tags;
}

void
rope_node_set_tags(struct RopeNode *node, uint64_t tags) {
	if (node->type == ROPE_NODE_BRANCH) {
		rope_node_set_tags(*node_left(node), tags);
		rope_node_set_tags(*node_right(node), tags);
	} else {
		node->data.leaf.tags = tags;
	}
}

int
rope_node_free(struct RopeNode *node, struct RopePool *pool) {
	if (node == NULL) {
		return 0;
	}
	switch (node->type) {
	case ROPE_NODE_LEAF:
		rope_rc_string_release(node->data.leaf.owned);
		break;
	case ROPE_NODE_BRANCH:
		rope_node_free(*node_left(node), pool);
		rope_node_free(*node_right(node), pool);
		break;
	default:
		break;
	}
	return rope_pool_recycle(pool, node);
}

void
print_escaped(const uint8_t *data, size_t size, FILE *out) {
	for (size_t i = 0; i < size; i++) {
		switch (data[i]) {
		case '\n':
			fputs("\\n", out);
			break;
		case '\r':
			fputs("\\r", out);
			break;
		case '\t':
			fputs("\\t", out);
			break;
		case '"':
			fputs("\\\"", out);
			break;
		case '\\':
			fputs("\\\\", out);
			break;
		default:
			fputc(data[i], out);
			break;
		}
	}
}

static void
print_node(struct RopeNode *node, FILE *out) {
	fprintf(out, "node%p", (void *)node);
	if (node == NULL) {
		fprintf(out, " [label=\"NULL\"]\n");
		return;
	}
	switch (node->type) {
	case ROPE_NODE_LEAF:
		fputs(" [label=\"leaf ", out);
		print_escaped(node->data.leaf.data, node->byte_size, out);
		fprintf(out, " %lu\"]\n", node->char_size);
		break;
	case ROPE_NODE_INLINE_LEAF:
		fputs(" [label=\"inline_leaf ", out);
		print_escaped(node->data.inline_leaf.data, node->byte_size, out);
		fprintf(out, " %lu\"]\n", node->char_size);
		break;
	case ROPE_NODE_BRANCH:
		fprintf(out, " [label=\"branch %lu\"]\n", node->char_size);
		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)*node_left(node));
		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)*node_right(node));
		print_node(*node_left(node), out);
		print_node(*node_right(node), out);
		break;
	}
}

void
rope_node_print(struct RopeNode *root, const char *file) {
	FILE *out = fopen(file, "w");
	if (!out) {
		return;
	}
	while (root->parent) {
		root = root->parent;
	}
	fputs("digraph G {\n", out);
	print_node(root, out);
	fputs("}\n", out);
	fclose(out);
}
