#include <assert.h>
#include <cextras/unicode.h>
#include <rope.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////
/// struct RopeNode

int
rope_node_init(struct RopeNode *node) {
	memset(node, 0, sizeof(*node));
	return 0;
}

static void
node_set_left(struct RopeNode *base, struct RopeNode *new_left) {
	base->data.fork.left = new_left;
	new_left->parent = base;
}

static void
node_set_right(struct RopeNode *base, struct RopeNode *new_right) {
	base->data.fork.right = new_right;
	new_right->parent = base;
}

static void
node_update(struct RopeNode *node) {
	while (node->parent != NULL) {
		node = node->parent;
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);
		node->char_size = left->char_size + right->char_size;
		node->byte_size = left->byte_size + right->byte_size;
		node->new_lines = left->new_lines + right->new_lines;
	}
}

struct RopeNode *
rope_node_left(struct RopeNode *base) {
	return base->data.fork.left;
}

struct RopeNode *
rope_node_right(struct RopeNode *base) {
	return base->data.fork.right;
}

int
rope_node_split(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index) {
	int rv = 0;
	size_t size;
	struct RopeNode *left = NULL, *right = NULL;

	assert(node->type != ROPE_NODE_LEAF || node->type != ROPE_NODE_INLINE_LEAF);
	left = rope_pool_get(&rope->pool);
	if (rv < 0) {
		goto out;
	}
	right = rope_pool_get(&rope->pool);
	if (rv < 0) {
		goto out;
	}

	const uint8_t *value = rope_node_value(node, &size);
	size_t byte_index = cx_utf8_bidx(value, size, index);
	size_t utf16_size = cx_utf8_16len(value, byte_index);

	left->type = node->type;
	left->byte_size = byte_index;
	left->char_size = index;
	left->utf16_size = utf16_size;

	right->type = node->type;
	right->byte_size = node->byte_size - byte_index;
	right->char_size = node->char_size - index;
	right->utf16_size = node->utf16_size - utf16_size;

	if (node->type == ROPE_NODE_LEAF) {
		left->data.leaf.data = node->data.leaf.data;
		right->data.leaf.data = node->data.leaf.data + byte_index;
	} else {
		memcpy(left->data.inline_leaf, node->data.inline_leaf,
			   ROPE_INLINE_LEAF_SIZE);
		memcpy(right->data.inline_leaf, &node->data.inline_leaf[byte_index],
			   right->byte_size);

		// Make sure we don't interpret the inline data as a pointer
		node->data.fork.owned_data = NULL;
	}

	node->type = ROPE_NODE_FORK;
	node_set_left(node, left);
	node_set_right(node, right);
	left = NULL;
	right = NULL;

out:
	rope_pool_recycle(&rope->pool, left);
	rope_pool_recycle(&rope->pool, right);
	return rv;
}

int
rope_node_replace(
		struct RopeNode *node, struct Rope *rope, struct RopeNode *new_node) {
	if (node->parent == NULL) {
		rope->root = new_node;
	} else if (rope_node_left(node->parent) == node) {
		node_set_left(node->parent, new_node);
	} else {
		node_set_right(node->parent, new_node);
	}
	return rope_pool_recycle(&rope->pool, node);
}

int
rope_node_delete(struct RopeNode *node, struct Rope *rope) {
	int rv = 0;
	struct RopeNode *parent = node->parent;

	if (parent == NULL) {
		rope_node_cleanup(rope->root);
		rope_node_init(rope->root);
		goto out;
	}

	struct RopeNode *left = rope_node_left(parent);
	struct RopeNode *right = rope_node_right(parent);
	if (left == node) {
		rope_node_replace(parent, rope, right);
	} else {
		rope_node_replace(parent, rope, left);
	}
	rope_pool_recycle(&rope->pool, node);
out:
	return rv;
}

struct RopeNode *
rope_node_find_line(struct RopeNode *node, rope_index_t *line) {
	if (*line > node->new_lines) {
		return NULL;
	}

	while (node->type == ROPE_NODE_FORK) {
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);
		size_t left_lines = left->new_lines;
		if (*line <= left_lines) {
			node = rope_node_left(node);
		} else {
			*line -= left_lines;
			node = right;
		}
	}
	return node;
}

struct RopeNode *
rope_node_find(struct RopeNode *node, rope_char_index_t *index) {
	if (*index > node->char_size) {
		return NULL;
	}

	while (node->type == ROPE_NODE_FORK) {
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);
		size_t left_size = left->char_size;
		if (*index <= left_size) {
			node = rope_node_left(node);
		} else {
			*index -= left_size;
			node = right;
		}
	}
	return node;
}

struct CongenericFn {
	struct RopeNode *(*forward)(struct RopeNode *);
	struct RopeNode *(*backward)(struct RopeNode *);
};

static bool
node_sibling(struct RopeNode **node, const struct CongenericFn *child_fns) {
	struct RopeNode *node2 = *node;
	while (node2->parent != NULL) {
		if (child_fns->backward(node2->parent) == node2) {
			node2 = child_fns->forward(node2->parent);
			break;
		}
		node2 = node2->parent;
	}
	if (node2->parent == NULL) {
		*node = NULL;
		return false;
	}
	while (node2->type == ROPE_NODE_FORK) {
		node2 = child_fns->backward(node2);
	}
	*node = node2;
	return true;
}
bool
rope_node_next(struct RopeNode **node) {
	static const struct CongenericFn child_fns = {
			rope_node_right,
			rope_node_left,
	};
	return node_sibling(node, &child_fns);
}

bool
rope_node_prev(struct RopeNode **node) {
	static const struct CongenericFn child_fns = {
			rope_node_left,
			rope_node_right,
	};
	return node_sibling(node, &child_fns);
}

static int
rope_node_insert_under(
		struct RopeNode *node, struct Rope *rope, struct RopeNode *new_node,
		enum RopeBias bias) {
	struct RopeNode *new_parent = rope_pool_get(&rope->pool);
	if (new_parent == NULL) {
		return -1;
	}
	new_parent->type = ROPE_NODE_FORK;
	if (node->parent == NULL) {
		rope->root = new_parent;
	} else if (rope_node_left(node->parent) == node) {
		node_set_left(node->parent, new_parent);
	} else {
		node_set_right(node->parent, new_parent);
	}

	if (bias == ROPE_BIAS_LEFT) {
		node_set_left(new_parent, node);
		node_set_right(new_parent, new_node);
	} else {
		node_set_left(new_parent, node);
		node_set_right(new_parent, new_node);
	}

	node_update(new_node);
	return 0;
}

int
rope_node_insert_right(
		struct RopeNode *node, struct Rope *rope, struct RopeNode *new_node) {
	return rope_node_insert_under(node, rope, new_node, ROPE_BIAS_RIGHT);
}

int
rope_node_insert_left(
		struct RopeNode *node, struct Rope *rope, struct RopeNode *new_node) {
	return rope_node_insert_under(node, rope, new_node, ROPE_BIAS_LEFT);
}

static struct RopeNode *
node_weighted_find(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t *index) {
	(void)rope;
	if (*index > node->char_size) {
		return NULL;
	}

	while (node->type == ROPE_NODE_FORK) {
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);
		bool left_leaning = left->char_size > right->char_size;
		if (left_leaning && *index == 0) {
			break;
		} else if (!left_leaning && *index == node->char_size) {
			break;
		} else if (*index < left->char_size) {
			node = left;
		} else {
			*index -= left->char_size;
			node = right;
		}
	}
	return node;
}

static ssize_t
count_new_lines(const uint8_t *data, size_t size) {
	ssize_t count = 0;
	for (const uint8_t *p = data; (p = memchr(p, '\n', size));) {
		size -= p - data + 1;
		count++;
		p++;
		data = p;
	}

	return count;
}

static struct RopeNode *
rope_node_new_leaf(struct Rope *rope, const uint8_t *data, size_t byte_size) {
	struct RopeNode *node = rope_pool_get(&rope->pool);
	if (node == NULL) {
		return NULL;
	}

	node->byte_size = byte_size;
	node->char_size = cx_utf8_clen(data, byte_size);
	node->utf16_size = cx_utf8_16len(data, byte_size);
	node->new_lines = count_new_lines(data, byte_size);

	if (byte_size <= ROPE_INLINE_LEAF_SIZE) {
		node->type = ROPE_NODE_INLINE_LEAF;
		memcpy(node->data.inline_leaf, data, byte_size);
	} else {
		node->type = ROPE_NODE_LEAF;
		node->data.leaf.data = node->data.leaf.owned_data = malloc(byte_size);
		if (node->data.leaf.owned_data == NULL) {
			rope_pool_recycle(&rope->pool, node);
			return NULL;
		}
		memcpy(node->data.leaf.owned_data, data, byte_size);
	}
	return node;
}

size_t
rope_node_line_number(const struct RopeNode *node, rope_char_index_t index) {
	size_t line_number = 0;
	size_t value_size = 0;

	const uint8_t *value = rope_node_value(node, &value_size);
	for (const uint8_t *p = value; (p = memchr(p, '\n', index));) {
		index -= p - value + 1;
		line_number++;
		p++;
		value = p;
	}

	for (struct RopeNode *parent; (parent = node->parent);) {
		if (rope_node_right(parent) == node) {
			line_number += rope_node_left(parent)->new_lines;
		}
		node = parent;
	}

	return line_number;
}

int
rope_node_insert(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index,
		const uint8_t *data, size_t byte_size) {
	int rv = 0;
	node = node_weighted_find(node, rope, &index);

	struct RopeNode *new_node = rope_node_new_leaf(rope, data, byte_size);
	if (new_node == NULL) {
		rv = -1;
		goto out;
	}
	if (index == 0) {
		if (node->byte_size == 0) {
			rv = rope_node_replace(node, rope, new_node);
		} else {
			rv = rope_node_insert_left(node, rope, new_node);
		}
	} else if (index == node->char_size) {
		rv = rope_node_insert_right(node, rope, new_node);
	} else {
		int rv = 0;
		rv = rope_node_split(node, rope, index);
		if (rv < 0) {
			goto out;
		}

		if (rope->bias == ROPE_BIAS_LEFT) {
			rv = rope_node_insert_right(rope_node_left(node), rope, new_node);
			rope->bias = ROPE_BIAS_RIGHT;
		} else {
			rv = rope_node_insert_left(rope_node_right(node), rope, new_node);
			rope->bias = ROPE_BIAS_LEFT;
		}
	}
	if (rv < 0) {
		goto out;
	}

	node_update(new_node);

out:
	return 0;
}

struct RopeNode *
rope_first(struct Rope *rope) {
	struct RopeNode *node = rope->root;
	while (node->type == ROPE_NODE_FORK) {
		node = node->data.fork.left;
	}
	return node;
}

int
rope_node_cleanup(struct RopeNode *node) {
	int rv = 0;

	switch (node->type) {
	case ROPE_NODE_LEAF:
		free(node->data.leaf.owned_data);
		node->data.leaf.owned_data = NULL;
		break;
	case ROPE_NODE_FORK:
		free(node->data.fork.owned_data);
		node->data.fork.owned_data = NULL;
		break;
	default:
		break;
	}
	memset(node, 0, sizeof(*node));

	return rv;
}

const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size) {
	*size = node->byte_size;
	switch (node->type) {
	case ROPE_NODE_LEAF:
		return node->data.leaf.data;
	case ROPE_NODE_INLINE_LEAF:
		return node->data.inline_leaf;
	default:
		*size = 0;
		return NULL;
	}
}

static void
print_node(struct RopeNode *node, FILE *out) {
	fprintf(out, "node%p", (void *)node);
	switch (node->type) {
	case ROPE_NODE_LEAF:
		fprintf(out, " [label=\"leaf %lu\"]\n", node->char_size);
		break;
	case ROPE_NODE_INLINE_LEAF:
		fprintf(out, " [label=\"inline_leaf %lu\"]\n", node->char_size);
		break;
	case ROPE_NODE_FORK:
		fprintf(out, " [label=\"fork %lu\"]\n", node->char_size);
		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)node->data.fork.left);
		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)node->data.fork.right);
		print_node(node->data.fork.left, out);
		print_node(node->data.fork.right, out);
		break;
	}
}
void
rope_print_tree(struct Rope *rope, FILE *out) {
	fputs("digraph G {\n", out);
	print_node(rope->root, out);
	fputs("}\n", out);
}
