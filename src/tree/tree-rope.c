#include <cextras/collection.h>
#include <rope.h>
#include <tree.h>

static void *
e_rope_down(void *node, void *user_data) {
	(void)user_data;
	struct RopeNode *rope_node = node;

	if (rope_node_type(rope_node) == ROPE_NODE_BRANCH) {
		return rope_node_left(rope_node);
	} else {
		return NULL;
	}
}

static void *
e_rope_up(void *node, void *user_data) {
	(void)user_data;
	struct RopeNode *rope_node = node;

	return rope_node_parent(rope_node);
}

static void *
e_rope_next_sibling(void *node, void *user_data) {
	(void)user_data;
	struct RopeNode *rope_node = node;

	if (rope_node_parent(rope_node) == NULL) {
		return NULL;
	} else if (rope_node_which(rope_node) == ROPE_NODE_LEFT) {
		return rope_node_sibling(rope_node);
	} else {
		return NULL;
	}
}

static int
e_rope_label(void *node, struct CxBuffer *buffer, void *user_data) {
	(void)user_data;
	struct RopeNode *rope_node = node;

	if (rope_node_type(rope_node) == ROPE_NODE_BRANCH) {
		return cx_buffer_append(buffer, (uint8_t *)"branch", 6);
	} else {
		size_t size = 0;
		const uint8_t *label = rope_node_value(rope_node, &size);
		if (label[size - 1] == '\n') {
			size--;
		}
		return cx_buffer_append(buffer, label, size);
	}
}

const struct ETreeVisitorImpl e_tree_visitor_rope_impl = {
		.down = e_rope_down,
		.up = e_rope_up,
		.next_sibling = e_rope_next_sibling,
		.label = e_rope_label,
};
