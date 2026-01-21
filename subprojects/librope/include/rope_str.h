#ifndef ROPE_STR_H
#define ROPE_STR_H

#include "rope_common.h"
#include <stdint.h>

#define ROPE_STR_MASK ((((uint64_t)1 << 11)) - 1)
#define ROPE_STR_SLOW_MASK ((((uint64_t)1 << 55)) - 1)
#define ROPE_STR_FAST_SIZE (1023 - sizeof(struct RopeStrHeap))
#define ROPE_STR_INLINE_SIZE (sizeof(void *[2]))

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

int rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size);

void rope_str_wrap(struct RopeStr *str, uint8_t *data, size_t byte_size);

int rope_str_alloc(struct RopeStr *str, size_t byte_size, uint8_t **data_ptr);

void rope_str_alloc_commit(struct RopeStr *str, size_t byte_size);

void rope_str_move(struct RopeStr *dest, struct RopeStr *src);

int rope_str_inline_append(
		struct RopeStr *str, const uint8_t *data, size_t byte_size);

int rope_str_trim(
		struct RopeStr *str, size_t offset, size_t size, enum RopeUnit unit);

const uint8_t *rope_str_data(const struct RopeStr *str, size_t *byte_size);

const uint8_t *rope_str_offset(
		struct RopeStr *str, enum RopeUnit unit, size_t index,
		size_t *byte_count);

void rope_str_split(
		struct RopeStr *str, struct RopeStr *new_str, enum RopeUnit unit,
		size_t index);

size_t rope_str_dim(const struct RopeStr *str, enum RopeUnit unit);

size_t rope_str_unit_to_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t index);

size_t rope_str_last_char_index(const struct RopeStr *str);

size_t rope_str_unit_from_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t byte_index);

bool rope_str_is_end(struct RopeStr *str, size_t index, enum RopeUnit unit);

bool rope_str_ends_with_newline(struct RopeStr *str);

void rope_str_cleanup(struct RopeStr *str);

int rope_str_clone(struct RopeStr *dest, const struct RopeStr *src);

#endif /* ROPE_STR_H */
