#include <assert.h>
#include <cextras/macro.h>
#include <grapheme.h>
#include <rope_error.h>
#include <rope_str.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ROPE_STR_IS_INLINE(str) \
	(rope_str_byte_size(str) <= ROPE_STR_INLINE_SIZE)

static void
heap_str_release(struct RopeStrHeap *heap_str) {
	if (heap_str->ref_count-- == 0) {
		free(heap_str);
	}
}

static int
str_update(
		struct RopeStr *str, const uint8_t *data, size_t byte_size,
		size_t char_index, uint_least16_t state) {
	size_t char_size = 0;
	size_t utf16_size = 0;
	uint_least32_t cp = 0, last_cp = 0;
	size_t i = 0;

	size_t last_char_offset = 0;
	for (i = 0; i < byte_size && char_size < char_index; last_cp = cp) {
		last_char_offset = i;
		size_t cp_size = grapheme_decode_utf8(
				(const char *)&data[i], byte_size - i, &cp);
		if (cp == GRAPHEME_INVALID_CODEPOINT) {
			// mark the first invalid character
			if (char_size <= byte_size) {
				char_size = SIZE_MAX - i;
			}
			i = byte_size;
			break;
		}
		i += cp_size;
		utf16_size += cp >= 0x10000 ? 2 : 1;
		if (i == 0) {
			continue;
		}

		if (grapheme_is_character_break(last_cp, cp, &state)) {
			state = 0;
			// Only increment character size as long as we haven't hit an
			// invalid character
			if (char_size <= byte_size) {
				char_size++;
			}
		}
	}
	str->state = state;
	str->char_size = char_size;
	str->utf16_size = utf16_size;
	str->byte_size = i;
	str->last_char_size = i - last_char_offset;

	return i;
}

int
rope_str_init(
		struct RopeStr *str, const uint8_t *data, size_t byte_size,
		uint_least16_t state) {
	int rv = 0;
	uint8_t *buffer = NULL;

	rv = rope_str(str, byte_size, &buffer);
	if (rv < 0) {
		goto out;
	}

	memcpy(buffer, data, byte_size);
	str_update(str, buffer, byte_size, SIZE_MAX, state);
out:
	return rv;
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
rope_str_update(struct RopeStr *str, uint_least16_t state) {
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	str_update(str, data, byte_size, SIZE_MAX, state);
}

int
rope_str_inline_append(
		struct RopeStr *str, const uint8_t *data, size_t byte_size) {
	size_t new_byte_size = 0;
	if (CX_ADD_OVERFLOW(str->byte_size, byte_size, &new_byte_size)) {
		return -ROPE_ERROR_OOB;
	}
	if (new_byte_size > ROPE_STR_INLINE_SIZE) {
		return -ROPE_ERROR_OOB;
	}
	uint8_t *insert = str->u.i.data + str->byte_size;
	memcpy(insert, data, byte_size);
	str->byte_size = new_byte_size;

	str_update(str, insert, byte_size, SIZE_MAX, str->state);
	return 0;
}

void
rope_str_split(
		struct RopeStr *str, struct RopeStr *new_str, size_t char_index) {
	assert(char_index <= str->char_size);

	if (char_index == 0) {
		memset(new_str, 0, sizeof(*new_str));
		return;
	} else if (char_index == str->char_size) {
		memcpy(new_str, str, sizeof(struct RopeStr));
		memset(str, 0, sizeof(struct RopeStr));
		return;
	}

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(str, &byte_size);
	size_t char_size = rope_str_char_size(str);
	size_t utf16_size = rope_str_utf16_size(str);

	size_t byte_index = str_update(
			str, rope_str_data(str, NULL), str->byte_size, char_index,
			str->state);

	size_t new_byte_size = byte_size - byte_index;
	size_t new_char_size = char_size - rope_str_char_size(str);
	size_t new_utf16_size = utf16_size - rope_str_utf16_size(str);
	size_t new_state = str->state;
	const uint8_t *split = &data[byte_index];
	if (new_byte_size <= ROPE_STR_INLINE_SIZE) {
		rope_str_init(new_str, split, new_byte_size, str->state);
	} else {
		new_str->byte_size = new_byte_size;
		new_str->char_size = new_char_size;
		new_str->utf16_size = new_utf16_size;
		new_str->state = new_state;

		new_str->u.h.str = str->u.h.str;
		new_str->u.h.str->ref_count++;
		new_str->u.h.data = split;
	}

	// check if we need to inline the original string
	if (byte_size > ROPE_STR_INLINE_SIZE &&
		byte_index <= ROPE_STR_INLINE_SIZE) {
		struct RopeStrHeap *heap_str = str->u.h.str;
		memcpy(str->u.i.data, data, ROPE_STR_INLINE_SIZE);
		heap_str_release(heap_str);
	}
}

const uint8_t *
rope_str_data(struct RopeStr *str, size_t *byte_size) {
	if (byte_size != NULL) {
		*byte_size = rope_str_byte_size(str);
	}
	if (ROPE_STR_IS_INLINE(str)) {
		return str->u.i.data;
	} else {
		return str->u.h.data;
	}
}

size_t
rope_str_char_size(struct RopeStr *str) {
	size_t char_size = str->char_size;
	if (char_size > rope_str_byte_size(str)) {
		return SIZE_MAX;
	}
	return char_size;
}

size_t
rope_str_utf16_size(struct RopeStr *str) {
	return str->utf16_size;
}

size_t
rope_str_byte_size(struct RopeStr *str) {
	return str->byte_size;
}

bool
rope_str_should_merge(struct RopeStr *str1, struct RopeStr *str2) {
	char buffer[8] = {0};
	uint_least32_t cp1 = 0, cp2 = 0;
	size_t byte_size1 = 0, byte_size2 = 0;
	uint_least16_t state = str1->state;

	const uint8_t *data1 = rope_str_data(str1, &byte_size1);
	const uint8_t *data2 = rope_str_data(str2, &byte_size2);
	size_t last_char_size = str1->last_char_size;
	size_t last_char_index = byte_size1 - last_char_size;

	// Check for breaks in utf-8 sequences
	memcpy(buffer, &data1[last_char_index], str1->last_char_size);
	memcpy(&buffer[last_char_size], data2, 4);
	size_t cp_size = grapheme_decode_utf8(buffer, 4, &cp1);
	if (cp1 == GRAPHEME_INVALID_CODEPOINT) {
		return false;
	} else if (cp_size > last_char_size) {
		return true;
	}

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
}
