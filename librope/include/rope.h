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
	size_t utf16_size;
	size_t new_lines;
};

int rope_node_init(struct RopeNode *node);

struct RopeNode *
rope_node_find(struct RopeNode *node, rope_char_index_t *index);

struct RopeNode *rope_node_find_line(struct RopeNode *node, rope_index_t *line);

size_t
rope_node_line_number(const struct RopeNode *node, rope_char_index_t index);

int rope_node_insert(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index,
		const uint8_t *data, size_t byte_size);

int rope_node_delete(struct RopeNode *node, struct Rope *rope);

bool rope_node_next(struct RopeNode **node);

bool rope_node_prev(struct RopeNode **node);

int rope_node_split(
		struct RopeNode *node, struct Rope *rope, rope_char_index_t index);

const uint8_t *rope_node_value(const struct RopeNode *node, size_t *size);

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

struct RopeNode *
rope_line(struct Rope *rope, rope_index_t line, rope_byte_index_t *index);

struct RopeNode *rope_first(struct Rope *rope);

struct RopeNode *rope_node_left(struct RopeNode *base);

struct RopeNode *rope_node_right(struct RopeNode *base);

const uint8_t *rope_node_value(const struct RopeNode *node, size_t *size);

int rope_cleanup(struct Rope *rope);

void rope_print_tree(struct Rope *rope, FILE *out);

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

int rope_cursor_init(
		struct RopeCursor *cursor, struct Rope *rope, rope_char_index_t index,
		rope_cursor_callback_t callback, void *userdata);

int rope_cursor_set(
		struct RopeCursor *cursor, rope_index_t line, rope_char_index_t column);

int rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size);

int rope_cursor_move(struct RopeCursor *cursor, off_t offset);

int rope_cursor_insert_str(struct RopeCursor *cursor, const char *str);

int rope_cursor_delete(struct RopeCursor *cursor, size_t char_count);

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index);

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
	struct RopeCursor *left;
	struct RopeCursor *right;
	bool is_collapsed;
	rope_range_callback_t offset_change_callback;
	rope_range_callback_t damage_callback;
	void *userdata;
};

int rope_range_init(
		struct RopeRange *range, struct Rope *rope,
		rope_range_callback_t offset_change_callback,
		rope_range_callback_t damage_callback, void *userdata);

struct RopeCursor *rope_range_left(struct RopeRange *range);

struct RopeCursor *rope_range_right(struct RopeRange *range);

int rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size);

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
