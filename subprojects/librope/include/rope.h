#ifndef ROPE_H
#define ROPE_H

#include "rope_error.h"
#include "rope_node.h"
#include "rope_str.h"
#include <cextras/memory.h>
#include <cextras/unicode.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

struct Rope;
struct RopeRange;

/**********************************
 * pool.c
 */

struct RopePool {
	struct CxPreallocPool pool;
};

int rope_pool_init(struct RopePool *pool);

struct RopeNode *rope_pool_get(struct RopePool *pool);

void rope_pool_recycle(struct RopePool *pool, struct RopeNode *node);

void rope_pool_cleanup(struct RopePool *pool);

/**********************************
 * rope.c
 */

struct Rope {
	struct RopeNode *root;
	struct RopeCursor *last_cursor;
	struct RopePool *pool;
	size_t chores_counter;
};

int rope_init(struct Rope *rope, struct RopePool *pool);

int rope_chores(struct Rope *rope);

int rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size);

int rope_append_str(struct Rope *rope, const char *str);

size_t rope_size(struct Rope *rope, enum RopeUnit unit);

int rope_insert(
		struct Rope *rope, enum RopeUnit unit, size_t index,
		const uint8_t *data, size_t byte_size);

int
rope_delete(struct Rope *rope, enum RopeUnit unit, size_t index, size_t count);

int rope_to_range(struct Rope *rope, struct RopeRange *range);

char *rope_to_str(struct Rope *rope, uint64_t tags);

int rope_to_end_cursor(struct Rope *rope, struct RopeCursor *cursor);

void rope_clear(struct Rope *rope);

void rope_cleanup(struct Rope *rope);

/**********************************
 * cursor.c
 */

struct RopeCursor;

typedef void (*rope_cursor_callback_t)(
		struct Rope *, struct RopeCursor *, void *);

struct RopeCursor {
	size_t byte_index;
	struct Rope *rope;
	struct RopeCursor *prev;
	rope_cursor_callback_t callback;
	void *userdata;
};

int rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope);

int rope_cursor_clone(struct RopeCursor *cursor, struct RopeCursor *from);

void rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata);

bool rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second);

int rope_cursor_move_to(
		struct RopeCursor *cursor, enum RopeUnit unit, size_t index,
		uint64_t tags);

int rope_cursor_insert(
		struct RopeCursor *cursor, struct RopeStr *str, uint64_t tags);

int rope_cursor_insert_data(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags);

int rope_cursor_move_by(
		struct RopeCursor *cursor, enum RopeUnit unit, off_t offset);

int rope_cursor_move(struct RopeCursor *cursor, off_t offset);

int rope_cursor_insert_str(
		struct RopeCursor *cursor, const char *str, uint64_t tags);

int
rope_cursor_delete(struct RopeCursor *cursor, enum RopeUnit unit, size_t count);

int
rope_cursor_insert_cp(struct RopeCursor *cursor, uint32_t cp, uint64_t tags);

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, size_t *byte_index);

uint_least32_t rope_cursor_cp(struct RopeCursor *cursor);

bool rope_cursor_is_eof(struct RopeCursor *cursor);

size_t
rope_cursor_index(struct RopeCursor *cursor, enum RopeUnit unit, uint64_t tags);

bool rope_cursor_starts_with_data(
		struct RopeCursor *cursor, const uint8_t *prefix, size_t prefix_size);

void rope_cursor_cleanup(struct RopeCursor *cursor);

/**********************************
 * range.c
 */

struct RopeRange;

typedef void (*rope_range_callback_t)(
		struct Rope *rope, struct RopeRange *range, bool damaged,
		void *userdata);

struct RopeRange {
	struct Rope *rope;
	// struct RopeCursor cursors[2];
	struct RopeCursor cursor_start;
	struct RopeCursor cursor_end;
	bool is_collapsed;
	rope_range_callback_t callback;
	void *callback_userdata;
};

int rope_range_init(struct RopeRange *range, struct Rope *rope);

void rope_range_set_callback(
		struct RopeRange *range, rope_range_callback_t callback,
		void *userdata);

int
rope_range_insert_str(struct RopeRange *range, const char *str, uint64_t tags);

int rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size,
		uint64_t tags);

struct RopeCursor *rope_range_start(struct RopeRange *range);

struct RopeCursor *rope_range_end(struct RopeRange *range);

int rope_range_delete(struct RopeRange *range);

int
rope_range_to_str(struct RopeRange *range, struct RopeStr *str, uint64_t tags);

int rope_range_copy_to(
		struct RopeRange *range, struct RopeCursor *target, uint64_t tags);

char *rope_range_to_cstr(struct RopeRange *range, uint64_t tags);

void rope_range_collapse(struct RopeRange *range, enum RopeDirection dir);

size_t rope_range_size(struct RopeRange *range, enum RopeUnit unit);

void rope_range_clone(struct RopeRange *range, struct RopeRange *from);

void rope_range_cleanup(struct RopeRange *range);

/**********************************
 * iterator.c
 */

struct RopeIterator {
	struct RopeRange *range;
	struct RopeNode *node;
	struct RopeNode *end;
	size_t start_byte;
	size_t end_byte;
	bool started;
	uint64_t tags;
};

int rope_iterator_init(
		struct RopeIterator *iter, struct RopeRange *range, uint64_t tags);

bool rope_iterator_next(struct RopeIterator *iter, struct RopeStr *str);

void rope_iterator_cleanup(struct RopeIterator *iter);

#endif
