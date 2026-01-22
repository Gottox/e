#include "cursor_internal.h"

#include <rope.h>

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
	cursor->index = 0;
	cursor->prev = NULL;
	cursor_attach(cursor);
	return 0;
}

int
rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata) {
	if (callback == NULL) {
		callback = dummy_callback;
	}
	cursor->callback = callback;
	cursor->userdata = userdata;
	return 0;
}

void
rope_cursor_cleanup(struct RopeCursor *cursor) {
	if (cursor->rope == NULL) {
		return;
	}
	cursor_detach(cursor);
	cursor->prev = NULL;
}
