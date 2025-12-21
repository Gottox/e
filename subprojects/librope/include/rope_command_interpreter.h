/*
 * Minimal command interpreter used by the fuzzer to exercise the rope API.
 */
#ifndef ROPE_COMMAND_INTERPRETER_H
#define ROPE_COMMAND_INTERPRETER_H

#include <rope.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct RopeCommandInterpreter {
	struct Rope rope;
	struct RopeCursor cursor;
	struct RopeRange range;
	bool range_ready;
};

int rope_command_interpreter_init(struct RopeCommandInterpreter *interpreter);

void rope_command_interpreter_run(
		struct RopeCommandInterpreter *interpreter, const uint8_t *data,
		size_t size);

void rope_command_interpreter_cleanup(struct RopeCommandInterpreter *interpreter);

#endif
