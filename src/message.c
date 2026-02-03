#include <assert.h>
#include <ctype.h>
#include <e_konstrukt.h>
#include <e_message.h>
#include <e_struktur.h>
#include <e_utils.h>
#include <stdbool.h>
#include <string.h>

int
e_message_parser_init(struct EMessageParser *parser, struct Rope *message) {
	int rv = 0;

	rv = rope_cursor_init(&parser->line, message);
	if (rv < 0) {
		goto out;
	}
	rv = rope_range_init(&parser->post, message);
	if (rv < 0) {
		goto out;
	}
	rv = rope_init(&parser->unescape_buffer, message->pool);

	struct RopeCursor *end = rope_range_end(&parser->post);

	rv = rope_cursor_move_to(end, ROPE_LINE, 1, 0);
	if (rv == -ROPE_ERROR_OOB) {
		// Ignore this error and move the cursor to the end.
		size_t size = rope_size(message, ROPE_BYTE);
		rv = rope_cursor_move_to(end, ROPE_BYTE, size, 0);
	}
	if (rv < 0) {
		goto out;
	}
	rope_range_collapse(&parser->post, ROPE_RIGHT);

out:
	if (rv < 0) {
		e_message_parser_cleanup(parser);
	}
	return rv;
}

static int
parse_terminator_message(struct EMessageParser *parser, struct RopeRange *tgt) {
	int rv = 0;
	size_t stopword_size;
	uint8_t stopword[256] = "@";
	struct RopeCursor *line = &parser->line;

	for (stopword_size = 1;; stopword_size++) {
		uint_least32_t cp = rope_cursor_cp(line);
		if (cp == ' ' || cp == '\n') {
			break;
		}
		if (stopword_size >= sizeof(stopword) - 2) {
			// TODO: better error code.
			rv = -1;
			goto out;
		}
		stopword[stopword_size] = (char)cp;
		rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
		if (rv < 0) {
			goto out;
		}
	}
	stopword[stopword_size] = '\n';
	stopword_size++;
	// We know the stopword search for it.
	
	struct RopeCursor *end = rope_range_end(&parser->post);
	for (;;) {
		if (rope_cursor_starts_with_data(end, stopword, stopword_size)) {
			break;
		}
		rv = rope_cursor_move_by(end, ROPE_LINE, 1);
		if (rv < 0) {
			goto out;
		}
	}
	rope_range_clone(tgt, &parser->post);

	rv = rope_cursor_move_by(end, ROPE_BYTE, stopword_size);


out:
	return rv;
}

static int
parse_sized_message(struct EMessageParser *parser, struct RopeRange *tgt) {
	int rv = 0;
	uint64_t byte_size = 0;
	struct RopeCursor *line = &parser->line;
	rv = e_parse_unsigned_dec(line, &byte_size);
	if (rv < 0) {
		goto out;
	}

	// On first sized field, advance post.end to after the header newline
	struct RopeCursor *end = rope_range_end(&parser->post);

	rope_range_collapse(&parser->post, ROPE_RIGHT);
	rv = rope_cursor_move_by(end, ROPE_BYTE, byte_size);
	if (rv < 0) {
		goto out;
	}

	rope_range_clone(tgt, &parser->post);

	uint32_t cp = rope_cursor_cp(end);
	if (cp != '\n') {
		// TODO: better error code.
		rv = -1;
		goto out;
	}

	rv = rope_cursor_move_by(end, ROPE_BYTE, 1);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

static int
parse_at_message(struct EMessageParser *parser, struct RopeRange *tgt) {
	int rv = 0;

	struct RopeCursor *line = &parser->line;
	// Skip '@'
	rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
	if (rv < 0) {
		return rv;
	}

	uint_least32_t cp = rope_cursor_cp(line);
	if (isalpha(cp)) {
		return parse_terminator_message(parser, tgt);
	} else if (isdigit(cp)) {
		return parse_sized_message(parser, tgt);
	} else {
		// TODO better error code
		return -1;
	}
}

static int
parse_quoted_message(struct EMessageParser *parser, struct RopeRange *tgt) {
	int rv = 0;
	char terminator[] = " \n";
	struct RopeCursor *line = &parser->line;

	uint32_t cp = rope_cursor_cp(line);

	rope_clear(&parser->unescape_buffer);

	struct RopeCursor output = {0};
	rv = rope_cursor_init(&output, &parser->unescape_buffer);

	if (cp == '"' || cp == '\'') {
		rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
		if (rv < 0) {
			goto out;
		}
		terminator[0] = cp;
		terminator[1] = '\0';
		cp = rope_cursor_cp(line);
	}

	for (;;) {
		if (rope_cursor_is_eof(line) && terminator[0] != ' ') {
			// TODO: better error code.
			rv = -1;
			goto out;
		}
		if (cp == '\\') {
			rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
			if (rv < 0) {
				goto out;
			}
			cp = rope_cursor_cp(line);
			if (cp == '\n') {
				// TODO: better error code.
				rv = -1;
				goto out;
			}
			switch (cp) {
			case 'n':
				cp = '\n';
				break;
			case 'r':
				cp = '\r';
				break;
			case 't':
				cp = '\t';
				break;
			// TODO: support hex and unicode escapes
			default:
				break;
			}
		} else if (strchr(terminator, cp)) {
			break;
		} else if (cp == '\n') {
			// TODO: better error code.
			rv = -1;
			goto out;
		}
		rv = rope_cursor_insert_cp(&output, cp, 0);
		if (rv < 0) {
			goto out;
		}
		rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
		if (rv < 0) {
			goto out;
		}
		cp = rope_cursor_cp(line);
	}
	if (terminator[0] != ' ') {
		rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
		if (rv < 0) {
			goto out;
		}
	}

	rv = rope_to_range(&parser->unescape_buffer, tgt);
	if (rv < 0) {
		goto out;
	}

out:
	rope_cursor_cleanup(&output);
	return rv;
}

bool
e_message_parser_next(
		struct EMessageParser *parser, struct RopeRange *tgt, int *err) {
	bool has_next = false;
	int rv = 0;

	rope_range_cleanup(tgt);
	struct RopeCursor *line = &parser->line;

	rv = e_skip_whitespace(line);
	if (rv < 0) {
		goto out;
	}

	uint32_t cp = rope_cursor_cp(line);
	has_next = true;
	switch (cp) {
	case '\n':
		has_next = false;
		break;
	case '@':
		rv = parse_at_message(parser, tgt);
		break;
	default:
		rv = parse_quoted_message(parser, tgt);
		break;
	}

out:
	if (err) {
		*err = rv;
	}
	return has_next;
}

int
e_message_parse_consume(struct EMessageParser *parser) {
	int rv = 0;
	struct RopeCursor *start = rope_range_start(&parser->post);

	rv = rope_cursor_move_to(start, ROPE_BYTE, 0, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_range_delete(&parser->post);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

void
e_message_parser_cleanup(struct EMessageParser *parser) {
	rope_cursor_cleanup(&parser->line);
	rope_range_cleanup(&parser->post);
	rope_cleanup(&parser->unescape_buffer);
}
