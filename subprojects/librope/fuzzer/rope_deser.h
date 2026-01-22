#ifndef ROPE_DESER_H
#define ROPE_DESER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @file rope_deser.h
 * @brief Header file for rope deserialization functions.
 */

struct Rope;

enum RopeDeserType {
	ROPE_DESER_RANGE_END,
	ROPE_DESER_RANGE_START,
	ROPE_DESER_INSERT,
	ROPE_DESER_DELETE,
	ROPE_DESER_END,
};

struct RopeDeserCommand {
	enum RopeDeserType type;
	uint8_t range_index;
	union {
		struct {
			uint8_t position;
			uint64_t tags;
		} span_range;
		struct {
			uint8_t length;
			uint64_t tags;
		} insert;
	} args;
};

int rope_deserialize(const uint8_t *data, size_t length, bool print);

#endif
