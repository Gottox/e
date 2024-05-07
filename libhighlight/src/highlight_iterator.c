#include <highlight.h>
#include <string.h>
#include <tree_sitter/api.h>

#define HIGHLIGHT_POSITION_POOL_SIZE 32
#define HIGHLIGHT_DIVIDE_CEIL(x, y) ((x) / (y) + !!((x) % (y)))

struct HighlightPosition {
	uint32_t position;
	uint32_t capture_id;
	struct HighlightPosition *next;
};

const TSQueryCapture *
get_highlight_capture(const TSQueryMatch *match) {
	return &match->captures[0];
}

struct HighlightPosition *
position_pool_new(struct HighlightIterator *iterator) {
	if (iterator->positions_recycle != NULL) {
		struct HighlightPosition *top = iterator->positions_recycle;
		iterator->positions_recycle = top->next;
		return top;
	}

	if (iterator->position_pool_index % HIGHLIGHT_POSITION_POOL_SIZE == 0) {
		size_t new_size =
				(iterator->position_pool_index / HIGHLIGHT_POSITION_POOL_SIZE) +
				1;

		struct HighlightPosition **new_pool_table = reallocarray(
				iterator->position_pools, new_size,
				sizeof(struct HighlightPosition *));
		if (new_pool_table == NULL) {
			return NULL;
		}
		iterator->position_pools = new_pool_table;
		struct HighlightPosition *new_pool = calloc(
				HIGHLIGHT_POSITION_POOL_SIZE, sizeof(struct HighlightPosition));
		if (new_pool == NULL) {
			return NULL;
		}
		iterator->position_pools[new_size - 1] = new_pool;
	}
	size_t outer_index =
			iterator->position_pool_index / HIGHLIGHT_POSITION_POOL_SIZE;
	size_t inner_index =
			iterator->position_pool_index % HIGHLIGHT_POSITION_POOL_SIZE;

	struct HighlightPosition *result =
			&iterator->position_pools[outer_index][inner_index];
	iterator->position_pool_index++;
	return result;
}

int
position_pool_cleanup(struct HighlightIterator *iterator) {
	if (iterator->position_pools == NULL) {
		return 0;
	}
	size_t outer_size = HIGHLIGHT_DIVIDE_CEIL(
			iterator->position_pool_index, HIGHLIGHT_POSITION_POOL_SIZE);
	for (size_t i = 0; i < outer_size; i++) {
		free(iterator->position_pools[i]);
	}
	free(iterator->position_pools);
	return 0;
}

int
highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlighter *highlighter,
		const TSTree *tree) {
	memset(iterator, 0, sizeof(struct HighlightIterator));

	iterator->highlighter = highlighter;
	iterator->tree = tree;
	iterator->cursor = ts_query_cursor_new();

	TSNode root = ts_tree_root_node(iterator->tree);
	ts_query_cursor_exec(iterator->cursor, highlighter->query, root);

	return 0;
}

bool
highlight_iterator_next(
		struct HighlightIterator *iterator, struct HighlightEvent *event,
		int *err) {
	int rv = 0;
	TSQueryMatch match = {0};
	bool has_next = ts_query_cursor_next_match(iterator->cursor, &match);
	if (has_next == false) {
		goto out;
	}
	const TSQueryCapture *capture = get_highlight_capture(&match);
	size_t start_byte = ts_node_start_byte(capture->node);
	size_t end_byte = ts_node_end_byte(capture->node);

	iterator->last_start_byte = start_byte;

out:
	if (rv < 0) {
		*err = rv;
		has_next = false;
	}
	return has_next;
}

int
highlight_iterator_cleanup(struct HighlightIterator *iterator) {
	ts_query_cursor_delete(iterator->cursor);
	return position_pool_cleanup(iterator);
}
