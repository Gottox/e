#include <ctype.h>
#include <e_utils.h>
#include <stdlib.h>
#include <string.h>

int
e_array_push(void **array, int *size, void *value) {
	size_t new_size = *size + 1;
	void **new_array = realloc(*array, new_size * sizeof(void *));
	if (!new_array) {
		return -1; // Memory allocation failed
	}
	new_array[*size] = value;
	*array = new_array;
	*size = new_size;
	return 0;
}

int
e_array_remove(void **array, int *size, int index) {
	if (index < 0 || index >= *size || *size == 0) {
		return -1; // Index out of bounds
	}
	size_t move_size = (*size - index - 1) * sizeof(void *);
	memmove(&array[index], &array[index + 1], move_size);
	*size -= 1;
	return 0;
}

int
e_parse_unsigned_dec(struct RopeCursor *cursor, uint64_t *number) {
	*number = 0;
	int rv = -1;
	while (!rope_cursor_is_eof(cursor)) {
		uint64_t c = rope_cursor_cp(cursor);
		if (c >= '0' && c <= '9') {
			rv = 0;
			*number = (*number * 10) + (c - '0');
			rv = rope_cursor_move_by(cursor, ROPE_BYTE, 1);
		} else {
			break;
		}
	}
	return rv; // Return -1 if no digits were found
}

int
e_parse_unsigned_hex(
		struct RopeCursor *cursor, uint64_t *number, size_t digits) {
	(void)digits;
	*number = 0;
	int rv = -1;
	int val;
	while (!rope_cursor_is_eof(cursor)) {
		uint64_t c = rope_cursor_cp(cursor);
		c = tolower(c);
		if (c >= '0' && c <= '9') {
			val = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			val = c - 'a' + 10;
		} else {
			break;
		}
		*number = (*number << 4) + val;
		rv = rope_cursor_move_by(cursor, ROPE_BYTE, 1);
		if (rv < 0) {
			goto out;
		}
	}
out:
	return rv; // Return -1 if no digits were found
}

int
e_write_unsigned_dec(struct RopeCursor *cursor, uint64_t number) {
	if (number == 0) {
		return rope_cursor_insert_str(cursor, "0", 0);
	}

	char buffer[32] = {0}; // Enough to hold max uint64_t
	char *p = &buffer[sizeof(buffer) - 1];
	while (number > 0) {
		p--;
		*p = '0' + (number % 10);
		number /= 10;
	}

	return rope_cursor_insert_str(cursor, p, 0);
}

int
e_skip_whitespace(struct RopeCursor *cursor) {
	int rv = 0;
	while (!rope_cursor_is_eof(cursor)) {
		uint64_t c = rope_cursor_cp(cursor);
		if (c == ' ' || c == '\t') {
			rv = rope_cursor_move_by(cursor, ROPE_BYTE, 1);
			if (rv < 0) {
				goto out;
			}
		} else {
			break;
		}
	}
out:
	return rv;
}
