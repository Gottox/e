#include <alloca.h>
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
		ptr = memchr(ptr, ROPE_NEWLINE, byte_size - (ptr - data));
		if (ptr == NULL) {
			return count;
		}
		count++;
		ptr++;
	}
}

static void
node_update(struct RopeNode *node) {
	if (node->type == ROPE_NODE_BRANCH) {
		(*node_left(node))->parent = node;
		(*node_right(node))->parent = node;
	} else {
		size_t byte_size;
		const uint8_t *data = rope_node_value(node, &byte_size);
		node->char_size = cx_utf8_clen(data, byte_size);
		node->utf16_size = cx_utf8_16len(data, byte_size);
		node->new_lines = count_lines(data, byte_size);

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
	node_update(node);
	return 0;
}

bool
rope_node_match_tags(struct RopeNode *node, uint64_t tags) {
	assert(node->type != ROPE_NODE_BRANCH);
	uint64_t node_tags = rope_node_tags(node);
	return (node_tags & tags) == tags;
}

int
rope_node_set_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	assert(node->type != ROPE_NODE_BRANCH);
	if (node->type == ROPE_NODE_LEAF) {
		rope_rc_string_release(node->data.leaf.owned);
	}

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

	if (node->type == ROPE_NODE_LEAF) {
		rope_rc_string_release(node->data.leaf.owned);
	}

	node->type = ROPE_NODE_LEAF;
	node->data.leaf.owned = rope_rc_string_retain(str);
	size_t byte_size;
	const uint8_t *data = rope_rc_string(str, &byte_size);
	node->data.leaf.data = &data[byte_index];

	if (ROPE_OVERFLOW_SUB(byte_size, byte_index, &node->byte_size)) {
		return -1;
	}

	node_update(node);
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

static int
node_inline_insert(
		struct RopeNode *target, struct RopeNode *extra,
		struct RopePool *pool) {
	if (target->type != ROPE_NODE_INLINE_LEAF ||
		extra->type != ROPE_NODE_INLINE_LEAF) {
		return -1;
	}
	if (target->byte_size + extra->byte_size > ROPE_INLINE_LEAF_SIZE) {
		return -1;
	}
	if (rope_node_tags(target) != rope_node_tags(extra)) {
		return -1;
	}

	memcpy(&target->data.inline_leaf.data[target->byte_size],
		   extra->data.inline_leaf.data, extra->byte_size);
	target->byte_size += extra->byte_size;
	node_update(target);
	// TODO: maybe detach extra too?
	rope_node_free(extra, pool);
	return 0;
}

int
rope_node_insert_left(
		struct RopeNode *target, struct RopeNode *new_node,
		struct RopePool *pool) {
	int rv = -1;
	struct RopeNode *prev = node_neighbour(target, ROPE_NODE_LEFT);
	if (prev) {
		rv = node_inline_insert(prev, new_node, pool);
	}
	if (rv < 0) {
		rv = node_insert(target, new_node, pool, ROPE_NODE_LEFT);
	}
	return rv;
}

int
rope_node_insert_right(
		struct RopeNode *target, struct RopeNode *new_node,
		struct RopePool *pool) {
	int rv = 0;
	rv = node_inline_insert(target, new_node, pool);
	if (rv < 0) {
		rv = node_insert(target, new_node, pool, ROPE_NODE_RIGHT);
	}
	return rv;
}

int
rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr) {
	int rv = 0;
	struct RopeNode *left = NULL, *right = NULL;

	size_t size;
	const uint8_t *value = rope_node_value(node, &size);

	if (byte_index == 0) {
		left = NULL;
		right = node;
		goto out;
	} else if (byte_index == node->byte_size) {
		left = node;
		right = NULL;
		goto out;
	}

	assert(node->type != ROPE_NODE_BRANCH);
	left = rope_pool_get(pool);
	if (rv < 0) {
		goto out;
	}
	right = rope_pool_get(pool);
	if (rv < 0) {
		goto out;
	}

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

out:
	if (rv >= 0) {
		if (left_ptr) {
			*left_ptr = left;
		}
		if (right_ptr) {
			*right_ptr = right;
		}
		left = NULL;
		right = NULL;
	}

	rope_node_free(left, pool);
	rope_node_free(right, pool);
	return rv;
}

static int
node_merge_direction(
		struct RopeNode *node, enum RopeNodeDirection which,
		struct RopePool *pool) {
	int rv = 0;
	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;
	struct RopeRcString *merged = NULL;

	struct RopeNode *neighbour = node_neighbour(node, which);
	if (neighbour == NULL) {
		goto out;
	}

	if (rope_node_tags(node) != rope_node_tags(neighbour)) {
		rv = -1;
		goto out;
	}

	if (which == ROPE_NODE_LEFT) {
		right = node;
		left = neighbour;
	} else {
		left = node;
		right = neighbour;
	}

	size_t left_size = 0;
	size_t right_size = 0;
	const uint8_t *left_value = rope_node_value(left, &left_size);
	const uint8_t *right_value = rope_node_value(right, &right_size);

	if (left_size + right_size <= ROPE_INLINE_LEAF_SIZE) {
		rv = rope_node_set_value(left, (uint8_t *)"", 0);
		if (rv < 0) {
			goto out;
		}
		memcpy(left->data.inline_leaf.data, left_value, left_size);
		memcpy(&left->data.inline_leaf.data[left_size], right_value,
			   right_size);
		left->byte_size = left_size + right_size;
		node_update(left);
	} else {
		merged = rope_rc_string_new2(
				left_value, left_size, right_value, right_size);
		if (merged == NULL) {
			rv = -1;
			goto out;
		}
		rv = rope_node_set_rc_string(left, merged, 0);
		if (rv < 0) {
			goto out;
		}
	}
	rv = rope_node_delete(right, pool);
	if (rv < 0) {
		goto out;
	}
out:
	rope_rc_string_release(merged);
	return rv;
}

int
rope_node_merge_left(struct RopeNode *node, struct RopePool *pool) {
	return node_merge_direction(node, ROPE_NODE_LEFT, pool);
}

int
rope_node_merge_right(struct RopeNode *node, struct RopePool *pool) {
	return node_merge_direction(node, ROPE_NODE_RIGHT, pool);
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
		if (line <= node->new_lines) {
			break;
		}
		line -= node->new_lines;
	} while (rope_node_next(&node));

	size_t byte_size = 0;
	const uint8_t *value = rope_node_value(node, &byte_size);
	// Fastpath for new lines at the end of the node.
	if (line == node->new_lines && value[byte_size - 1] == ROPE_NEWLINE) {
		struct RopeNode *next_node = node;
		if (rope_node_next(&next_node)) {
			*char_index = 0;
			return next_node;
		} else {
			*char_index = node->char_size;
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
/*static struct RopeNode *
find_line(
		struct RopeNode *node, rope_index_t line,
		rope_byte_index_t *char_index, uint64_t tags) {
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
		remaining = memchr(remaining, ROPE_NEWLINE, remaining_size);
		assert(remaining != NULL);
		line--;
		remaining++; // Skip the newline character.
		remaining_size = byte_size - (remaining - value);
	}
	*char_index = cx_utf8_bidx(value, byte_size, remaining - value);
	return node;
}*/

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
		if (node->char_size > column) {
			break;
		}
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
		struct RopeNode *node, rope_index_t char_index, uint64_t tags,
		rope_byte_index_t *byte_index) {
	*byte_index = 0;
	if (tags == 0) {
		for (; node && node->type == ROPE_NODE_BRANCH;) {
			struct RopeNode *left = *node_left(node);
			if (left->char_size >= char_index) {
				node = left;
			} else {
				char_index -= left->char_size;
				node = *node_right(node);
			}
		}
	} else {
		node = rope_node_first(node);
		do {
			if (rope_node_match_tags(node, tags) == false) {
				continue;
			}
			if (node->char_size > char_index) {
				break;
			}
			char_index -= node->char_size;
		} while (rope_node_next(&node));
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
	assert(node->type != ROPE_NODE_BRANCH);
	switch (node->type) {
	case ROPE_NODE_INLINE_LEAF:
		return node->data.inline_leaf.tags;
	case ROPE_NODE_LEAF:
		return node->data.leaf.tags;
	default:
		__builtin_unreachable();
	}
}

int
rope_node_delete_by_tags(
		struct RopeNode *node, struct RopePool *pool, uint64_t tags) {
	int rv = 0;
	assert(rope_node_type(node) == ROPE_NODE_BRANCH);

	node = rope_node_first(node);
	while (node) {
		if (!rope_node_match_tags(node, tags)) {
			rope_node_next(&node);
			continue;
		}
		struct RopeNode *parent = rope_node_parent(node);
		rv = rope_node_delete(node, pool);
		if (rv < 0) {
			goto out;
		}
		node = parent;
	}
out:
	return rv;
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

void
rope_node_add_tags(struct RopeNode *node, uint64_t tags) {
	if (node->type == ROPE_NODE_BRANCH) {
		rope_node_add_tags(*node_left(node), tags);
		rope_node_add_tags(*node_right(node), tags);
	} else {
		rope_node_set_tags(node, rope_node_tags(node) | tags);
	}
}

void
rope_node_remove_tags(struct RopeNode *node, uint64_t tags) {
	if (node->type == ROPE_NODE_BRANCH) {
		rope_node_remove_tags(*node_left(node), tags);
		rope_node_remove_tags(*node_right(node), tags);
	} else {
		rope_node_set_tags(node, rope_node_tags(node) & ~tags);
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
