#ifndef ROPE_STR_H
#define ROPE_STR_H

#include "rope_common.h"
#include <stdint.h>

#define ROPE_STR_MASK ((((uint64_t)1 << 11)) - 1)
#define ROPE_STR_SLOW_MASK ((((uint64_t)1 << 55)) - 1)
#define ROPE_STR_FAST_SIZE (1024 - sizeof(struct RopeStrHeap))
#define ROPE_STR_INLINE_SIZE 48


/**********************************
 * str.c
 */

struct RopeStrHeap {
	uint32_t ref_count;
	// uint8_t data[];
};

struct RopeStr {
	/*
	 * Normal strings:
	 *
	 * | bits    |                   |
	 * |---------|-------------------|
	 * | 00 - 11 | bytes             |
	 * | 12 - 22 | chars             |
	 * | 23 - 33 | codepoints        |
	 * | 34 - 44 | newlines          |
	 * | 45 - 55 | utf16 codepoints  |
	 * | 56 - 64 | last char size    |
	 *
	 * Slow strings:
	 *
	 * | bits    |                   |
	 * |---------|-------------------|
	 * | 00 - 55 | bytes             |
	 * | 56 - 64 | ALL BITS SET TO 1 |
	 *
	 */
	uint64_t dim;
	// struct RopeStrDimensions dimensions;
	union {
		uint8_t inplace[ROPE_STR_INLINE_SIZE];
		struct {
			uint8_t *data;
			struct RopeStrHeap *str;
		} heap;
	} data;
};

ROPE_NO_UNUSED int
rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size);

void rope_str_wrap(struct RopeStr *str, uint8_t *data, size_t byte_size);

ROPE_NO_UNUSED int
rope_str_alloc(struct RopeStr *str, size_t byte_size, uint8_t **data_ptr);

void rope_str_alloc_commit(struct RopeStr *str, size_t byte_size);

void rope_str_move(struct RopeStr *dest, struct RopeStr *src);

ROPE_NO_UNUSED int rope_str_inline_insert(
		struct RopeStr *str, enum RopeUnit unit, size_t index,
		const uint8_t *data, size_t byte_size);

ROPE_NO_UNUSED int rope_str_inline_insert_str(
		struct RopeStr *str, enum RopeUnit unit, size_t index,
		struct RopeStr *insert_str);

ROPE_NO_UNUSED int rope_str_trim(
		struct RopeStr *str, enum RopeUnit unit, size_t offset, size_t size);

ROPE_NO_UNUSED int rope_str_clone_trim(
		struct RopeStr *str, struct RopeStr *src, enum RopeUnit unit,
		size_t offset, size_t size);

ROPE_NO_UNUSED const uint8_t *
rope_str_data(const struct RopeStr *str, size_t *byte_size);

ROPE_NO_UNUSED int
rope_str_split_fast(struct RopeStr *str, struct RopeStr *new_str);

ROPE_NO_UNUSED int rope_str_split(
		struct RopeStr *str, struct RopeStr *new_str, enum RopeUnit unit,
		size_t index);

ROPE_NO_UNUSED size_t
rope_str_size(const struct RopeStr *str, enum RopeUnit unit);

ROPE_NO_UNUSED size_t rope_str_unit_to_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t index);

ROPE_NO_UNUSED size_t rope_str_last_char_index(const struct RopeStr *str);

ROPE_NO_UNUSED size_t rope_str_unit_from_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t byte_index);

ROPE_NO_UNUSED bool
rope_str_is_end(const struct RopeStr *str, enum RopeUnit unit, size_t index);

void rope_str_cleanup(struct RopeStr *str);

ROPE_NO_UNUSED int
rope_str_clone(struct RopeStr *dest, const struct RopeStr *src);

ROPE_NO_UNUSED size_t rope_str_should_stitch(
		const struct RopeStr *left, const struct RopeStr *right,
		uint_least16_t *state);

ROPE_NO_UNUSED int rope_str_stitch(
		struct RopeStr *seam, struct RopeStr *left, size_t left_index,
		struct RopeStr *right, size_t right_index);

/**********************************
 * str_iter.c
 */

struct RopeStrIter {
	struct RopeStr *prev;
	struct RopeStr *str;
};

#endif /* ROPE_STR_H */
