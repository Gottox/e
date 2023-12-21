#include <assert.h>
#include <rope.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int
rope_init(struct Rope *rope) {
	int rv = 0;
	rv = rope_pool_init(&rope->pool);
	if (rv < 0) {
		goto out;
	}

	rope->root = rope_pool_get(&rope->pool);
	if (rope->root == NULL) {
		rv = -1;
		goto out;
	}
	rope->bias = ROPE_BIAS_LEFT;

out:
	return rv;
}

int
rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size) {
	size_t char_count = rope->root->char_size;
	return rope_insert(rope, char_count, data, byte_size);
}

int
rope_delete(struct Rope *rope, rope_char_index_t index, size_t char_count) {
	struct RopeNode *node = rope_node_find(rope->root, &index);
	struct RopeNode *next = NULL;

	if (index != 0) {
		rope_node_split(node, rope, index);
		node = rope_node_right(node);
	}
	while (node && node->char_size <= char_count) {
		next = node;
		rope_node_next(&next);
		char_count -= node->char_size;
		rope_node_delete(node, rope);
		node = next;
	}
	if (node && node->char_size > char_count) {
		rope_node_split(node, rope, char_count);
		node = rope_node_left(node);
		rope_node_delete(node, rope);
	}
	return 0;
}

int
rope_insert(struct Rope *rope, size_t index, const uint8_t *data,
			size_t byte_size) {
	return rope_node_insert(rope->root, rope, index, data, byte_size);
}

int
rope_cleanup(struct Rope *rope) {
	return rope_pool_cleanup(&rope->pool);
}
