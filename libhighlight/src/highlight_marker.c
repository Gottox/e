#include <highlight_private.h>

struct HighlightMarker *
highlight_marker_new(struct HighlightMarkerPool *pool) {
	if (pool->recycle != NULL) {
		struct HighlightMarker *top = pool->recycle;
		pool->recycle = top->next;
		return top;
	}

	if (pool->pool_size % HIGHLIGHT_POSITION_POOL_SIZE == 0) {
		size_t new_size = (pool->pool_size / HIGHLIGHT_POSITION_POOL_SIZE) + 1;

		struct HighlightMarker **new_pool_table = reallocarray(
				pool->pools, new_size, sizeof(struct HighlightMarker *));
		if (new_pool_table == NULL) {
			return NULL;
		}
		pool->pools = new_pool_table;
		struct HighlightMarker *new_pool = calloc(
				HIGHLIGHT_POSITION_POOL_SIZE, sizeof(struct HighlightMarker));
		if (new_pool == NULL) {
			return NULL;
		}
		pool->pools[new_size - 1] = new_pool;
	}
	size_t outer_index = pool->pool_size / HIGHLIGHT_POSITION_POOL_SIZE;
	size_t inner_index = pool->pool_size % HIGHLIGHT_POSITION_POOL_SIZE;

	struct HighlightMarker *result = &pool->pools[outer_index][inner_index];
	pool->pool_size++;
	return result;
}

int
highlight_marker_list_init(
		struct HighlightMarkerList *list, struct HighlightMarkerPool *pool) {
	list->head = NULL;
	list->pool = pool;
	return 0;
}

int
highlight_marker_list_cleanup(struct HighlightMarkerList *list) {
	struct HighlightMarker *iterator = list->head;
	while (iterator != NULL) {
		struct HighlightMarker *next = iterator->next;
		highlight_marker_recycle(iterator, list);
		iterator = next;
	}
	return 0;
}

int
highlight_marker_recycle(
		struct HighlightMarker *marker, struct HighlightMarkerList *list) {
	marker->next = list->pool->recycle;
	list->pool->recycle = marker;
	return 0;
}

int
highlight_marker_pool_init(struct HighlightMarkerPool *iterator) {
	iterator->pool_size = 0;
	iterator->recycle = NULL;
	iterator->pools = NULL;
	return 0;
}

int
highlight_marker_pool_cleanup(struct HighlightMarkerPool *iterator) {
	if (iterator->pools == NULL) {
		return 0;
	}
	size_t outer_size = HIGHLIGHT_DIVIDE_CEIL(
			iterator->pool_size, HIGHLIGHT_POSITION_POOL_SIZE);
	for (size_t i = 0; i < outer_size; i++) {
		free(iterator->pools[i]);
	}
	free(iterator->pools);
	return 0;
}
