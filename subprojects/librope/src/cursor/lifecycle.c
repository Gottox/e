#include "cursor_internal.h"

#include <rope.h>
#include <string.h>

static void
dummy_callback(struct Rope *rope, struct RopeCursor *cursor, void *user) {
	(void)rope;
	(void)cursor;
	(void)user;
}

int
rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope) {
	cursor->callback = dummy_callback;
	cursor->userdata = NULL;
	cursor->rope = rope;
	cursor->byte_index = 0;
	cursor->prev = NULL;
	cursor_attach(cursor);
	return 0;
}

int
rope_cursor_clone(struct RopeCursor *cursor, struct RopeCursor *from) {
	int rv = rope_cursor_init(cursor, from->rope);
	if (rv < 0) {
		return rv;
	}
	size_t byte_index = rope_cursor_index(from, ROPE_BYTE, 0);
	//rope_cursor_set_callback(cursor, from->callback, from->userdata);

	return rope_cursor_move_to(cursor, ROPE_BYTE, byte_index, 0);
}

void
rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata) {
	if (callback == NULL) {
		callback = dummy_callback;
	}
	cursor->callback = callback;
	cursor->userdata = userdata;
}

void
rope_cursor_cleanup(struct RopeCursor *cursor) {
	if (cursor->rope == NULL) {
		return;
	}
	cursor_detach(cursor);
	memset(cursor, 0, sizeof(*cursor));
}
