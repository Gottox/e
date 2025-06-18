#include <cextras/memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef ROPE_H
#	define ROPE_H

#	define ROPE_INLINE_LEAF_SIZE (sizeof(void *[2]))
#	define ROPE_POOL_CHUNK_SIZE 1024
#	define ROPE_OVERFLOW_ADD(a, b, res) __builtin_add_overflow(a, b, res)
#	define ROPE_OVERFLOW_SUB(a, b, res) __builtin_sub_overflow(a, b, res)

struct Rope;

typedef size_t rope_char_index_t;
typedef size_t rope_byte_index_t;
typedef size_t rope_index_t;

/**********************************
 * rc_string.c
 */

struct RopeRcString {
	size_t ref_count;
	size_t size;
	uint8_t data[];
};

struct RopeRcString *rope_rc_string_new(const uint8_t *data, size_t size);

struct RopeRcString *rope_rc_string_retain(struct RopeRcString *str);

const uint8_t *rope_rc_string(struct RopeRcString *rc_str, size_t *size);

void rope_rc_string_release(struct RopeRcString *str);

/**********************************
 * pool.c
 */

struct RopePool {
	struct CxPreallocPool pool;
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
	ROPE_NODE_BRANCH,
};

enum RopeNodeDirection {
	ROPE_NODE_LEFT,
	ROPE_NODE_RIGHT,
};

struct RopeNode {
	enum RopeNodeType type;

	union {
		struct {
			uint64_t tags;
			uint8_t data[ROPE_INLINE_LEAF_SIZE];
		} inline_leaf;
		struct {
			uint64_t tags;
			struct RopeRcString *owned;
			const uint8_t *data;
		} leaf;
		struct {
			struct RopeNode *children[2];
			size_t leafs;
		} branch;
	} data;

	struct RopeNode *parent;
	size_t byte_size;
	size_t char_size;
	size_t utf16_size;
	size_t new_lines;
};

struct RopeNode *rope_node_new(struct RopePool *pool);

int rope_node_set_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size);

int rope_node_set_rc_string(
		struct RopeNode *node, struct RopeRcString *str, size_t byte_size);

int rope_node_insert_left(
		struct RopeNode *parent, struct RopeNode *new_node,
		struct RopePool *pool);

int rope_node_insert_right(
		struct RopeNode *parent, struct RopeNode *new_node,
		struct RopePool *pool);

int rope_node_delete(struct RopeNode *node, struct RopePool *pool);

int rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr);

int rope_node_merge(
		struct RopeNode *left, struct RopeNode *right, struct RopePool *pool);

struct RopeNode *rope_node_find(
		struct RopeNode *node, rope_index_t line, rope_index_t column,
		rope_byte_index_t *byte_index);

struct RopeNode *rope_node_find_char(
		struct RopeNode *node, rope_char_index_t char_index,
		rope_byte_index_t *byte_index);

enum RopeNodeType rope_node_type(struct RopeNode *node);

enum RopeNodeDirection rope_node_which(struct RopeNode *node);

struct RopeNode *rope_node_sibling(struct RopeNode *node);

struct RopeNode *rope_node_first(struct RopeNode *node);

struct RopeNode *rope_node_last(struct RopeNode *node);

struct RopeNode *rope_node_left(struct RopeNode *node);

struct RopeNode *rope_node_right(struct RopeNode *node);

struct RopeNode *rope_node_parent(struct RopeNode *node);

bool rope_node_up(struct RopeNode **node);

bool rope_node_next(struct RopeNode **node);

bool rope_node_prev(struct RopeNode **node);

const uint8_t *rope_node_value(const struct RopeNode *node, size_t *size);

uint64_t rope_node_tags(struct RopeNode *node);

void rope_node_set_tags(struct RopeNode *node, uint64_t tags);

int rope_node_free(struct RopeNode *node, struct RopePool *pool);

/**********************************
 * rope.c
 */

enum RopeBias {
	ROPE_BIAS_LEFT,
	ROPE_BIAS_RIGHT,
};

struct Rope {
	struct RopeNode *root;
	struct RopeCursor *cursors;
	struct RopePool pool;
	enum RopeBias bias;
};

int rope_init(struct Rope *rope);

int rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size);

int rope_append_str(struct Rope *rope, const char *str);

struct RopeNode *rope_new_node(struct Rope *rope);

struct RopeNode *rope_find(
		struct Rope *rope, rope_char_index_t char_index,
		rope_byte_index_t *byte_index);

int rope_insert(
		struct Rope *rope, size_t index, const uint8_t *data, size_t byte_size);

int rope_delete(struct Rope *rope, size_t index, size_t char_count);

struct RopeNode *rope_first(struct Rope *rope);

int rope_cleanup(struct Rope *rope);

int rope_char_size(struct Rope *rope);

int rope_byte_size(struct Rope *rope);

void rope_node_print(struct RopeNode *root, const char *file);

/**********************************
 * cursor.c
 */

typedef void (*rope_cursor_callback_t)(
		struct Rope *, struct RopeCursor *, void *);

struct RopeCursor {
	rope_char_index_t index;
	rope_index_t line;
	rope_char_index_t column;
	struct Rope *rope;
	struct RopeCursor *next;
	rope_cursor_callback_t callback;
	void *userdata;
};

int rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope);

int rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata);

int rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t char_index);

int rope_cursor_move_to(
		struct RopeCursor *cursor, rope_index_t line, rope_char_index_t column);

int rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size);

int rope_cursor_move(struct RopeCursor *cursor, off_t offset);

int rope_cursor_insert_str(struct RopeCursor *cursor, const char *str);

int rope_cursor_delete(struct RopeCursor *cursor, size_t char_count);

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index);

int32_t rope_cursor_codepoint(struct RopeCursor *cursor);

int rope_cursor_set_codepoint(struct RopeCursor *cursor, int32_t codepoint);

int rope_cursor_cleanup(struct RopeCursor *cursor);

/**********************************
 * range.c
 */

struct RopeRange;

typedef void (*rope_range_callback_t)(
		struct Rope *rope, struct RopeRange *cursor, void *userdata);

struct RopeRange {
	struct Rope *rope;
	struct RopeCursor cursors[2];
	bool is_collapsed;
	rope_range_callback_t offset_change_callback;
	rope_range_callback_t damage_callback;
	void *userdata;
};

int rope_range_init(
		struct RopeRange *range, struct Rope *rope,
		rope_range_callback_t offset_change_callback,
		rope_range_callback_t damage_callback, void *userdata);

struct RopeCursor *rope_range_start(struct RopeRange *range);

struct RopeCursor *rope_range_end(struct RopeRange *range);

int rope_range_insert_str(struct RopeRange *range, const char *str);

int rope_range_delete(struct RopeRange *range);

int rope_range_cleanup(struct RopeRange *range);

/**********************************
 * iterator.c
 */
struct RopeIterator {
	struct RopeRange *range;
	struct RopeNode *left;
	struct RopeNode *current;
	struct RopeNode *right;
	rope_char_index_t current_index;
	rope_char_index_t end_index;
};

int rope_iterator_init(struct RopeIterator *iter, struct RopeRange *range);

bool rope_iterator_next(
		struct RopeIterator *iter, const uint8_t **value, size_t *size);

int rope_iterator_cleanup(struct RopeIterator *iter);

#endif
