#include <rope.h>
#include <stdbool.h>
#include <stddef.h>

#if 1
#	include <grapheme.h>

size_t
rope_utf8_char_to_byte_index(
		const uint8_t *str, rope_byte_index_t byte_length,
		rope_char_index_t char_index) {
	const uint8_t *p = str;
	for (; char_index; char_index--) {
		size_t char_size = grapheme_next_character_break_utf8(
				(const char *)p, byte_length);
		p += char_size;
		byte_length -= char_size;
	}
	return p - str;
}

size_t
rope_utf8_char_length(const uint8_t *str, rope_byte_index_t byte_length) {
	int chars = 0;
	for (; byte_length; chars++) {
		size_t char_size = grapheme_next_character_break_utf8(
				(const char *)str, byte_length);
		str += char_size;
		byte_length -= char_size;
	}
	return chars;
}

#else
#	include <cextras/unicode.h>

size_t
rope_utf8_char_to_byte_index(
		const uint8_t *str, rope_byte_index_t byte_length,
		rope_char_index_t char_index) {
	return cx_utf8_bidx(str, byte_length, char_index);
}

size_t
rope_utf8_char_length(const uint8_t *str, rope_byte_index_t byte_length) {
	return cx_utf8_clen(str, byte_length);
}
#endif
