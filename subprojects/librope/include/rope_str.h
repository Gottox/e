#ifndef ROPE_STR_H
#define ROPE_STR_H

#include "rope_common.h"
#include <stdint.h>

#define ROPE_STR_MASK ((uint64_t)4095)
#define ROPE_STR_MAX_SIZE (4095 - sizeof(struct RopeStrHeap))
#define ROPE_STR_INLINE_SIZE (sizeof(void *[2]))

struct RopeStrHeap {
	uint32_t ref_count;
	// uint8_t data[];
};

struct RopeStr {
	/*
	 * | bits    |                  |
	 * |---------|------------------|
	 * | 00 - 11 | bytes            |
	 * | 12 - 23 | chars            |
	 * | 24 - 35 | codepoints       |
	 * | 36 - 47 | newlines         |
	 * | 48 - 59 | utf16 codepoints |
	 * | 60 - 64 | userdata         |
	 */
	uint64_t dim;
	// struct RopeStrDimensions dimensions;
	uint_least16_t state;
	union {
		uint8_t inplace[ROPE_STR_INLINE_SIZE];
		struct {
			uint8_t *data;
			struct RopeStrHeap *str;
		} heap;
	} data;
};

int rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size);

int rope_str_freeable(struct RopeStr *str, uint8_t *data, size_t byte_size);

int rope_str(struct RopeStr *str, size_t byte_size, uint8_t **data_ptr);

void rope_str_truncate(struct RopeStr *str, size_t byte_size);

void rope_str_move(struct RopeStr *dest, struct RopeStr *src);

int rope_str_inline_append(
		struct RopeStr *str, const uint8_t *data, size_t byte_size);

void rope_str_byte_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index);

void rope_str_char_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t char_index);

void rope_str_cp_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t cp_index);

const uint8_t *rope_str_data(const struct RopeStr *str, size_t *byte_size);
const uint8_t *
rope_str_at_char(struct RopeStr *str, size_t char_index, size_t *byte_count);
const uint8_t *
rope_str_at_utf16(struct RopeStr *str, size_t utf16_index, size_t *byte_count);
const uint8_t *
rope_str_at_cp(struct RopeStr *str, size_t cp_index, size_t *byte_size);
void rope_str_utf16_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t utf16_index);
uint_fast16_t rope_str_codepoints(const struct RopeStr *str);
uint_fast16_t rope_str_utf16_cps(const struct RopeStr *str);

size_t rope_str_codepoints(const struct RopeStr *str);

size_t rope_str_chars(const struct RopeStr *str);

size_t rope_str_utf16_cps(const struct RopeStr *str);

size_t rope_str_col_count(const struct RopeStr *str);

size_t rope_str_bytes(const struct RopeStr *str);

bool rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2);

void rope_str_cleanup(struct RopeStr *str);

size_t rope_str_char_to_byte_index(
		const uint8_t *data, size_t byte_size, size_t char_index);

size_t rope_str_measure_char_count(const uint8_t *data, size_t byte_size);

#endif /* ROPE_STR_H */
