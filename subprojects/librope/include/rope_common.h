#ifndef ROPE_COMMON_H
#define ROPE_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROPE_POOL_CHUNK_SIZE 1024

#define ROPE_OVERFLOW_ADD(a, b, res) __builtin_add_overflow(a, b, res)

#define ROPE_OVERFLOW_SUB(a, b, res) __builtin_sub_overflow(a, b, res)

#define ROPE_UNREACHABLE() __builtin_unreachable()

#define ROPE_NO_EXPORT __attribute__((visibility("hidden")))

#define ROPE_NO_UNUSED __attribute__((warn_unused_result))

#define ROPE_NODE_IS_LEAF(node) (rope_node_type(node) == ROPE_NODE_LEAF)

#define ROPE_NODE_IS_BRANCH(node) (rope_node_type(node) == ROPE_NODE_BRANCH)

#define ROPE_NODE_IS_ROOT(node) (rope_node_parent(node) == NULL)

#define ROPE_NODE_COMPACT_THRESHOLD 10

#define ROPE_CHORE_RUN_INTERVAL 8192

typedef size_t rope_char_index_t;
typedef size_t rope_byte_index_t;
typedef size_t rope_index_t;

enum RopeDirection {
	ROPE_LEFT,
	ROPE_RIGHT,
};

enum RopeUnit {
	ROPE_BYTE,
	ROPE_CHAR,
	ROPE_CP,
	ROPE_LINE,
	ROPE_UTF16,
	ROPE_UNIT_COUNT,
};

struct RopeDim {
	size_t dim[ROPE_UNIT_COUNT];
};

#endif
