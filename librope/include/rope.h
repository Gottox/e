#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ROPE_INLINE_LEAF_SIZE (sizeof(void *[3]))
#define ROPE_POOL_CHUNK_SIZE 1024

struct Rope;

typedef size_t rope_char_index_t;
typedef size_t rope_byte_index_t;
typedef size_t rope_index_t;

/**********************************
 * pool.c
 */

struct RopePool {
	struct RopeNode **nodes;
	size_t next_node_index;
	struct RopeNode *reuse_pool;
};

int rope_pool_init(struct RopePool *pool);

struct RopeNode *rope_pool_get(struct RopePool *pool);

int rope_pool_recycle(struct RopePool *pool, struct RopeNode *node);

int rope_pool_cleanup(struct RopePool *pool);

/**********************************
 * node.c
 */

enum RopeNodeType {
	ROPE_NODE_INLINE_LEAF,
	ROPE_NODE_LEAF,
	ROPE_NODE_FORK,
};

struct RopeNode {
	enum RopeNodeType type;

	union {
		uint8_t inline_leaf[ROPE_INLINE_LEAF_SIZE];
		struct {
			uint8_t *owned_data;
			uint8_t *data;
		} leaf;
		struct {
			uint8_t *owned_data;
			struct RopeNode *left;
			struct RopeNode *right;
		} fork;
		struct {
			struct RopeNode *next;
		} reuse;
	} data;

	struct RopeNode *parent;
	size_t char_size;
	size_t byte_size;
};

int rope_node_init(struct RopeNode *node);

struct RopeNode *
rope_node_find(struct RopeNode *node, rope_char_index_t *index);

int rope_node_insert(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index,
		const uint8_t *data, size_t byte_size);

int rope_node_delete(struct RopeNode *node, struct Rope *rope);

bool rope_node_next(struct RopeNode **node);

int rope_node_split(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index);

const uint8_t *rope_node_value(struct RopeNode *node, size_t *size);

int rope_node_cleanup(struct RopeNode *node);

/**********************************
 * rope.c
 */

enum RopeBias {
	ROPE_BIAS_LEFT,
	ROPE_BIAS_RIGHT,
};

struct Rope {
	struct RopeNode *root;

	struct RopePool pool;

	enum RopeBias bias;
};

int rope_init(struct Rope *rope);

int rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size);

struct RopeNode *rope_new_node(struct Rope *rope);

int rope_insert(
		struct Rope *rope, size_t index, const uint8_t *data, size_t byte_size);

int rope_delete(struct Rope *rope, size_t index, size_t char_count);

struct RopeNode *rope_first(struct Rope *rope);

struct RopeNode *rope_node_left(struct RopeNode *base);

struct RopeNode *rope_node_right(struct RopeNode *base);

const uint8_t *rope_node_value(struct RopeNode *node, size_t *size);

int rope_cleanup(struct Rope *rope);

void rope_print_tree(struct Rope *rope, FILE *out);
