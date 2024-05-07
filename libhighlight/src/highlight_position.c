#include <highlight.h>

#define HIGHLIGHT_POSITION_POOL_SIZE 32
#define HIGHLIGHT_DIVIDE_CEIL(x, y) ((x) / (y) + !!((x) % (y)))

struct HighlightPosition *
highlight_position_pool_new(struct HighlightPositionPool *pool) {
	if (pool->recycle != NULL) {
		struct HighlightPosition *top = pool->recycle;
		pool->recycle = top->next;
		return top;
	}

	if (pool->pool_size % HIGHLIGHT_POSITION_POOL_SIZE == 0) {
		size_t new_size = (pool->pool_size / HIGHLIGHT_POSITION_POOL_SIZE) + 1;

		struct HighlightPosition **new_pool_table = reallocarray(
				pool->pools, new_size, sizeof(struct HighlightPosition *));
		if (new_pool_table == NULL) {
			return NULL;
		}
		pool->pools = new_pool_table;
		struct HighlightPosition *new_pool = calloc(
				HIGHLIGHT_POSITION_POOL_SIZE, sizeof(struct HighlightPosition));
		if (new_pool == NULL) {
			return NULL;
		}
		pool->pools[new_size - 1] = new_pool;
	}
	size_t outer_index = pool->pool_size / HIGHLIGHT_POSITION_POOL_SIZE;
	size_t inner_index = pool->pool_size % HIGHLIGHT_POSITION_POOL_SIZE;

	struct HighlightPosition *result = &pool->pools[outer_index][inner_index];
	pool->pool_size++;
	return result;
}

int
highlight_position_pool_recycle(
		struct HighlightPositionPool *iterator,
		struct HighlightPosition *position) {
	position->next = iterator->recycle;
	iterator->recycle = position;
	return 0;
}

int
highlight_position_pool_cleanup(struct HighlightPositionPool *iterator) {
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
