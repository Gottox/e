#ifndef ROPE_STR_H
#define ROPE_STR_H

#include "rope_common.h"
#include <stdint.h>
#include <sys/types.h>

#define ROPE_STR_INLINE_SIZE (sizeof(void *[2]))

struct RopeStrState {
	size_t byte_count;
	size_t cp_count;
	size_t char_count;
	size_t utf16_count;
	size_t column_count;
	uint_least16_t state;
	uint_least8_t last_char_size;
};
struct RopeStrHeap {
	size_t ref_count;
	uint8_t data[];
};

struct RopeStr {
	struct RopeStrState state;
	union {
		struct {
			uint8_t data[ROPE_STR_INLINE_SIZE];
		} i;
		struct {
			struct RopeStrHeap *str;
			const uint8_t *data;
		} h;
	} u;
};

int rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size);

int rope_str(struct RopeStr *str, size_t byte_size, uint8_t **data_ptr);

void rope_str_update(struct RopeStr *str);

int rope_str_inline_append(
		struct RopeStr *str, const uint8_t *data, size_t byte_size);

void rope_str_byte_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index);

void rope_str_char_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t char_index);

void rope_str_cp_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t cp_index);

const uint8_t *rope_str_data(const struct RopeStr *str, size_t *byte_size);

size_t rope_str_cp_count(const struct RopeStr *str);

size_t rope_str_char_count(const struct RopeStr *str);

size_t rope_str_utf16_count(const struct RopeStr *str);

size_t rope_str_col_count(const struct RopeStr *str);

size_t rope_str_byte_count(const struct RopeStr *str);

bool rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2);

void rope_str_cleanup(struct RopeStr *str);

#endif /* ROPE_STR_H */
