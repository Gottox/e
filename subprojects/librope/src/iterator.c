#include <cextras/unicode.h>
#include <rope.h>
#include <stdbool.h>

int
rope_iterator_init(
		struct RopeIterator *iter, struct RopeRange *range, uint64_t tags) {
	iter->range = range;
	struct RopeCursor *start = rope_range_start(range);
	struct RopeCursor *end = rope_range_end(range);

	iter->node = rope_cursor_node(start, &iter->start_byte);
	iter->end = rope_cursor_node(end, &iter->end_byte);
	iter->started = false;
	iter->tags = tags;

	if (iter->node == iter->end && iter->start_byte >= iter->end_byte) {
		iter->node = NULL;
	}

	return 0;
}

bool
rope_iterator_next(
		struct RopeIterator *iter, const uint8_t **value, size_t *size) {
	if (iter->node == NULL) {
		return false;
	}

	size_t node_size = 0;
	const uint8_t *data = rope_node_value(iter->node, &node_size);
	rope_byte_index_t start = iter->started ? 0 : iter->start_byte;
	rope_byte_index_t end = node_size;

	if (iter->node == iter->end) {
		end = iter->end_byte;
	}

	if (start > end) {
		start = end;
	}

	*value = data + start;
	*size = (end > start) ? end - start : 0;

	if (iter->node == iter->end) {
		iter->node = NULL;
	} else {
		do {
			if (!rope_node_next(&iter->node)) {
				break;
			}
		} while (rope_node_match_tags(iter->node, iter->tags) == false);
		iter->started = true;
	}

	return true;
}

int
rope_iterator_cleanup(struct RopeIterator *iter) {
	(void)iter;
	return 0;
}
