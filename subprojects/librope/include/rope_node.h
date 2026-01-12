#ifndef ROPE_NODE_H
#define ROPE_NODE_H

#include <cextras/memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "rope_common.h"

#define ROPE_INLINE_LEAF_SIZE (sizeof(void *[2]))
#define ROPE_TYPE_SHIFT (sizeof(uint64_t) * 8 - 2)
#define ROPE_TYPE_MASK (~(uint64_t)0 << ROPE_TYPE_SHIFT)

struct RopePool;

/**********************************
 * node/node.c
 */

enum RopeNodeType {
	ROPE_NODE_INLINE_LEAF,
	ROPE_NODE_LEAF,
	ROPE_NODE_BRANCH,
};

enum RopeDirection {
	ROPE_LEFT,
	ROPE_RIGHT,
};

struct RopeNode {
	/**
	 * | types \ bits | 64,63 | 62-0         |
	 * |--------------|-------|--------------|
	 * | branch       | type  | child depth  |
	 * | leaf         | type  | user defined |
	 * | inline leaf  | type  | user defined |
	 */
	uint64_t tags;
	struct RopeNode *parent;

	union {
		struct {
			size_t byte_size;
			uint8_t data[ROPE_INLINE_LEAF_SIZE];
		} inline_leaf;
		struct {
			size_t byte_size;
			struct RopeRcString *owned;
			const uint8_t *data;
		} leaf;
		struct {
			struct RopeNode *children[2];
		} branch;
	} data;
};

typedef bool (*rope_node_condition_f)(const struct RopeNode *node, void *userdata);

struct RopeNode *rope_node_new(struct RopePool *pool) ROPE_NO_UNUSED;

bool rope_node_match_tags(struct RopeNode *node, uint64_t tags) ROPE_NO_UNUSED;

size_t rope_node_new_lines(const struct RopeNode *node) ROPE_NO_UNUSED;

size_t rope_node_char_size(const struct RopeNode *node) ROPE_NO_UNUSED;

size_t rope_node_byte_size(const struct RopeNode *node) ROPE_NO_UNUSED;

int rope_node_set_value(
		struct RopeNode *node, const uint8_t *data,
		size_t byte_size) ROPE_NO_UNUSED;

int rope_node_append_value(
		struct RopeNode *node, const uint8_t *data,
		size_t byte_size) ROPE_NO_UNUSED;

void rope_node_set_rc_string(
		struct RopeNode *node, struct RopeRcString *str, size_t byte_index,
		size_t size);

void rope_node_delete(struct RopeNode *node, struct RopePool *pool);

void rope_node_delete_child(
		struct RopeNode *node, struct RopePool *pool, enum RopeDirection which);

struct RopeNode *
rope_node_delete_while(
		struct RopeNode *node, struct RopePool *pool,
		rope_node_condition_f condition, void *userdata);

struct RopeNode *rope_node_find(
		struct RopeNode *node, rope_index_t line, rope_index_t column,
		uint64_t tags, rope_byte_index_t *byte_index) ROPE_NO_UNUSED;

struct RopeNode *rope_node_find_char(
		struct RopeNode *node, rope_char_index_t char_index, uint64_t tags,
		rope_byte_index_t *byte_index) ROPE_NO_UNUSED;

enum RopeNodeType rope_node_type(const struct RopeNode *node) ROPE_NO_UNUSED;

enum RopeDirection rope_node_which(const struct RopeNode *node) ROPE_NO_UNUSED;

struct RopeNode *rope_node_sibling(const struct RopeNode *node) ROPE_NO_UNUSED;

struct RopeNode *
rope_node_leaf(struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

struct RopeNode *rope_node_child(
		const struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

struct RopeNode *rope_node_parent(const struct RopeNode *node) ROPE_NO_UNUSED;

struct RopeNode *rope_node_neighbour(
		const struct RopeNode *node, enum RopeDirection which) ROPE_NO_UNUSED;

void rope_node_cleanup(struct RopeNode *node);

void rope_node_free(struct RopeNode *node, struct RopePool *pool);

/**********************************
 * node/tags.c
 */

uint64_t rope_node_tags(struct RopeNode *node) ROPE_NO_UNUSED;

void rope_node_add_tags(struct RopeNode *node, uint64_t tags);

void rope_node_remove_tags(struct RopeNode *node, uint64_t tags);

void rope_node_set_tags(struct RopeNode *node, uint64_t tags);

/**********************************
 * node/info.c
 */

size_t rope_node_depth(struct RopeNode *node);

const uint8_t *rope_node_value(const struct RopeNode *node, size_t *size);

/**********************************
 * node/mutation.c
 */

void rope_node_set_type(struct RopeNode *node, enum RopeNodeType type);

int rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr);

int rope_node_insert(
		struct RopeNode *target, struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which);

void rope_node_rotate(struct RopeNode *node, enum RopeDirection which);

void rope_node_balance_up(struct RopeNode *node);

/**********************************
 * inline node functions
 */

static inline struct RopeNode *
rope_node_left(const struct RopeNode *node) {
	return rope_node_child(node, ROPE_LEFT);
}

static inline struct RopeNode *
rope_node_right(const struct RopeNode *node) {
	return rope_node_child(node, ROPE_RIGHT);
}

static inline struct RopeNode *
rope_node_next(const struct RopeNode *node) {
	return rope_node_neighbour(node, ROPE_RIGHT);
}

static inline struct RopeNode *
rope_node_prev(const struct RopeNode *node) {
	return rope_node_neighbour(node, ROPE_LEFT);
}

static inline struct RopeNode *
rope_node_first(const struct RopeNode *node) {
	return rope_node_leaf((struct RopeNode *)node, ROPE_LEFT);
}

static inline struct RopeNode *
rope_node_last(const struct RopeNode *node) {
	return rope_node_leaf((struct RopeNode *)node, ROPE_RIGHT);
}

#endif /* ROPE_NODE_H */
