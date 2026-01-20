#ifndef ROPE_UTIL_H
#define ROPE_UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct RopeUtf8Counter {
	size_t buffer_size;
	char buffer[8];
	uint_least16_t state;
};

size_t rope_utf8_char_break(struct RopeUtf8Counter *counter, const uint8_t *data, size_t size);

#endif /* ROPE_UTIL_H */
