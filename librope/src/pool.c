#include <rope.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////
/// struct RopePool

int
rope_pool_init(struct RopePool *pool) {
	memset(pool, 0, sizeof(*pool));

	return 0;
}

struct RopeNode *
reuse_node(struct RopePool *pool) {
	struct RopeNode *node = pool->reuse_pool;
	pool->reuse_pool = node->data.reuse.next;
	return node;
}

static int
add_chunk(struct RopePool *pool) {
	size_t outer_size = pool->next_node_index / ROPE_POOL_CHUNK_SIZE + 1;
	pool->nodes =
			reallocarray(pool->nodes, outer_size, sizeof(struct RopeNode *));
	if (pool->nodes == NULL) {
		return -1;
	}
	struct RopeNode *new_chunk =
			calloc(ROPE_POOL_CHUNK_SIZE, sizeof(struct RopeNode));
	if (new_chunk == NULL) {
		return -1;
	}
	pool->nodes[outer_size - 1] = new_chunk;
	return 0;
}

struct RopeNode *
rope_pool_get(struct RopePool *pool) {
	struct RopeNode *node = NULL;
	int rv = 0;
	if (pool->reuse_pool != NULL) {
		return reuse_node(pool);
	} else if (pool->next_node_index % ROPE_POOL_CHUNK_SIZE == 0) {
		rv = add_chunk(pool);
		if (rv < 0) {
			return NULL;
		}
	}

	size_t next_index = pool->next_node_index;
	node = &pool->nodes[next_index / ROPE_POOL_CHUNK_SIZE]
					   [next_index % ROPE_POOL_CHUNK_SIZE];
	rope_node_init(node);

	pool->next_node_index++;

	return node;
}

int
rope_pool_recycle(struct RopePool *pool, struct RopeNode *node) {
	if (node != NULL) {
		rope_node_cleanup(node);
		node->data.reuse.next = pool->reuse_pool;
		pool->reuse_pool = node;
	}
	return 0;
}

int
rope_pool_cleanup(struct RopePool *pool) {
	int rv = 0;

	size_t outer_size = 0;
	for (rope_index_t i = 0; i < pool->next_node_index; i++) {
		struct RopeNode *node = &pool->nodes[i / ROPE_POOL_CHUNK_SIZE]
											[i % ROPE_POOL_CHUNK_SIZE];
		if (i % ROPE_POOL_CHUNK_SIZE == 0) {
			outer_size++;
		}

		rv |= rope_node_cleanup(node);
	}

	for (rope_index_t i = 0; i < outer_size; i++) {
		free(pool->nodes[i]);
	}

	free(pool->nodes);

	return rv;
}
