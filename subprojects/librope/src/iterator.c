#include "rope_str.h"
#include <cextras/unicode.h>
#include <rope.h>
#include <stdbool.h>

int
rope_iterator_init(
		struct RopeIterator *iter, struct RopeRange *range, uint64_t tags) {
	iter->range = range;
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;

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
rope_iterator_next(struct RopeIterator *iter, struct RopeStr *str) {
	rope_str_cleanup(str);

	if (iter->node == NULL) {
		return false;
	}

	size_t node_size = rope_str_size(&iter->node->data.leaf, ROPE_BYTE);
	size_t start = iter->started ? 0 : iter->start_byte;
	size_t end = node_size;

	if (iter->node == iter->end) {
		end = iter->end_byte;
	}

	if (start > end) {
		start = end;
	}

	int rv = rope_str_clone_trim(
			str, &iter->node->data.leaf, ROPE_BYTE, start, end - start);
	if (rv != 0) {
		return false;
	}

	if (iter->node == iter->end) {
		iter->node = NULL;
	} else {
		do {
			iter->node = rope_node_next(iter->node);
			if (!iter->node) {
				break;
			}
		} while (rope_node_match_tags(iter->node, iter->tags) == false);
		iter->started = true;
	}

	return true;
}

void
rope_iterator_cleanup(struct RopeIterator *iter) {
	(void)iter;
}
