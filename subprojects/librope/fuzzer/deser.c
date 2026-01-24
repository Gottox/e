#include "rope_deser.h"
#include <ctype.h>
#include <inttypes.h>
#include <rope.h>
#include <stdio.h>
#include <string.h>

#define ROPE_DESER_TYPE_SIZE 1
#define ROPE_DESER_RANGE_COUNT 3
#define TAG_MASK (UINT64_MAX >> 1)

static int
deser_read_command(
		struct RopeDeserCommand *command, const uint8_t **data,
		size_t *length) {
	int rv = 0;
	memset(command, 0, sizeof(*command));
	if (*length < sizeof(command->type) + sizeof(command->range_index)) {
		rv = -1;
		goto out;
	}
	enum RopeDeserType type = 0;
	memcpy(&type, *data, ROPE_DESER_TYPE_SIZE);
	type = type % ROPE_DESER_END;
	*data += ROPE_DESER_TYPE_SIZE;
	*length -= ROPE_DESER_TYPE_SIZE;

	memcpy(&command->range_index, *data, sizeof(command->range_index));
	command->range_index = command->range_index % ROPE_DESER_RANGE_COUNT;
	*data += sizeof(command->range_index);
	*length -= sizeof(command->range_index);

	size_t command_args_size = 0;
	switch (type) {
	case ROPE_DESER_INSERT:
		command_args_size = sizeof(command->args.insert);
		break;
	case ROPE_DESER_DELETE:
		// command_args_size = sizeof(command->args.delete);
		break;
	case ROPE_DESER_RANGE_START:
	case ROPE_DESER_RANGE_END:
		command_args_size = sizeof(command->args.span_range);
		break;
	default:
		__builtin_unreachable();
	}

	if (*length < command_args_size) {
		rv = -1;
		goto out;
	}
	command->type = type;
	memcpy(&command->args, *data, command_args_size);
	*data += command_args_size;
	*length -= command_args_size;

	rv = *length > 0;
out:
	return rv;
}

static void
print_string(const uint8_t *data, size_t length) {
	fputs("(const uint8_t *)\"", stderr);
	for (size_t i = 0; i < length; i++) {
		if (!isprint(data[i])) {
			fprintf(stderr, "\\x%02x", data[i]);
		} else if (data[i] == '"') {
			fputs("\\\"", stderr);
		} else {
			fputc(data[i], stderr);
		}
	}
	fputc('"', stderr);
}

static void
print_rv_check(int rv, bool print) {
	if (print) {
		fprintf(stderr, "ASSERT_EQ(%u, rv);\n", rv);
	}
}

int
rope_deserialize(const uint8_t *data, size_t length, bool print) {
	int rv = 0;
	uint8_t *payload;
	uint8_t tmp = 0;
	struct RopeDeserCommand command = {0};
	struct RopeRange ranges[ROPE_DESER_RANGE_COUNT] = {0};
	struct RopePool pool = {0};
	struct Rope rope = {0};

	rv = rope_pool_init(&pool);
	if (rv < 0) {
		return rv;
	}

	rv = rope_init(&rope, &pool);
	if (rv < 0) {
		return rv;
	}

	for (int i = 0; i < ROPE_DESER_RANGE_COUNT; i++) {
		rv = rope_range_init(&ranges[i], &rope);
		if (rv < 0) {
			goto out;
		}
	}

	while ((rv = deser_read_command(&command, &data, &length)) > 0) {
		struct RopeRange *range = &ranges[command.range_index];

		// fprintf(stderr, "Processing command on range %u: ",
		// command.range_index);
		switch (command.type) {
		case ROPE_DESER_INSERT:
			tmp = command.args.insert.length;
			if (length < command.args.insert.length) {
				rv = -1;
				goto out;
			}
			payload = cx_memdup(data, command.args.insert.length);
			if (!payload) {
				rv = -1;
				goto out;
			}
			data += command.args.insert.length;
			length -= command.args.insert.length;

			// TODO: make sure the payload is capped to utf-8 single byte
			// sequences only. Later we can expand to arbitrary binary data.
			for (size_t i = 0; i < command.args.insert.length; i++) {
				// payload[i] = payload[i] % 0x80;
				payload[i] = 'a';
			}

			if (print) {
				fprintf(stderr, "// Inserting %u bytes into range %u\n",
						command.args.insert.length, command.range_index);
				fprintf(stderr, "rv = rope_range_insert(&ranges[%u], ",
						command.range_index);
				print_string(payload, command.args.insert.length);
				fprintf(stderr, ", %u, 0x%" PRIx64 ");\n",
						command.args.insert.length,
						command.args.insert.tags & TAG_MASK);
			}

			rv = rope_range_insert(
					range, payload, command.args.insert.length,
					command.args.insert.tags & TAG_MASK);

			print_rv_check(rv, print);

			free(payload);
		case ROPE_DESER_DELETE:
			// fprintf(stderr, "rv = rope_range_delete(&range[%i]\n");
			// rv = rope_range_delete(range);
			break;
		case ROPE_DESER_RANGE_START:
			if (print) {
				fprintf(stderr, "// Moving range %u start to index %i\n",
						command.range_index, tmp);
				fprintf(stderr,
						"rv = rope_range_start_move_to_index(&ranges[%u], "
						"%u);\n",
						command.range_index, command.args.span_range.position);
			}

			tmp = command.args.span_range.position;
			rv = rope_cursor_move_to(
					rope_range_start(range), ROPE_CHAR,
					command.args.span_range.position,
					command.args.span_range.tags & TAG_MASK);

			print_rv_check(rv, print);
			break;
		case ROPE_DESER_RANGE_END:
			tmp = command.args.span_range.position;
			if (print) {
				fprintf(stderr, "// Moving range %u end to index %i\n",
						command.range_index, tmp);
				fprintf(stderr,
						"rv = rope_range_end_move_to_index(&ranges[%u], %u, "
						"0x%" PRIx64 ");\n",
						command.range_index, command.args.span_range.position,
						command.args.span_range.tags & TAG_MASK);
			}
			rv = rope_cursor_move_to(
					rope_range_end(range), ROPE_CHAR,
					command.args.span_range.position,
					command.args.span_range.tags & TAG_MASK);
			print_rv_check(rv, print);
			break;
		default:
			__builtin_unreachable();
		}

		if (rv < 0) {
			goto out;
		}
	}

out:
	for (int i = 0; i < ROPE_DESER_RANGE_COUNT; i++) {
		rope_range_cleanup(&ranges[i]);
	}
	if (rv < 0) {
		rope_cleanup(&rope);
	}
	rope_pool_cleanup(&pool);
	return rv;
}
