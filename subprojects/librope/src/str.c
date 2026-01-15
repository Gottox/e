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

#define ROPE_STR_STATE_APPLY(tgt, op, src) \
	do { \
		tgt.byte_count op src.byte_count; \
		tgt.char_count op src.char_count; \
		tgt.utf16_count op src.utf16_count; \
		tgt.column_count op src.column_count; \
		tgt.cp_count op src.cp_count; \
	} while (0)

static void
heap_str_release(struct RopeStrHeap *heap_str) {
	if (heap_str->ref_count-- == 0) {
		free(heap_str);
	}
}

static int
str_process(
		struct RopeStrState *str, const uint8_t *data, size_t byte_size,
		uint_least16_t state, size_t char_index, size_t utf16_index,
		size_t column_index, size_t cp_index) {
	size_t char_count = 0;
	size_t utf16_count = 0;
	size_t column_count = 0;
	size_t cp_count = 0;
	uint_least32_t cp = GRAPHEME_INVALID_CODEPOINT;
	uint_least32_t last_cp = GRAPHEME_INVALID_CODEPOINT;
	size_t byte_index = 0;
	size_t last_byte_index = 0;
	size_t invalid_index = SIZE_MAX;

	// SIZE_MAX is a magic value. We're asserting a lower value of byte_size, so
	// that we can safely hint the compiler ROPE_UNREACHABLE() later on. CPUs
	// aren't even able to handle such large allocations anyway, so it's more
	// for my state of mind to assert this.
	assert(byte_size < SIZE_MAX);

	size_t last_char_offset = 0;
	for (byte_index = 0; byte_index < byte_size; last_cp = cp) {
		last_byte_index = byte_index;

		size_t cp_size = grapheme_decode_utf8(
				(const char *)&data[byte_index], byte_size - byte_index, &cp);
		byte_index += cp_size;

		if (cp == GRAPHEME_INVALID_CODEPOINT) {
			// mark the first invalid character
			if (invalid_index == SIZE_MAX) {
				invalid_index = last_byte_index;
			}
			utf16_count += 1;
			column_count += 1;
		} else {
			last_char_offset = byte_index;
			utf16_count += cp >= 0x10000 ? 2 : 1;
			column_count += cx_cp_width(cp);
		}
		cp_count += 1;

		if (grapheme_is_character_break(last_cp, cp, &state)) {
			state = 0;
			// Only increment character size as long as we haven't hit an
			// invalid character
			if (char_count <= byte_size) {
				char_count += 1;
			}
		}

		// Hint the compiler that char_size and utf16_size are never reaching
		// SIZE_MAX. This allows to optimize out these checks when str_process
		// used to calculate the byte_indexes.
		if (char_count >= SIZE_MAX || utf16_count >= SIZE_MAX ||
			column_count >= SIZE_MAX) {
			ROPE_UNREACHABLE();
		} else if (char_count >= char_index) {
			break;
		} else if (utf16_count >= utf16_index) {
			break;
		} else if (column_count >= column_index) {
			break;
		} else if (cp_count >= cp_index) {
			break;
		}
	}

	if (str) {
		str->state = state;
		if (invalid_index != SIZE_MAX) {
			str->char_count = SIZE_MAX - invalid_index;
		} else {
			str->char_count = char_count;
		}
		str->cp_count = cp_count;
		str->utf16_count = utf16_count;
		str->byte_count = CX_MIN(byte_index, byte_size);
		str->last_char_size = str->byte_count - last_char_offset;
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
			&str->state, buffer, byte_size, state, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX);
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
			&str->state, data, byte_size, 0, SIZE_MAX, SIZE_MAX, SIZE_MAX,
			SIZE_MAX);
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
			&append_state, insert, byte_size, str->state.state, SIZE_MAX,
			SIZE_MAX, SIZE_MAX, SIZE_MAX);

	ROPE_STR_STATE_APPLY(str->state, +=, append_state);
	str->state.last_char_size = append_state.last_char_size;
	str->state.state = append_state.state;

	return 0;
}

// String splitting

static void
str_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t byte_index,
		size_t char_index, size_t utf16_index, size_t column_index,
		size_t cp_index) {
	size_t char_count = rope_str_char_count(str);

	if (char_index == 0) {
		memset(new_str, 0, sizeof(*new_str));
		return;
	} else if (char_index == char_count) {
		memcpy(new_str, str, sizeof(struct RopeStr));
		memset(str, 0, sizeof(struct RopeStr));
		return;
	}

	memcpy(&new_str->state, &str->state, sizeof(struct RopeStrState));

	size_t byte_count = 0;
	const uint8_t *data = rope_str_data(str, &byte_count);

	size_t left_byte_count = CX_MIN(byte_count, byte_index);
	byte_index = str_process(
			&str->state, data, left_byte_count, str->state.state, char_index,
			utf16_index, column_index, cp_index);

	const uint8_t *split = &data[byte_index];
	ROPE_STR_STATE_APPLY(new_str->state, -=, str->state);
	size_t new_byte_count = rope_str_byte_count(new_str);

	if (new_byte_count <= ROPE_STR_INLINE_SIZE) {
		memcpy(new_str->u.i.data, split, new_byte_count);
	} else {
		new_str->u.h.str = str->u.h.str;
		new_str->u.h.str->ref_count++;
		new_str->u.h.data = split;
	}

	// check if we need to inline the original string
	if (byte_count > ROPE_STR_INLINE_SIZE &&
		byte_index <= ROPE_STR_INLINE_SIZE) {
		struct RopeStrHeap *heap_str = str->u.h.str;

		// We can use fixed-size copy here since we know that the original
		// string was longer than ROPE_STR_INLINE_SIZE
		memcpy(str->u.i.data, data, ROPE_STR_INLINE_SIZE);
		heap_str_release(heap_str);
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
	str_split(str, new_str, SIZE_MAX, SIZE_MAX, SIZE_MAX, SIZE_MAX, cp_index);
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
			NULL, rope_str_data(str, NULL), str->state.byte_count, 0,
			char_index, SIZE_MAX, SIZE_MAX, SIZE_MAX);
	*byte_count = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_utf16(struct RopeStr *str, size_t utf16_index, size_t *byte_count) {
	size_t byte_index = str_process(
			NULL, rope_str_data(str, NULL), str->state.byte_count, 0, SIZE_MAX,
			utf16_index, SIZE_MAX, SIZE_MAX);
	*byte_count = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

const uint8_t *
rope_str_at_cp(struct RopeStr *str, size_t cp_index, size_t *byte_size) {
	size_t byte_count = rope_str_byte_count(str);
	size_t byte_index = str_process(
			NULL, rope_str_data(str, NULL), byte_count, 0, SIZE_MAX, SIZE_MAX,
			SIZE_MAX, cp_index);
	*byte_size = rope_str_byte_count(str) - byte_index;
	return &rope_str_data(str, NULL)[byte_index];
}

size_t
rope_str_char_count(const struct RopeStr *str) {
	size_t char_count = str->state.char_count;
	if (char_count > rope_str_byte_count(str)) {
		return SIZE_MAX;
	}
	return char_count;
}

size_t
rope_str_utf16_count(const struct RopeStr *str) {
	return str->state.utf16_count;
}

size_t
rope_str_col_count(const struct RopeStr *str) {
	return str->state.column_count;
}

size_t
rope_str_cp_count(const struct RopeStr *str) {
	return str->state.cp_count;
}

size_t
rope_str_byte_count(const struct RopeStr *str) {
	return str->state.byte_count;
}

bool
rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2) {
	char buffer[7] = {0};
	uint_least32_t cp1 = 0, cp2 = 0;
	size_t byte_size1 = 0, byte_size2 = 0;
	uint_least16_t state = str1->state.state;

	const uint8_t *data1 = rope_str_data(str1, &byte_size1);
	const uint8_t *data2 = rope_str_data(str2, &byte_size2);
	size_t last_char_size = str1->state.last_char_size;
	size_t last_char_index = byte_size1 - last_char_size;

	// Check for breaks in utf-8 sequences
	if (last_char_size >= 4) {
		return false;
	}
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
		heap_str_release(str->u.h.str);
	}
	memset(str, 0, sizeof(struct RopeStr));
}
