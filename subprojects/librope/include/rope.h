#ifndef ROPE_H
#define ROPE_H

#include "rope_error.h"
#include "rope_node.h"
#include <cextras/memory.h>
#include <cextras/unicode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

struct Rope;

/**********************************
 * rc_string.c
 */

struct RopeRcString {
	struct CxRc rc;
	size_t size;
	uint8_t data[];
};

struct RopeRcString *rope_rc_string_new(const uint8_t *data, size_t size);

struct RopeRcString *rope_rc_string_new2(
		const uint8_t *data1, size_t size1, const uint8_t *data2, size_t size2);

struct RopeRcString *rope_rc_string_retain(struct RopeRcString *str);

const uint8_t *rope_rc_string(struct RopeRcString *rc_str, size_t *size);

void rope_rc_string_release(struct RopeRcString *str);

/**********************************
 * string_nav.c
 */

size_t rope_next_line_break(const char *str, size_t len);

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
	struct RopePool pool;
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

char *rope_to_str(struct Rope *rope, uint64_t tags);

struct RopeNode *rope_first(struct Rope *rope);

int rope_cleanup(struct Rope *rope);

int rope_char_size(struct Rope *rope);

int rope_byte_size(struct Rope *rope);

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
	struct RopeCursor *prev;
	rope_cursor_callback_t callback;
	void *userdata;
};

int rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope);

int rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata);

bool rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second);

int rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t char_index, uint64_t tags);

int rope_cursor_move_to(
		struct RopeCursor *cursor, rope_index_t line, rope_char_index_t column);

int rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags);

int rope_cursor_move(struct RopeCursor *cursor, off_t offset);

int rope_cursor_insert_str(
		struct RopeCursor *cursor, const char *str, uint64_t tags);

int rope_cursor_delete(struct RopeCursor *cursor, size_t char_count);

struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index);

int32_t rope_cursor_codepoint(struct RopeCursor *cursor);

int rope_cursor_cleanup(struct RopeCursor *cursor);

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

int rope_range_set_callback(
		struct RopeRange *range, rope_range_callback_t callback,
		void *userdata);

int rope_range_start_move_to(
		struct RopeRange *range, rope_index_t line, rope_char_index_t column);

int rope_range_end_move_to(
		struct RopeRange *range, rope_index_t line, rope_char_index_t column);

int rope_range_start_move_to_index(
		struct RopeRange *range, rope_char_index_t index, uint64_t tags);

int rope_range_end_move_to_index(
		struct RopeRange *range, rope_char_index_t index, uint64_t tags);

void rope_range_set_tags(struct RopeRange *range, uint64_t tags);

uint64_t rope_range_get_tags(struct RopeRange *range);

int
rope_range_insert_str(struct RopeRange *range, const char *str, uint64_t tags);

int rope_range_insert_codepoints(
		struct RopeRange *range, const uint32_t *data, size_t byte_size,
		uint64_t tags);

int rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size,
		uint64_t tags);

int rope_range_delete(struct RopeRange *range);

int rope_range_line(struct RopeRange *range, rope_index_t line);

char *rope_range_to_str(struct RopeRange *range, uint64_t tags);

int rope_range_cleanup(struct RopeRange *range);

/**********************************
 * iterator.c
 */

struct RopeIterator {
	struct RopeRange *range;
	struct RopeNode *node;
	struct RopeNode *end;
	rope_byte_index_t start_byte;
	rope_byte_index_t end_byte;
	bool started;
	uint64_t tags;
};

int rope_iterator_init(
		struct RopeIterator *iter, struct RopeRange *range, uint64_t tags);

bool rope_iterator_next(
		struct RopeIterator *iter, const uint8_t **value, size_t *size);

int rope_iterator_cleanup(struct RopeIterator *iter);

#endif
