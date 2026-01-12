#include <rope.h>
#include <stdio.h>

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
	switch (rope_node_type(node)) {
	case ROPE_NODE_LEAF:
		fputs(" [label=\"leaf ", out);
		print_escaped(node->data.leaf.data, rope_node_byte_size(node), out);
		fprintf(out, " %lu", rope_node_char_size(node));
		fputs("\"]\n", out);
		break;
	case ROPE_NODE_INLINE_LEAF:
		fputs(" [label=\"inline_leaf ", out);
		print_escaped(
				node->data.inline_leaf.data, rope_node_byte_size(node), out);
		fprintf(out, " %lu", rope_node_char_size(node));
		fputs("\"]\n", out);
		break;
	case ROPE_NODE_BRANCH:
		fprintf(out, " [label=\"branch %lu\n", rope_node_char_size(node));
		fprintf(out, " %lu", rope_node_depth(node));
		fputs("\"]\n", out);

		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)rope_node_left(node));
		fprintf(out, "node%p -> node%p\n", (void *)node,
				(void *)rope_node_right(node));
		print_node(rope_node_left(node), out);
		print_node(rope_node_right(node), out);
		break;
	}
}

int
rope_node_to_graphviz(struct RopeNode *root, const char *file) {
	FILE *out = fopen(file, "w");
	if (!out) {
		return -1;
	}
	while (root->parent) {
		root = root->parent;
	}
	fputs("digraph G {\n", out);
	print_node(root, out);
	fputs("}\n", out);
	fclose(out);
}
