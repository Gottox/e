#ifndef ROPE_NODE_H
#define ROPE_NODE_H

#include <cextras/memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "rope_common.h"
#include "rope_str.h"

#define ROPE_INLINE_LEAF_SIZE (sizeof(void *[2]))

struct RopePool;

/**********************************
 * node/node.c
 */

#define ROPE_NODE_TYPE_MASK (~(UINT64_MAX >> 1))
enum RopeNodeType {
	ROPE_NODE_LEAF,
	ROPE_NODE_BRANCH,
};

typedef enum RopeNodeType rope_node_type_t;

struct RopeBranch {
	struct RopeNode *children[2];
	struct RopeDim dim;
};

struct RopeNode {
	/*
	 * High bit: type (leaf/branch);
	 * Low 63 bits:
	 * - For branch nodes: depth in the tree
	 * - For leaf nodes: tags
	 */
	uint64_t bits;
	struct RopeNode *parent;

	union {
		struct RopeStr leaf;
		struct RopeBranch branch;
	} data;
};

typedef bool (*rope_node_condition_f)(
		const struct RopeNode *node, void *userdata);

ROPE_NO_UNUSED struct RopeNode *
rope_node_new(struct RopePool *pool) ROPE_NO_UNUSED;

void rope_node_cleanup(struct RopeNode *node);

void rope_node_free(struct RopeNode *node, struct RopePool *pool);

/**********************************
 * node/navigation.c
 */

enum RopeDirection rope_node_which(const struct RopeNode *node) ROPE_NO_UNUSED;

ROPE_NO_UNUSED struct RopeNode *
rope_node_leaf(struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

ROPE_NO_UNUSED struct RopeNode *rope_node_child(
		const struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

ROPE_NO_UNUSED struct RopeNode *
rope_node_parent(const struct RopeNode *node) ROPE_NO_UNUSED;

ROPE_NO_UNUSED struct RopeNode *rope_node_neighbour(
		const struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

/**********************************
 * node/tags.c
 */

ROPE_NO_UNUSED uint64_t rope_node_tags(struct RopeNode *node) ROPE_NO_UNUSED;

ROPE_NO_UNUSED bool
rope_node_match_tags(struct RopeNode *node, uint64_t tags) ROPE_NO_UNUSED;

void rope_node_add_tags(struct RopeNode *node, uint64_t tags);

void rope_node_remove_tags(struct RopeNode *node, uint64_t tags);

void rope_node_set_tags(struct RopeNode *node, uint64_t tags);

/**********************************
 * node/info.c
 */

ROPE_NO_UNUSED size_t
rope_node_size(const struct RopeNode *node, enum RopeUnit unit) ROPE_NO_UNUSED;

ROPE_NO_UNUSED enum RopeNodeType
rope_node_type(const struct RopeNode *node) ROPE_NO_UNUSED;

ROPE_NO_UNUSED size_t rope_node_depth(struct RopeNode *node);

ROPE_NO_UNUSED const uint8_t *
rope_node_value(const struct RopeNode *node, size_t *size);

/**********************************
 * node/mutation.c
 */

void rope_node_delete(struct RopeNode *node, struct RopePool *pool);

void rope_node_delete_child(
		struct RopeNode *node, struct RopePool *pool, enum RopeDirection which);

struct RopeNode *rope_node_delete_and_neighbour(
		struct RopeNode *node, struct RopePool *pool, enum RopeDirection which);

ROPE_NO_UNUSED int
rope_node_skip(struct RopeNode *node, enum RopeUnit unit, size_t offset);

ROPE_NO_UNUSED int
rope_node_merge(struct RopeNode *node, size_t count, struct RopePool *pool);

void rope_node_update_depth(struct RopeNode *node);

void rope_node_update_sizes(struct RopeNode *node);

void rope_node_propagate_sizes(struct RopeNode *node);

void rope_node_update_children(struct RopeNode *node);

void rope_node_move(struct RopeNode *target, struct RopeNode *node);

void rope_node_set_type(struct RopeNode *node, enum RopeNodeType type);

ROPE_NO_UNUSED int rope_node_split(
		struct RopeNode *node, struct RopePool *pool, size_t index,
		enum RopeUnit unit, struct RopeNode **left_ptr,
		struct RopeNode **right_ptr);

void rope_node_rotate(struct RopeNode *node, enum RopeDirection which);

ROPE_NO_UNUSED int
rope_node_chores(struct RopeNode *node, struct RopePool *pool);

void rope_node_balance_up(struct RopeNode *node);

ROPE_NO_UNUSED int
rope_node_compact(struct RopeNode *node, struct RopePool *pool);

/**********************************
 * node/insert.c
 */

int rope_node_insert(
		struct RopeNode *node, struct RopeStr *str, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which);

ROPE_NO_UNUSED int rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool);

ROPE_NO_UNUSED int rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool);

ROPE_NO_UNUSED int rope_node_insert_heap_left(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool);

ROPE_NO_UNUSED int rope_node_insert_heap_right(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool);

/**********************************
 * inline node functions
 */

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_left(const struct RopeNode *node) {
	return rope_node_child(node, ROPE_LEFT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_right(const struct RopeNode *node) {
	return rope_node_child(node, ROPE_RIGHT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_next(const struct RopeNode *node) {
	return rope_node_neighbour(node, ROPE_RIGHT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_prev(const struct RopeNode *node) {
	return rope_node_neighbour(node, ROPE_LEFT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_first(const struct RopeNode *node) {
	return rope_node_leaf((struct RopeNode *)node, ROPE_LEFT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_last(const struct RopeNode *node) {
	return rope_node_leaf((struct RopeNode *)node, ROPE_RIGHT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_delete_and_prev(struct RopeNode *node, struct RopePool *pool) {
	return rope_node_delete_and_neighbour(node, pool, ROPE_LEFT);
}

ROPE_NO_UNUSED static inline struct RopeNode *
rope_node_delete_and_next(struct RopeNode *node, struct RopePool *pool) {
	return rope_node_delete_and_neighbour(node, pool, ROPE_RIGHT);
}

#endif /* ROPE_NODE_H */
