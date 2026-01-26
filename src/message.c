#include <assert.h>
#include <e_konstrukt.h>
#include <e_message.h>
#include <e_struktur.h>
#include <e_utils.h>
#include <stdbool.h>
#include <string.h>

int
e_message_init(struct EMessage *message, struct EKonstrukt *k) {
	int rv = 0;

	rv = rope_init(&message->content, &k->rope_pool);
	if (rv < 0) {
		goto out;
	}

	message->sender_id = 0;
out:
	return rv;
}

int
e_message_from_klient(struct EMessage *message, struct EKlient *klient) {
	assert(klient->base.type == &e_struktur_type_klient);
	int rv = 0;
	struct RopeRange range = {0};
	struct RopeCursor message_cursor = {0};

	rv = rope_range_init(&range, &klient->input_buffer);
	if (rv < 0) {
		goto out;
	}

	struct RopeCursor *start = rope_range_start(&range);
	struct RopeCursor *end = rope_range_end(&range);

	rv = rope_cursor_move_to(start, ROPE_BYTE, 0, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_move_to(end, ROPE_LINE, 1, 0);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_init(&message_cursor, &message->content);
	if (rv < 0) {
		goto out;
	}
	rv = rope_range_copy_to(&range, &message_cursor, 0);
	if (rv < 0) {
		goto out;
	}
out:
	rope_cursor_cleanup(&message_cursor);
	rope_range_cleanup(&range);
	return rv;
}

int
e_message_add(struct EMessage *message, struct RopeStr *str) {
	(void)message;
	(void)str;
	return 0;
}

int
e_message_add_data(struct EMessage *message, const uint8_t *data, size_t size) {
	int rv = 0;
	struct RopeStr str = {0};

	rv = rope_str_init(&str, data, size);
	if (rv < 0) {
		goto out;
	}
	rv = e_message_add(message, &str);
out:
	return rv;
}

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

out:
	if (rv < 0) {
		e_message_parse_cleanup(parser);
	}
	return 0;
}

static int
parse_sized_message(struct EMessageParser *parser, struct RopeRange *tgt) {
	int rv = 0;

	struct RopeCursor *line = &parser->line;
	// Skip '@'
	rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
	if (rv < 0) {
		goto out;
	}

	uint64_t byte_size = 0;
	rv = e_parse_unsigned_dec(line, &byte_size);
	if (rv < 0) {
		goto out;
	}

	// On first sized field, advance post.end to after the header newline
	struct RopeCursor *end = rope_range_end(&parser->post);
	if (rope_cursor_index(end, ROPE_BYTE, 0) == 0) {
		rv = rope_cursor_move_to(end, ROPE_LINE, 1, 0);
		if (rv < 0) {
			goto out;
		}
	}

	rope_range_collapse(&parser->post, ROPE_RIGHT);
	rv = rope_cursor_move_by(end, ROPE_BYTE, byte_size);
	if (rv < 0) {
		goto out;
	}

	rope_range_clone(tgt, &parser->post);

	uint32_t cp = rope_cursor_cp(end);
	if (cp != '\n') {
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
		if (cp == '\\') {
			rv = rope_cursor_move_by(line, ROPE_BYTE, 1);
			if (rv < 0) {
				goto out;
			}
			cp = rope_cursor_cp(line);
			if (cp == '\n') {
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
e_message_parse_next(
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
		rv = parse_sized_message(parser, tgt);
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

void
e_message_parse_cleanup(struct EMessageParser *parser) {
	rope_cursor_cleanup(&parser->line);
	rope_range_cleanup(&parser->post);
	rope_cleanup(&parser->unescape_buffer);
}
