#include <rope.h>
#include <wctype.h>

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define SCALE_FACTOR 1
#define CHARS_PER_OUTPUT (2 * SCALE_FACTOR)
#define BRAILLE_BASE 0x2800

static const uint8_t bit_table[] = {
		0, 1 << 3, 1 << 1, 1 << 4, 1 << 2, 1 << 5, 1 << 6, 1 << 7,
};

int
expose(struct Rope *input, struct Rope *output, int columns, int tab_width) {
	(void)columns;
	(void)tab_width;
	(void)bit_table; // Suppress unused variable warnings
	int rv = 0;
	struct RopeCursor input_cursor = {0};
	struct RopeCursor output_cursor = {0};

	rv = rope_cursor_init(&input_cursor, input);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_init(&output_cursor, output);
	if (rv < 0) {
		goto out;
	}

	size_t char_size = rope_char_size(input);
	rope_index_t input_line = 0;
	rope_index_t input_column = 0;
	rope_index_t output_line = 0;
	rope_index_t output_column = 0;
	for (size_t i = 0; i < char_size; i++) {
		int32_t cp = rope_cursor_codepoint(&input_cursor);
		if (cp == '\n') {
			input_line++;
			input_column = 0;
			output_column = 0;
			continue;
		} else {
			input_column++;
			output_column++;
		}
		rope_cursor_move_to(&output_cursor, output_line, output_column);
		rope_cursor_move_to(&input_cursor, input_line, input_column);

		if (input_cursor.column % 2 == 0) {
			rope_cursor_move(&input_cursor, 1);
		}
	}

out:
	return 0;
}
