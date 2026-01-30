#ifndef E_MESSAGE_H
#define E_MESSAGE_H

#include "e_common.h"

struct EMessageParser {
	struct Rope unescape_buffer;
	struct Rope *message;
	struct RopeCursor line;
	struct RopeRange post;
};

int e_message_parser_init(struct EMessageParser *iter, struct Rope *message);

bool e_message_parser_next(
		struct EMessageParser *iter, struct RopeRange *tgt, int *err);

int e_message_parse_consume(struct EMessageParser *parser);

void e_message_parser_cleanup(struct EMessageParser *iter);

#endif /* E_MESSAGE_H */
