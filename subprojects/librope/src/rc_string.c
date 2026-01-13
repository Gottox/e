#include <rope.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct RopeRcString *
rope_rc_string_allocate(size_t size, uint8_t **data_ptr) {
	struct RopeRcString *rc_str;
	size_t alloc_size;

	if (ROPE_OVERFLOW_ADD(sizeof(*rc_str), size, &alloc_size)) {
		return NULL;
	}

	rc_str = calloc(alloc_size, sizeof(uint8_t));
	if (!rc_str) {
		return NULL;
	}
	*data_ptr = rc_str->data;
	cx_rc_init(&rc_str->rc);
	rc_str->size = size;

	return rc_str;
}

struct RopeRcString *
rope_rc_string_new(
		const uint8_t *data, size_t size) {
	uint8_t *data_ptr;
	struct RopeRcString *rc_str = rope_rc_string_allocate(size, &data_ptr);

	memcpy(data_ptr, data, size);
	return rc_str;
}

const uint8_t *
rope_rc_string(struct RopeRcString *rc_str, size_t *size) {
	*size = rc_str->size;
	return rc_str->data;
}

struct RopeRcString *
rope_rc_string_retain(struct RopeRcString *rc_str) {
	cx_rc_retain(&rc_str->rc);
	return rc_str;
}

void
rope_rc_string_release(struct RopeRcString *rc_str) {
	if (rc_str && cx_rc_release(&rc_str->rc)) {
		free(rc_str);
	}
}
