#ifndef E_MESSAGE_H
#define E_MESSAGE_H

#include "e_common.h"

struct EMessage {
	uint64_t sender_id;
	struct Rope content;
	struct RopeCursor message_cursor;
	struct RopeCursor field_cursor;
};

struct EMessageParser {
	struct Rope unescape_buffer;
	struct Rope *message;
	struct RopeCursor line;
	struct RopeRange post;
};

int e_message_parser_init(struct EMessageParser *iter, struct Rope *message);

bool
e_message_parse_next(
		struct EMessageParser *iter, struct RopeRange *tgt, int *err);


void e_message_parse_cleanup(struct EMessageParser *iter);

#endif /* E_MESSAGE_H */
