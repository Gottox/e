#include <assert.h>
#include <cextras/macro.h>
#include <cextras/unicode.h>
#include <grapheme.h>
#include <rope_common.h>
#include <rope_error.h>
#include <rope_str.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DIM(name, offset) \
	static void str_set_##name(struct RopeStr *str, uint_fast16_t value) { \
		assert(value <= ROPE_STR_MAX_SIZE); \
		str->dim &= ~(ROPE_STR_MASK << (offset)); \
		str->dim |= (value << (offset)); \
	} \
	uint_fast16_t rope_str_##name(const struct RopeStr *str) { \
		return (str->dim >> (offset)) & ROPE_STR_MASK; \
	}

DIM(bytes, 0)
DIM(chars, 12)
DIM(codepoints, 24)
DIM(newlines, 36)
DIM(utf16_cps, 48)
DIM(last_char_size, 60)

static bool
str_is_inline(const struct RopeStr *str) {
	return rope_str_bytes(str) <= ROPE_STR_INLINE_SIZE;
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

static int
str_process(
		struct RopeStr *str, struct RopeDim *dim, const uint8_t *data,
		size_t byte_size, size_t char_size, size_t utf16_size, size_t cp_size,
		size_t newline_size, uint_least16_t state) {
	uint_least32_t cp;
	uint_least32_t last_cp = GRAPHEME_INVALID_CODEPOINT;
	size_t byte_index = 0;
	size_t last_char_byte_index = 0;
	struct RopeDim local_dim = {0};

	if (dim == NULL) {
		dim = &local_dim;
	}

	// SIZE_MAX is a magic value. We're asserting a lower value of byte_size, so
	// that we can safely hint the compiler ROPE_UNREACHABLE() later on. CPUs
	// aren't even able to handle such large allocations anyway, so it's more
	// for my state of mind to assert this.
	assert(byte_size < SIZE_MAX);

	for (byte_index = 0; byte_index < byte_size; last_cp = cp) {
		// Hint the compiler that *_size are never reaching SIZE_MAX. This
		// allows to optimize out these checks when str_process used to
		// calculate the byte_indexes.
#define X(f) ((dim->f##_count) >= SIZE_MAX)
		if (X(char) || X(cp) || X(newline) || X(utf16)) {
			ROPE_UNREACHABLE();
#undef X
		} else if (dim->char_count >= char_size) {
			break;
		} else if (dim->utf16_count >= utf16_size) {
			break;
		} else if (dim->cp_count >= cp_size) {
			break;
		} else if (dim->newline_count >= newline_size) {
			break;
		}

		last_char_byte_index = byte_index;

		size_t cp_size = grapheme_decode_utf8(
				(const char *)&data[byte_index], byte_size - byte_index, &cp);

		if (cp == GRAPHEME_INVALID_CODEPOINT) {
			dim->utf16_count += 1;
		} else {
			const size_t utf16_count = cp >= 0x10000 ? 2 : 1;
			dim->utf16_count += utf16_count;
			dim->newline_count += cp == '\n' ? 1 : 0;
		}
		byte_index += cp_size;
		dim->cp_count += 1;

		const bool is_partial_utf8 = byte_index > byte_size;
		if (is_partial_utf8) {
			dim->char_count += 1;
		} else if (grapheme_is_character_break(last_cp, cp, &state)) {
			state = 0;
			dim->char_count += 1;
		}
	}
	dim->byte_count = CX_MIN(byte_size, byte_index);

	// Sanity check: last_char_size should never exceed 4 bytes.
	const size_t last_char_size = dim->byte_count - last_char_byte_index;
	assert(last_char_size <= 4);

	if (str) {
		str_set_bytes(str, dim->byte_count);
		str_set_chars(str, dim->char_count);
		str_set_codepoints(str, dim->cp_count);
		str_set_newlines(str, dim->newline_count);
		str_set_utf16_cps(str, dim->utf16_count);
		str_set_last_char_size(str, last_char_size);

		str->state = state;
	}

	return byte_index;
}

int
rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	uint8_t *buffer = NULL;

	rv = rope_str(str, byte_size, &buffer);
	if (rv < 0) {
		goto out;
	}

	memcpy(buffer, data, byte_size);
	str_process(
			str, NULL, buffer, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, 0);
out:
	return rv;
}

int
rope_str_freeable(struct RopeStr *str, uint8_t *data, size_t byte_size) {
	int rv = 0;
	if (byte_size > ROPE_STR_MAX_SIZE) {
		rv = -ROPE_ERROR_OOB;
		goto out;
	}

	if (byte_size <= ROPE_STR_INLINE_SIZE) {
		rv = rope_str_init(str, data, byte_size);
		free(data);
		goto out;
	} else {
		str->data.heap.str = NULL;
		str->data.heap.data = data;
		str_process(
				str, NULL, data, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX,
				SIZE_MAX, 0);
	}
out:
	return rv;
}

int
rope_str(struct RopeStr *str, size_t byte_size, uint8_t **buffer) {
	int rv = 0;

	if (byte_size > ROPE_STR_MAX_SIZE) {
		rv = -ROPE_ERROR_OOB;
		goto out;
	}

	memset(str, 0, sizeof(struct RopeStr));

	str_set_bytes(str, byte_size);
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
rope_str_truncate(struct RopeStr *str, size_t byte_size) {
	size_t original_byte_size = 0;
	const uint8_t *data = rope_str_data(str, &original_byte_size);
	if (byte_size == SIZE_MAX) {
		byte_size = original_byte_size;
	}
	assert(byte_size <= original_byte_size);
	str_process(
			str, NULL, data, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			0);
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
	size_t old_byte_size = rope_str_bytes(str);
	if (CX_ADD_OVERFLOW(old_byte_size, byte_size, &new_byte_size)) {
		return -ROPE_ERROR_OOB;
	}
	if (new_byte_size > ROPE_STR_INLINE_SIZE) {
		return -ROPE_ERROR_OOB;
	}
	uint8_t *insert = &str->data.inplace[old_byte_size];
	memcpy(insert, data, byte_size);

	str_process(
			str, NULL, str->data.inplace, new_byte_size, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, SIZE_MAX, 0);

	return 0;
}

// String splitting

static void
str_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index,
		size_t char_index, size_t utf16_index, size_t cp_index,
		size_t newline_index) {
	if (byte_index == SIZE_MAX) {
		byte_index = rope_str_bytes(str);
	}
	assert(byte_index < 4096);
	memset(new_str, 0, sizeof(*new_str));
	new_str->state = str->state;

	size_t byte_count = 0;
	const uint8_t *data = rope_str_data(str, &byte_count);

	const size_t left_byte_count =
			byte_index == SIZE_MAX ? byte_count : byte_index;
	byte_index = str_process(
			str, NULL, data, left_byte_count, char_index, utf16_index, cp_index,
			newline_index, str->state);

	assert(byte_index > 0 && byte_index < byte_count);

	uint8_t *split = (uint8_t *)&data[byte_index];

	size_t new_byte_count = byte_count - byte_index;
	if (new_byte_count <= ROPE_STR_INLINE_SIZE) {
		memcpy(new_str->data.inplace, split, new_byte_count);
	} else if (str->data.heap.str == NULL) {
		rope_str_init(new_str, split, new_byte_count);
	} else {
		new_str->data.heap.str = str->data.heap.str;
		new_str->data.heap.data = split;
		str_set_bytes(new_str, new_byte_count);
		str_retain(new_str);
	}
	str_process(
			new_str, NULL, split, new_byte_count, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, 0);

	// check if we need to inline the original string
	if (byte_count > ROPE_STR_INLINE_SIZE &&
		byte_index <= ROPE_STR_INLINE_SIZE) {
		struct RopeStrHeap *heap_str = str->data.heap.str;
		uint8_t *data = str->data.heap.data;

		// We can use fixed-size copy here since we know that the original
		// string was longer than ROPE_STR_INLINE_SIZE
		memcpy(str->data.inplace, data, ROPE_STR_INLINE_SIZE);
		str_heap_release(heap_str, data);
	}
}

void
rope_str_byte_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index) {
	str_split(str, new_str, byte_index, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX);
}

void
rope_str_char_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t char_index) {
	str_split(str, new_str, SIZE_MAX, char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX);
}

void
rope_str_utf16_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t utf16_index) {
	str_split(
			str, new_str, SIZE_MAX, SIZE_MAX, utf16_index, SIZE_MAX, SIZE_MAX);
}

void
rope_str_cp_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t cp_index) {
	str_split(str, new_str, SIZE_MAX, SIZE_MAX, SIZE_MAX, cp_index, SIZE_MAX);
}

const uint8_t *
rope_str_data(const struct RopeStr *str, size_t *byte_count) {
	if (byte_count != NULL) {
		*byte_count = rope_str_bytes(str);
	}
	if (str_is_inline(str)) {
		return str->data.inplace;
	} else {
		return str->data.heap.data;
	}
}

const uint8_t *
rope_str_at_char(struct RopeStr *str, size_t char_index, size_t *byte_count) {
	size_t size = 0;
	const uint8_t *data = rope_str_data(str, &size);
	size_t byte_index = str_process(
			NULL, NULL, data, size, char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			0);
	*byte_count = rope_str_bytes(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_utf16(struct RopeStr *str, size_t utf16_index, size_t *byte_count) {
	size_t size = 0;
	const uint8_t *data = rope_str_data(str, &size);
	size_t byte_index = str_process(
			NULL, NULL, data, size, SIZE_MAX, utf16_index, SIZE_MAX, SIZE_MAX,
			0);
	*byte_count = rope_str_bytes(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_cp(struct RopeStr *str, size_t cp_index, size_t *byte_size) {
	size_t size = 0;
	const uint8_t *data = rope_str_data(str, &size);
	size_t byte_index = str_process(
			NULL, NULL, data, size, SIZE_MAX, SIZE_MAX, cp_index, SIZE_MAX, 0);
	*byte_size = rope_str_bytes(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

bool
rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2) {
	char buffer[8] = {0};
	uint_least32_t cp1 = 0, cp2 = 0;
	size_t byte_size1 = 0, byte_size2 = 0;
	uint_least16_t state = str1->state;

	const uint8_t *data1 = rope_str_data(str1, &byte_size1);
	const uint8_t *data2 = rope_str_data(str2, &byte_size2);
	size_t last_char_size = rope_str_last_char_size(str1);
	size_t last_char_index = byte_size1 - last_char_size;

	// Check for breaks in utf-8 sequences
	memcpy(buffer, &data1[last_char_index], last_char_size);
	memcpy(&buffer[last_char_size], data2, 4);
	size_t cp_size = grapheme_decode_utf8(buffer, 4, &cp1);
	if (cp1 == GRAPHEME_INVALID_CODEPOINT) {
		return false;
	} else if (cp_size > last_char_size) {
		return true;
	}

	// Check for graphemes across the boundary
	grapheme_decode_utf8((const char *)data2, byte_size2, &cp2);
	if (cp2 == GRAPHEME_INVALID_CODEPOINT) {
		return false;
	}

	return !grapheme_is_character_break(cp1, cp2, &state);
}

void
rope_str_cleanup(struct RopeStr *str) {
	if (!str_is_inline(str)) {
		str_heap_release(str->data.heap.str, str->data.heap.data);
	}
	memset(str, 0, sizeof(struct RopeStr));
}

size_t
rope_str_char_to_byte_index(
		const uint8_t *data, size_t byte_size, size_t char_index) {
	return str_process(
			NULL, NULL, data, byte_size, char_index, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, 0);
}

size_t
rope_str_measure_char_count(const uint8_t *data, size_t byte_size) {
	struct RopeDim dim = {0};
	str_process(
			NULL, &dim, data, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			0);
	return dim.char_count;
}
