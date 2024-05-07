#include <assert.h>
#include <highlight.h>
#include <stdint.h>
#include <string.h>
#include <tree_sitter/api.h>

#define HIGHLIGHT_NO_CAPTURE UINT32_MAX

const TSQueryCapture *
get_highlight_capture(const TSQueryMatch *match) {
	return &match->captures[0];
}

int
highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlighter *highlighter,
		const TSTree *tree) {
	memset(iterator, 0, sizeof(struct HighlightIterator));

	iterator->highlighter = highlighter;
	iterator->tree = tree;
	iterator->cursor = ts_query_cursor_new();
	iterator->current_capture_id = HIGHLIGHT_NO_CAPTURE;

	TSNode root = ts_tree_root_node(iterator->tree);
	iterator->current_offset = ts_node_start_byte(root);
	iterator->end_offset = ts_node_end_byte(root);
	ts_query_cursor_exec(iterator->cursor, highlighter->query, root);

	return 0;
}

struct HighlightPosition *
position_insert(
		struct HighlightIterator *iterator, struct HighlightPosition **top,
		struct HighlightPosition *new_position, uint32_t *old_capture) {
	struct HighlightPosition *current = *top;
	if (current == NULL) {
		*top = new_position;
		new_position->next = NULL;
		return new_position;
	}

	while (current->next != NULL) {
		if (new_position->position < current->next->position) {
			break;
		}
		current = current->next;
	}

	if (old_capture) {
		*old_capture = current->capture_id;
	}

	if (new_position->position == current->position) {
		current->capture_id = new_position->capture_id;
		highlight_position_pool_recycle(&iterator->position_pool, new_position);
		return current;
	} else {
		new_position->next = current->next;
		current->next = new_position;
		return new_position;
	}
}

static int
add_match(struct HighlightIterator *iterator) {
	int rv = 0;
	TSQueryMatch match = {0};
	struct Highlighter *highlighter = iterator->highlighter;

	bool has_next = ts_query_cursor_next_match(iterator->cursor, &match);
	if (!has_next) {
		iterator->tree_completed_offset = iterator->end_offset;
		goto out;
	}
	const TSQueryCapture *query_capture = get_highlight_capture(&match);
	if (query_capture == NULL) {
		goto out;
	}
	const uint_fast32_t start_byte = ts_node_start_byte(query_capture->node);
	const uint_fast32_t end_byte = ts_node_end_byte(query_capture->node);
	assert(start_byte < end_byte);

	iterator->tree_completed_offset = start_byte;

	uint32_t old_capture = HIGHLIGHT_NO_CAPTURE;

	uint32_t capture_id =
			highlighter->capture_lookup_table[query_capture->index];
	struct HighlightPosition *start =
			highlight_position_pool_new(&iterator->position_pool);
	start->position = start_byte;
	start->capture_id = capture_id;
	start = position_insert(
			iterator, &iterator->positions, start, &old_capture);
	if (start == NULL) {
		rv = -1;
		goto out;
	}

	struct HighlightPosition *end =
			highlight_position_pool_new(&iterator->position_pool);
	end->position = end_byte;
	end->capture_id = old_capture;
	end = position_insert(iterator, &start, end, NULL);
	if (end == NULL) {
		rv = -1;
		goto out;
	}

out:
	return rv;
}

static int
positions_fill(struct HighlightIterator *iterator) {
	int rv = 0;
	const struct HighlightPosition dummy = {0};
	const struct HighlightPosition *start =
			iterator->positions ? iterator->positions : &dummy;
	const struct HighlightPosition *end = start->next ? start->next : &dummy;

	while (iterator->tree_completed_offset != iterator->end_offset &&
		   end->position >= iterator->tree_completed_offset) {
		rv = add_match(iterator);
		if (rv < 0) {
			goto out;
		}
		if (iterator->positions != NULL) {
			start = iterator->positions;
			end = start->next;
		}
	}
out:
	return rv;
}

bool
highlight_iterator_next(
		struct HighlightIterator *iterator, struct HighlightEvent *event,
		int *err) {
	int rv = 0;

	memset(event, 0, sizeof(struct HighlightEvent));

	if (iterator->current_offset == iterator->end_offset &&
		iterator->positions == NULL) {
		return false;
	}

	rv = positions_fill(iterator);
	if (rv < 0) {
		goto out;
	}

	struct HighlightPosition *next = iterator->positions;
	const uint32_t next_position = next ? next->position : iterator->end_offset;
	if (iterator->current_offset != next_position) {
		event->type = HIGHLIGHT_TEXT;
		event->text.start = iterator->current_offset;
		event->text.end = iterator->current_offset = next_position;
	} else if (iterator->current_capture_id == HIGHLIGHT_NO_CAPTURE) {
		uint32_t capture_id = next->capture_id;
		event->type = HIGHLIGHT_START;
		event->start.capture_id = capture_id;
		iterator->current_capture_id = capture_id;
		iterator->positions = next->next;
		highlight_position_pool_recycle(&iterator->position_pool, next);
	} else {
		event->type = HIGHLIGHT_END;
		iterator->current_capture_id = HIGHLIGHT_NO_CAPTURE;
		if (next->capture_id == HIGHLIGHT_NO_CAPTURE) {
			iterator->positions = next->next;
			highlight_position_pool_recycle(&iterator->position_pool, next);
		}
	}

out:
	if (rv < 0) {
		*err = rv;
	}
	return true;
}

int
highlight_iterator_cleanup(struct HighlightIterator *iterator) {
	ts_query_cursor_delete(iterator->cursor);
	return highlight_position_pool_cleanup(&iterator->position_pool);
}
