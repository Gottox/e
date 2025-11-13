#include <rope.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct RopeRcString *
rope_rc_string_new2(const uint8_t *data1, size_t size1, const uint8_t *data2, size_t size2) {
	struct RopeRcString *rc_str;
	size_t alloc_size;

	if (ROPE_OVERFLOW_ADD(sizeof(*rc_str), size1, &alloc_size)) {
		return NULL;
	}
	if (ROPE_OVERFLOW_ADD(alloc_size, size2, &alloc_size)) {
		return NULL;
	}

	rc_str = calloc(alloc_size, sizeof(uint8_t));
	if (!rc_str) {
		return NULL;
	}
	cx_rc_init(&rc_str->rc);

	if (data1) {
		memcpy(rc_str->data, data1, size1);
	}
	if (data2) {
		memcpy(&rc_str->data[size1], data2, size2);
	}
	rc_str->size = size1 + size2;
	return rc_str;
}

struct RopeRcString *
rope_rc_string_new(const uint8_t *data, size_t size) {
	return rope_rc_string_new2(data, size, NULL, 0);
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
