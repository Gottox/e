#include <assert.h>
#include <highlight_private.h>
#include <stdint.h>
#include <string.h>
#include <tree_sitter/api.h>

const TSQueryCapture *
get_highlight_capture(const TSQueryMatch *match) {
	return &match->captures[0];
}

int
highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlight *highlight,
		const TSTree *tree) {
	memset(iterator, 0, sizeof(struct HighlightIterator));

	iterator->highlight = highlight;
	iterator->tree = tree;
	iterator->cursor = ts_query_cursor_new();
	iterator->current_capture_id = HIGHLIGHT_NO_CAPTURE;

	TSNode root = ts_tree_root_node(iterator->tree);
	iterator->current_offset = ts_node_start_byte(root);
	iterator->end_offset = ts_node_end_byte(root);
	ts_query_cursor_exec(iterator->cursor, highlight->query, root);

	cx_prealloc_pool_init(
			&iterator->marker_pool, sizeof(struct HighlightMarker));

	iterator->markers = NULL;

	return 0;
}

struct HighlightMarker *
marker_insert(
		struct HighlightIterator *iterator, struct HighlightMarker **top,
		struct HighlightMarker *new_marker, uint32_t *old_capture) {
	struct HighlightMarker *current = *top;
	if (current == NULL) {
		*top = new_marker;
		new_marker->next = NULL;
		return new_marker;
	}

	while (current->next != NULL) {
		if (new_marker->offset < current->next->offset) {
			break;
		}
		current = current->next;
	}

	*old_capture = current->capture_id;

	if (new_marker->offset == current->offset) {
		current->capture_id = new_marker->capture_id;
		cx_prealloc_pool_recycle(&iterator->marker_pool, new_marker);
		return current;
	} else {
		new_marker->next = current->next;
		current->next = new_marker;
		return new_marker;
	}
}

static int
add_match(struct HighlightIterator *iterator) {
	int rv = 0;
	TSQueryMatch match = {0};
	struct Highlight *highlight = iterator->highlight;

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

	uint32_t capture_id = highlight->capture_lookup_table[query_capture->index];
	if (HIGHLIGHT_IS_SPECIAL(capture_id)) {
		goto out;
	}

	struct HighlightMarker *start =
			cx_prealloc_pool_get(&iterator->marker_pool);
	if (start == NULL) {
		rv = -1;
		goto out;
	}
	start->offset = start_byte;
	start->capture_id = HIGHLIGHT_CAPTURE_ID(capture_id);
	start = marker_insert(iterator, &iterator->markers, start, &old_capture);

	struct HighlightMarker *end = cx_prealloc_pool_get(&iterator->marker_pool);
	if (end == NULL) {
		rv = -1;
		goto out;
	}
	end->offset = end_byte;
	end->capture_id = old_capture;
	end->next = start->next;
	start->next = end;

out:
	return rv;
}

static int
markers_fill(struct HighlightIterator *iterator) {
	int rv = 0;
	const struct HighlightMarker dummy = {0};
	const struct HighlightMarker *start =
			iterator->markers ? iterator->markers : &dummy;
	const struct HighlightMarker *end = start->next ? start->next : &dummy;

	while (iterator->tree_completed_offset != iterator->end_offset &&
		   end->offset >= iterator->tree_completed_offset) {
		rv = add_match(iterator);
		if (rv < 0) {
			goto out;
		}
		if (iterator->markers != NULL) {
			start = iterator->markers;
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
		iterator->markers == NULL) {
		return false;
	}

	rv = markers_fill(iterator);
	if (rv < 0) {
		goto out;
	}

	struct HighlightMarker *next = iterator->markers;
	const uint32_t next_marker = next ? next->offset : iterator->end_offset;
	if (iterator->current_offset != next_marker) {
		event->type = HIGHLIGHT_TEXT;
		event->text.start = iterator->current_offset;
		event->text.end = iterator->current_offset = next_marker;
	} else if (iterator->current_capture_id == HIGHLIGHT_NO_CAPTURE) {
		uint32_t capture_id = next->capture_id;
		event->type = HIGHLIGHT_START;
		event->start.capture_id = capture_id;
		iterator->current_capture_id = capture_id;
		iterator->markers = next->next;
		cx_prealloc_pool_recycle(&iterator->marker_pool, next);
	} else {
		event->type = HIGHLIGHT_END;
		iterator->current_capture_id = HIGHLIGHT_NO_CAPTURE;
		if (next->capture_id == HIGHLIGHT_NO_CAPTURE) {
			iterator->markers = next->next;
			cx_prealloc_pool_recycle(&iterator->marker_pool, next);
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
	cx_prealloc_pool_cleanup(&iterator->marker_pool);
	return 0;
}
