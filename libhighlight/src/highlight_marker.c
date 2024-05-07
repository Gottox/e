#include <highlight_private.h>

struct HighlightMarker *
highlight_marker_pool_new(struct HighlightMarkerPool *pool) {
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
highlight_marker_pool_recycle(
		struct HighlightMarkerPool *iterator, struct HighlightMarker *marker) {
	marker->next = iterator->recycle;
	iterator->recycle = marker;
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
