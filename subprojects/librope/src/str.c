#include <assert.h>
#include <cextras/macro.h>
#include <cextras/unicode.h>
#include <grapheme.h>
#include <rope_error.h>
#include <rope_str.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ROPE_STR_IS_INLINE(str) \
	(rope_str_byte_count(str) <= ROPE_STR_INLINE_SIZE)

static void
str_heap_release(struct RopeStrHeap *heap_str) {
	if (heap_str->ref_count-- == 0) {
		free(heap_str);
	}
}

static void
str_retain(const struct RopeStr *str) {
	if (!ROPE_STR_IS_INLINE(str)) {
		str->u.h.str->ref_count++;
	}
}

static int
str_process(
		struct RopeStrState *str, const uint8_t *data, size_t byte_size,
		size_t char_size, size_t utf16_size, size_t column_size, size_t cp_size,
		size_t newline_size, uint_least16_t state) {
	struct RopeStrDimensions dim = {0};
	uint_least32_t cp = GRAPHEME_INVALID_CODEPOINT;
	uint_least32_t last_cp = GRAPHEME_INVALID_CODEPOINT;
	size_t byte_index = 0;
	size_t last_char_byte_index = 0;

	// SIZE_MAX is a magic value. We're asserting a lower value of byte_size, so
	// that we can safely hint the compiler ROPE_UNREACHABLE() later on. CPUs
	// aren't even able to handle such large allocations anyway, so it's more
	// for my state of mind to assert this.
	assert(byte_size < SIZE_MAX);

	for (byte_index = 0; byte_index < byte_size; last_cp = cp) {
		// Hint the compiler that *_size are never reaching SIZE_MAX. This
		// allows to optimize out these checks when str_process used to
		// calculate the byte_indexes.
#define X(f) ((dim.f##_count) >= SIZE_MAX)
		if (X(char) || X(column) || X(cp) || X(newline) || X(utf16)) {
			ROPE_UNREACHABLE();
#undef X
		} else if (dim.char_count >= char_size) {
			break;
		} else if (dim.utf16_count >= utf16_size) {
			break;
		} else if (dim.column_count >= column_size) {
			break;
		} else if (dim.cp_count >= cp_size) {
			break;
		} else if (dim.newline_count >= newline_size) {
			break;
		}

		last_char_byte_index = byte_index;

		size_t cp_size = grapheme_decode_utf8(
				(const char *)&data[byte_index], byte_size - byte_index, &cp);

		if (cp == GRAPHEME_INVALID_CODEPOINT) {
			dim.utf16_count += 1;
			dim.column_count += 1;
		} else {
			const size_t column_width = cx_cp_width(cp);
			const size_t utf16_count = cp >= 0x10000 ? 2 : 1;
			if (column_width + dim.column_count > column_size) {
				break;
			}
			dim.utf16_count += utf16_count;
			dim.column_count += column_width;
			dim.newline_count += cp == '\n' ? 1 : 0;
		}
		byte_index += cp_size;
		dim.cp_count += 1;

		const bool is_partial_utf8 = byte_index > byte_size;
		if (is_partial_utf8) {
			dim.char_count += 1;
		} else if (grapheme_is_character_break(last_cp, cp, &state)) {
			state = 0;
			dim.char_count += 1;
		}
	}
	dim.byte_count = CX_MIN(byte_size, byte_index);

	// Sanity check: last_char_size should never exceed 4 bytes.
	assert(dim.byte_count - last_char_byte_index <= 4);

	if (str) {
		ROPE_STR_DIMENSIONS_APPLY(&str->dimensions, =, &dim);
		str->state = state;
		str->last_char_size = dim.byte_count - last_char_byte_index;
	}

	return byte_index;
}

static int
str_init(
		struct RopeStr *str, const uint8_t *data, size_t byte_size,
		uint_least16_t state) {
	int rv = 0;
	uint8_t *buffer = NULL;

	rv = rope_str(str, byte_size, &buffer);
	if (rv < 0) {
		goto out;
	}

	memcpy(buffer, data, byte_size);
	str_process(
			&str->state, buffer, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, SIZE_MAX, state);
out:
	return rv;
}

int
rope_str_init(struct RopeStr *str, const uint8_t *data, size_t byte_size) {
	return str_init(str, data, byte_size, 0);
}

int
rope_str(struct RopeStr *str, size_t byte_size, uint8_t **buffer) {
	int rv = 0;
	str->state.dimensions.byte_count = byte_size;
	if (byte_size <= ROPE_STR_INLINE_SIZE) {
		*buffer = str->u.i.data;
	} else {
		if (CX_ADD_OVERFLOW(
					byte_size, sizeof(struct RopeStrHeap), &byte_size)) {
			rv = -ROPE_ERROR_OOB;
			goto out;
		}
		str->u.h.str = calloc(byte_size, 1);
		str->u.h.data = *buffer = str->u.h.str->data;
		if (str->u.h.str == NULL) {
			rv = -ROPE_ERROR_OOM;
			goto out;
		}
	}
out:
	return rv;
}

void
rope_str_update(struct RopeStr *str) {
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	str_process(
			&str->state, data, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, SIZE_MAX, 0);
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
	size_t old_byte_size = rope_str_byte_count(str);
	if (CX_ADD_OVERFLOW(old_byte_size, byte_size, &new_byte_size)) {
		return -ROPE_ERROR_OOB;
	}
	if (new_byte_size > ROPE_STR_INLINE_SIZE) {
		return -ROPE_ERROR_OOB;
	}
	uint8_t *insert = str->u.i.data + old_byte_size;
	memcpy(insert, data, byte_size);

	struct RopeStrState append_state = {0};
	str_process(
			&append_state, insert, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, SIZE_MAX, str->state.state);

	ROPE_STR_DIMENSIONS_APPLY(
					&str->state.dimensions, +=, &append_state.dimensions);
	str->state.last_char_size = append_state.last_char_size;
	str->state.state = append_state.state;

	return 0;
}

// String splitting

static void
str_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index,
		size_t char_index, size_t utf16_index, size_t column_index,
		size_t cp_index, size_t newline_index) {
	size_t char_count = rope_str_char_count(str);

	if (char_index == 0) {
		memset(new_str, 0, sizeof(*new_str));
		return;
	} else if (char_index == char_count) {
		memcpy(new_str, str, sizeof(struct RopeStr));
		memset(str, 0, sizeof(struct RopeStr));
		return;
	}

	new_str->state = str->state;

	size_t byte_count = 0;
	const uint8_t *data = rope_str_data(str, &byte_count);

	size_t left_byte_count = CX_MIN(byte_count, byte_index);
	byte_index = str_process(
			&str->state, data, left_byte_count, char_index, utf16_index,
			column_index, cp_index, newline_index, str->state.state);

	const uint8_t *split = &data[byte_index];
	ROPE_STR_DIMENSIONS_APPLY(
					&new_str->state.dimensions, -=, &str->state.dimensions);

	if (ROPE_STR_IS_INLINE(new_str)) {
		size_t new_byte_count = rope_str_byte_count(new_str);
		memcpy(new_str->u.i.data, split, new_byte_count);
	} else {
		new_str->u.h.str = str->u.h.str;
		new_str->u.h.data = split;
		str_retain(new_str);
	}

	// check if we need to inline the original string
	if (byte_count > ROPE_STR_INLINE_SIZE &&
		byte_index <= ROPE_STR_INLINE_SIZE) {
		struct RopeStrHeap *heap_str = str->u.h.str;

		// We can use fixed-size copy here since we know that the original
		// string was longer than ROPE_STR_INLINE_SIZE
		memcpy(str->u.i.data, data, ROPE_STR_INLINE_SIZE);
		str_heap_release(heap_str);
	}
}

void
rope_str_byte_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index) {
	str_split(
			str, new_str, byte_index, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX);
}

void
rope_str_char_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t char_index) {
	str_split(
			str, new_str, SIZE_MAX, char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX);
}

void
rope_str_utf16_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t utf16_index) {
	str_split(
			str, new_str, SIZE_MAX, SIZE_MAX, utf16_index, SIZE_MAX, SIZE_MAX,
			SIZE_MAX);
}

void
rope_str_cp_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t cp_index) {
	str_split(
			str, new_str, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX, cp_index,
			SIZE_MAX);
}

const uint8_t *
rope_str_data(const struct RopeStr *str, size_t *byte_count) {
	if (byte_count != NULL) {
		*byte_count = rope_str_byte_count(str);
	}
	if (ROPE_STR_IS_INLINE(str)) {
		return str->u.i.data;
	} else {
		return str->u.h.data;
	}
}

const uint8_t *
rope_str_at_char(struct RopeStr *str, size_t char_index, size_t *byte_count) {
	size_t byte_index = str_process(
			NULL, rope_str_data(str, NULL), str->state.dimensions.byte_count,
			char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX, 0);
	*byte_count = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_utf16(struct RopeStr *str, size_t utf16_index, size_t *byte_count) {
	size_t byte_index = str_process(
			NULL, rope_str_data(str, NULL), str->state.dimensions.byte_count,
			SIZE_MAX, utf16_index, SIZE_MAX, SIZE_MAX, SIZE_MAX, 0);
	*byte_count = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_cp(struct RopeStr *str, size_t cp_index, size_t *byte_size) {
	size_t byte_count = rope_str_byte_count(str);
	size_t byte_index = str_process(
			NULL, rope_str_data(str, NULL), byte_count, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, cp_index, SIZE_MAX, 0);
	*byte_size = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

size_t
rope_str_char_count(const struct RopeStr *str) {
	return str->state.dimensions.char_count;
}

size_t
rope_str_utf16_count(const struct RopeStr *str) {
	return str->state.dimensions.utf16_count;
}

size_t
rope_str_col_count(const struct RopeStr *str) {
	return str->state.dimensions.column_count;
}

size_t
rope_str_cp_count(const struct RopeStr *str) {
	return str->state.dimensions.cp_count;
}

size_t
rope_str_byte_count(const struct RopeStr *str) {
	return str->state.dimensions.byte_count;
}

bool
rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2) {
	char buffer[8] = {0};
	uint_least32_t cp1 = 0, cp2 = 0;
	size_t byte_size1 = 0, byte_size2 = 0;
	uint_least16_t state = str1->state.state;

	const uint8_t *data1 = rope_str_data(str1, &byte_size1);
	const uint8_t *data2 = rope_str_data(str2, &byte_size2);
	size_t last_char_size = str1->state.last_char_size;
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
	if (!ROPE_STR_IS_INLINE(str)) {
		str_heap_release(str->u.h.str);
	}
	memset(str, 0, sizeof(struct RopeStr));
}

size_t
rope_str_char_to_byte_index(
		const uint8_t *data, size_t byte_size, size_t char_index) {
	return str_process(
			NULL, data, byte_size, char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, 0);
}

size_t
rope_str_measure_char_count(const uint8_t *data, size_t byte_size) {
	struct RopeStrState state = {0};
	str_process(
			&state, data, byte_size, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, 0);
	return state.dimensions.char_count;
}
