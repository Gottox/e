#include <rope.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct RopeRcString *
rope_rc_string_new(const uint8_t *data, size_t size) {
	struct RopeRcString *rc_str;
	size_t alloc_size;

	if (ROPE_OVERFLOW_ADD(sizeof(*rc_str), size, &alloc_size)) {
		return NULL;
	}

	rc_str = calloc(alloc_size, sizeof(uint8_t *));
	if (!rc_str) {
		return NULL;
	}

	memcpy(rc_str->data, data, size);
	rc_str->size = size;
	return rc_str;
}

const uint8_t *
rope_rc_string(struct RopeRcString *rc_str, size_t *size) {
	*size = rc_str->size;
	return rc_str->data;
}

struct RopeRcString *
rope_rc_string_retain(struct RopeRcString *rc_str) {
	rc_str->ref_count++;
	return rc_str;
}

void
rope_rc_string_release(struct RopeRcString *rc_str) {
	if (rc_str && rc_str->ref_count) {
		rc_str->ref_count--;
	} else {
		free(rc_str);
	}
}
