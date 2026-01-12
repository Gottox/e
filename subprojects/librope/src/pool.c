#include <rope.h>
#include <stdbool.h>

//////////////////////////////
/// struct RopePool

int
rope_pool_init(struct RopePool *pool) {
	cx_prealloc_pool_init(&pool->pool, sizeof(struct RopeNode));

	return 0;
}

struct RopeNode *
rope_pool_get(struct RopePool *pool) {
	return cx_prealloc_pool_get(&pool->pool);
}

void
rope_pool_recycle(struct RopePool *pool, struct RopeNode *node) {
	cx_prealloc_pool_recycle(&pool->pool, node);
}

void
rope_pool_cleanup(struct RopePool *pool) {
	cx_prealloc_pool_cleanup(&pool->pool);
}
