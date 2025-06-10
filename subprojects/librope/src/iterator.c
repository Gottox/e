#include <rope.h>
#include <utf8util.h>

int
rope_iterator_init(struct RopeIterator *iter, struct RopeRange *range) {
	iter->range = range;
	struct RopeCursor *left_cursor = rope_range_left(range);
	struct RopeCursor *right_cursor = rope_range_right(range);

	iter->left = rope_cursor_node(left_cursor, &iter->current_index);
	iter->right = rope_cursor_node(right_cursor, &iter->end_index);
	return 0;
}

bool
rope_iterator_next(
		struct RopeIterator *iter, const uint8_t **value, size_t *size) {
	rope_char_index_t index = iter->current_index;
	const struct RopeNode *node = iter->current;

	if (node == iter->right) {
		return false;
	} else if (node == NULL) {
		node = iter->current = iter->left;
	}
	const uint8_t *data = rope_node_value(node, size);
	rope_byte_index_t left_index = cx_utf8_bidx(data, *size, index);
	rope_byte_index_t right_index = node->byte_size;
	if (iter->right == node) {
		right_index = cx_utf8_bidx(data, *size, iter->end_index);
	}

	iter->current_index = 0;
	rope_node_next(&iter->left);
	*size = left_index + right_index;

	*value = &data[left_index];
	return true;
}

int
rope_iterator_cleanup(struct RopeIterator *iter) {
	(void)iter;
	return 0;
}
