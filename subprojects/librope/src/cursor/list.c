#include "cursor_internal.h"

#include <rope.h>

struct RopeCursor **
cursor_detach(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = &cursor->rope->last_cursor;
	struct RopeCursor *c = *ptr;

	for (; c->index > cursor->index; c = c->prev) {
		ptr = &c->prev;
	}
	struct RopeCursor **insert_ptr = ptr;
	for (; c != cursor; c = c->prev) {
		ptr = &c->prev;
	}
	*ptr = cursor->prev;
	return insert_ptr;
}

void
cursor_attach_at(struct RopeCursor *cursor, struct RopeCursor **ptr) {
	cursor->prev = *ptr;
	*ptr = cursor;
}

void
cursor_attach(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = &cursor->rope->last_cursor;
	struct RopeCursor *c = *ptr;

	for (; c && c->index > cursor->index; c = c->prev) {
		ptr = &c->prev;
	}
	cursor_attach_at(cursor, ptr);
}

void
cursor_bubble_up(struct RopeCursor *cursor) {
	struct RopeCursor **ptr = cursor_detach(cursor);
	cursor_attach_at(cursor, ptr);
}

void
cursor_update(struct RopeCursor *cursor) {
	if (cursor->prev && cursor->prev->index <= cursor->index) {
		cursor_bubble_up(cursor);
	} else {
		cursor_detach(cursor);
		cursor_attach(cursor);
	}
}

void
cursor_damaged(
		struct RopeCursor *cursor, rope_byte_index_t lower_bound,
		off_t byte_offset) {
	struct RopeCursor *last = cursor->rope->last_cursor;
	struct RopeCursor *barrier = cursor->prev;
	struct RopeCursor *c = last;

	for (; c != barrier; c = c->prev) {
		if (byte_offset < 0 && c->index < (size_t)-byte_offset) {
			// underflow
			break;
		}
		c->index += byte_offset;
		if (c->index < lower_bound) {
			break;
		}
	}
	for (; c != barrier; c = c->prev) {
		c->index = lower_bound;
	}
	for (c = last; c != barrier; c = c->prev) {
		c->callback(c->rope, c, c->userdata);
	}
}
