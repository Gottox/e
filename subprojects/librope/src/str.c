#include <assert.h>
#include <cextras/macro.h>
#include <cextras/unicode.h>
#include <grapheme.h>
#include <rope_common.h>
#include <rope_error.h>
#include <rope_str.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void str_process(
		struct RopeStr *str, struct RopeDim *dim, size_t *last_char_index,
		const uint8_t *data, size_t byte_size);
static bool str_is_slow(const struct RopeStr *str);

#define ROPE_DIM_ALL \
	((struct RopeDim){{SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX}})

#define GET_SET_EXTRA(name, offset, ...) \
	static void str_set_##name(struct RopeStr *str, size_t value) { \
		str->dim &= ~(ROPE_STR_MASK << (offset)); \
		str->dim |= (value << (offset)); \
	} \
	static size_t str_##name(const struct RopeStr *str) { \
		if (str_is_slow(str)) \
			__VA_ARGS__ \
		return (str->dim >> (offset)) & ROPE_STR_MASK; \
	}
#define GET_SET(name, upper, offset) \
	GET_SET_EXTRA(name, offset, { \
		struct RopeDim dim = ROPE_DIM_ALL; \
		size_t byte_size = 0; \
		const uint8_t *data = rope_str_data(str, &byte_size); \
		str_process(NULL, &dim, NULL, data, byte_size); \
		return dim.dim[ROPE_##upper]; \
	})

GET_SET_EXTRA(last_char_size, 55, {
	size_t last_char_size = 0;
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	str_process(NULL, NULL, &last_char_size, data, byte_size);
	return last_char_size;
})
GET_SET_EXTRA(bytes, 0, { return str->dim & ROPE_STR_SLOW_MASK; })
GET_SET(chars, CHAR, 11)
GET_SET(codepoints, CP, 22)
GET_SET(newlines, LINE, 33)
GET_SET(utf16_cps, UTF16, 44)

static struct RopeDim
str_unit_to_limits(enum RopeUnit unit, size_t index) {
	struct RopeDim limits = {
			{SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX}};
	limits.dim[unit] = index;
	return limits;
}

static bool
str_is_inline(const struct RopeStr *str) {
	return str_bytes(str) <= ROPE_STR_INLINE_SIZE;
}

static bool
str_is_wrapped(const struct RopeStr *str) {
	return !str_is_inline(str) && str->data.heap.str == NULL;
}

static bool
str_is_slow(const struct RopeStr *str) {
	return (str->dim & ~ROPE_STR_SLOW_MASK) == ~ROPE_STR_SLOW_MASK;
}

static uint8_t *
str_heap_data(struct RopeStrHeap *heap) {
	return (uint8_t *)&heap[1];
}

static void
str_heap_release(struct RopeStrHeap *heap_str, uint8_t *data) {
	if (heap_str == NULL) {
		free(data);
	} else if (heap_str->ref_count-- == 0) {
		free(heap_str);
	}
}

static void
str_retain(const struct RopeStr *str) {
	if (!str_is_inline(str)) {
		str->data.heap.str->ref_count++;
	}
}

static void
str_try_inline(struct RopeStr *str, size_t byte_size) {
	size_t old_byte_size = 0;
	const uint8_t *data = rope_str_data(str, &old_byte_size);

	if (byte_size <= ROPE_STR_INLINE_SIZE &&
		old_byte_size > ROPE_STR_INLINE_SIZE) {
		struct RopeStrHeap *heap_str = str->data.heap.str;
		uint8_t *heap_data = str->data.heap.data;
		memcpy(str->data.inplace, heap_data, byte_size);
		str_heap_release(heap_str, heap_data);
		data = str->data.inplace;
	}
	str_process(str, NULL, NULL, data, byte_size);
}

// Check if any dimension limit has been reached.
static bool
str_process_limit(const struct RopeDim *result, const struct RopeDim *limit) {
	for (size_t unit = 0; unit < ROPE_UNIT_COUNT; unit++) {
		// Hint to the compiler that counts can never reach SIZE_MAX.
		if (result->dim[unit] >= SIZE_MAX) {
			ROPE_UNREACHABLE();
		}
		if (result->dim[unit] >= limit->dim[unit]) {
			return true;
		}
	}
	return false;
}

static void
str_process(
		struct RopeStr *str, struct RopeDim *dim, size_t *last_char_size,
		const uint8_t *data, size_t byte_size) {
	struct RopeDim result = {0};
	struct RopeDim limit = ROPE_DIM_ALL;
	if (dim != NULL) {
		limit = *dim;
	}

	assert(byte_size <= ROPE_STR_SLOW_MASK);

	uint_least16_t state = 0;
	uint_least32_t last_cp = GRAPHEME_INVALID_CODEPOINT;
	size_t last_char_start = 0;
	size_t pos = 0;

	while (pos < byte_size && !str_process_limit(&result, &limit)) {
		size_t cp_start = pos;
		uint_least32_t cp;
		size_t cp_size = grapheme_decode_utf8(
				(const char *)&data[pos], byte_size - pos, &cp);
		pos += cp_size;
		result.dim[ROPE_CP] += 1;

		// Count UTF-16 units and newlines.
		if (cp == GRAPHEME_INVALID_CODEPOINT) {
			result.dim[ROPE_UTF16] += 1;
		} else {
			result.dim[ROPE_UTF16] += cp >= 0x10000 ? 2 : 1;
			result.dim[ROPE_LINE] += cp == '\n' ? 1 : 0;
		}

		if (grapheme_is_character_break(last_cp, cp, &state)) {
			last_char_start = cp_start;
			result.dim[ROPE_CHAR] += 1;
		}
		last_cp = cp;
	}

	size_t byte_count = CX_MIN(byte_size, pos);
	result.dim[ROPE_BYTE] = byte_count;

	if (dim != NULL) {
		*dim = result;
	}

	size_t last_char_byte_size = byte_count - last_char_start;
	if (last_char_size != NULL) {
		*last_char_size = last_char_byte_size;
	}
	if (str != NULL) {
		if (byte_size > ROPE_STR_FAST_SIZE || last_char_byte_size >= 512) {
			str->dim = ~ROPE_STR_SLOW_MASK | byte_size;
		} else {
			str_set_bytes(str, result.dim[ROPE_BYTE]);
			str_set_chars(str, result.dim[ROPE_CHAR]);
			str_set_codepoints(str, result.dim[ROPE_CP]);
			str_set_newlines(str, result.dim[ROPE_LINE]);
			str_set_utf16_cps(str, result.dim[ROPE_UTF16]);
			str_set_last_char_size(str, last_char_byte_size);
		}
	}
}

int
rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	uint8_t *buffer = NULL;

	rv = rope_str_alloc(str, byte_size, &buffer);
	if (rv < 0) {
		goto out;
	}

	memcpy(buffer, data, byte_size);
	rope_str_alloc_commit(str, byte_size);
out:
	return rv;
}

void
rope_str_wrap(struct RopeStr *str, uint8_t *data, size_t byte_size) {
	str_set_bytes(str, ROPE_STR_FAST_SIZE);
	str->data.heap.str = NULL;
	str->data.heap.data = data;
	str_try_inline(str, byte_size);
}

int
rope_str_alloc(struct RopeStr *str, size_t byte_size, uint8_t **buffer) {
	int rv = 0;

	memset(str, 0, sizeof(struct RopeStr));

	if (byte_size > ROPE_STR_FAST_SIZE) {
		str->dim = ~ROPE_STR_SLOW_MASK | byte_size;
	} else {
		str_set_bytes(str, byte_size);
	}
	if (byte_size <= ROPE_STR_INLINE_SIZE) {
		*buffer = str->data.inplace;
	} else {
		if (CX_ADD_OVERFLOW(
					byte_size, sizeof(struct RopeStrHeap), &byte_size)) {
			rv = -ROPE_ERROR_OOB;
			goto out;
		}
		str->data.heap.str = calloc(byte_size, 1);
		if (str->data.heap.str == NULL) {
			rv = -ROPE_ERROR_OOM;
			goto out;
		}
		str->data.heap.data = *buffer = str_heap_data(str->data.heap.str);
	}
out:
	return rv;
}

void
rope_str_alloc_commit(struct RopeStr *str, size_t byte_size) {
	size_t old_byte_size = str_bytes(str);
	if (byte_size == SIZE_MAX) {
		byte_size = old_byte_size;
	}

	int rv = rope_str_trim(str, 0, byte_size, ROPE_BYTE);
	assert(rv == 0);
}

int
rope_str_trim(
		struct RopeStr *str, size_t offset, size_t size, enum RopeUnit unit) {
	size_t byte_size = rope_str_unit_to_byte(str, unit, size);
	size_t byte_offset = rope_str_unit_to_byte(str, unit, offset);
	assert(byte_size + byte_offset <= str_bytes(str));

	if (byte_offset == 0) {
		str_try_inline(str, byte_size);
		return 0;
	} else if (str_is_inline(str)) {
		uint8_t *data = str->data.inplace;
		memmove(data, &data[byte_offset], byte_size);
	} else if (str_is_wrapped(str)) {
		uint8_t *data = str->data.heap.data;
		int rv = rope_str_init(str, &data[byte_offset], byte_size);
		free(data);
		return rv;
	} else {
		str->data.heap.data += byte_offset;
	}

	str_try_inline(str, byte_size);
	return 0;
}

void
rope_str_move(struct RopeStr *dest, struct RopeStr *src) {
	memcpy(dest, src, sizeof(struct RopeStr));
	memset(src, 0, sizeof(struct RopeStr));
}

int
rope_str_inline_append(
		struct RopeStr *str, const uint8_t *data, size_t byte_size) {
	size_t new_byte_size = 0;
	size_t old_byte_size = str_bytes(str);
	if (CX_ADD_OVERFLOW(old_byte_size, byte_size, &new_byte_size)) {
		return -ROPE_ERROR_OOB;
	}
	if (new_byte_size > ROPE_STR_INLINE_SIZE) {
		return -ROPE_ERROR_OOB;
	}
	uint8_t *insert = &str->data.inplace[old_byte_size];
	memcpy(insert, data, byte_size);

	str_process(str, NULL, NULL, str->data.inplace, new_byte_size);

	return 0;
}

void
rope_str_split(
		struct RopeStr *str, struct RopeStr *new_str, enum RopeUnit unit,
		size_t index) {
	size_t byte_index = rope_str_unit_to_byte(str, unit, index);
	size_t byte_size = str_bytes(str);
	// TODO: This should never abort, but it does
	// assert(byte_index > 0 && byte_index < byte_size - 1);

	rope_str_clone(new_str, str);
	rope_str_trim(new_str, byte_index, byte_size - byte_index, ROPE_BYTE);
	rope_str_trim(str, 0, byte_index, ROPE_BYTE);
}

const uint8_t *
rope_str_data(const struct RopeStr *str, size_t *byte_count) {
	if (byte_count != NULL) {
		*byte_count = str_bytes(str);
	}
	if (str_is_inline(str)) {
		return str->data.inplace;
	} else {
		return str->data.heap.data;
	}
}

void
rope_str_cleanup(struct RopeStr *str) {
	if (!str_is_inline(str)) {
		str_heap_release(str->data.heap.str, str->data.heap.data);
	}
	memset(str, 0, sizeof(struct RopeStr));
}

size_t
rope_str_dim(const struct RopeStr *str, enum RopeUnit unit) {
	switch (unit) {
	case ROPE_BYTE:
		return str_bytes(str);
	case ROPE_CHAR:
		return str_chars(str);
	case ROPE_CP:
		return str_codepoints(str);
	case ROPE_LINE:
		return str_newlines(str);
	case ROPE_UTF16:
		return str_utf16_cps(str);
	default:
		ROPE_UNREACHABLE();
	}
}

size_t
rope_str_unit_to_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t index) {
	if (unit == ROPE_BYTE) {
		return index;
	}

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	struct RopeDim limits = str_unit_to_limits(unit, index);

	str_process(NULL, &limits, NULL, data, byte_size);
	return limits.dim[ROPE_BYTE];
}

size_t
rope_str_last_char_index(const struct RopeStr *str) {
	return str_bytes(str) - str_last_char_size(str);
}

size_t
rope_str_unit_from_byte(
		const struct RopeStr *str, enum RopeUnit unit, size_t byte_index) {
	if (unit == ROPE_BYTE) {
		return byte_index;
	}

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	struct RopeDim dim = ROPE_DIM_ALL;
	str_process(NULL, &dim, NULL, data, byte_index);

	return dim.dim[unit];
}

bool
rope_str_is_end(struct RopeStr *str, size_t index, enum RopeUnit unit) {
	if (unit == ROPE_LINE) {
		size_t byte_size = 0;
		const uint8_t *data = rope_str_data(str, &byte_size);
		if (byte_size == 0) {
			return true;
		} else if (data[byte_size - 1] != '\n' && index == str_newlines(str)) {
			return false;
		}
	}
	return index >= rope_str_dim(str, unit);
}

int
rope_str_clone(struct RopeStr *target, const struct RopeStr *src) {
	if (str_is_wrapped(src)) {
		size_t byte_size = 0;
		const uint8_t *data = rope_str_data(src, &byte_size);
		return rope_str_init(target, data, byte_size);
	} else {
		memcpy(target, src, sizeof(struct RopeStr));
		str_retain(target);
		return 0;
	}
}
