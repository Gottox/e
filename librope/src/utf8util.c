#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <utf8util.h>

/*
 * Map was generated using the following source:
 *
 * #include <stdio.h>
 * int
 * main(void) {
 * 	for (int i = 0; i < 256; i++) {
 * 		if ((i & 0x80) == 0) {
 * 			putchar('1');
 * 		} else if ((i & 0xE0) == 0xC0) {
 * 			putchar('2');
 * 		} else if ((i & 0xF0) == 0xE0) {
 * 			putchar('3');
 * 		} else if ((i & 0xF8) == 0xF0) {
 * 			putchar('4');
 * 		} else {
 * 			putchar('0');
 * 		}
 * 		fputs((i + 1) % 26 && i != 255 ? ", " : ",\n", stdout);
 * 	}
 * }
 */
static const uint8_t utf8_len_map[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*
 * Map was generated using the following source:
 *
 * #include <stdio.h>
 * int
 * main(void) {
 * 	for (int i = 0; i < 256; i++) {
 * 		if ((i & 0x80) == 0 || (i & 0xE0) == 0xC0) {
 * 			putchar('1');
 * 		} else if ((i & 0xF0) == 0xE0 || (i & 0xF8) == 0xF0) {
 * 			putchar('2');
 * 		} else {
 * 			putchar('0');
 * 		}
 * 		fputs((i + 1) % 26 && i != 255 ? ", " : ",\n", stdout);
 * 	}
 * }
 */
static const uint8_t utf16_len_map[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0,
};

ssize_t
utf8_clen(const uint8_t *str, size_t length) {
	size_t count = 0;
	for (size_t i = 0; i < length; count++) {
		uint8_t char_len = utf8_len_map[str[i]];
		if (char_len == 0) {
			return -1;
		}
		i += char_len;
	}
	return count;
}

ssize_t
utf8_bidx(const uint8_t *str, size_t length, size_t char_index) {
	size_t byte_index = 0;
	for (size_t i = 0; byte_index < length && i < char_index; i++) {
		uint8_t char_len = utf8_len_map[str[byte_index]];
		if (char_len == 0) {
			return -1;
		}
		byte_index += char_len;
	}
	if (byte_index > length) {
		return -1;
	}
	return byte_index;
}

ssize_t
utf8_16len(const uint8_t *str, size_t length) {
	size_t len16 = 0;
	for (size_t i = 0; i < length;) {
		uint8_t char_len = utf8_len_map[str[i]];
		len16 += utf16_len_map[str[i]];
		if (char_len == 0) {
			return -1;
		}
		i += char_len;
	}

	return len16;
}
