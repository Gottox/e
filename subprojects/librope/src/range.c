#include <assert.h>
#include <rope.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void
dummy_callback(
		struct Rope *rope, struct RopeRange *range, bool damaged,
		void *userdata) {
	(void)rope;
	(void)range;
	(void)damaged;
	(void)userdata;
}

static bool
is_collapsed(struct RopeRange *range) {
	return range->cursor_start.index == range->cursor_end.index;
}

static void
handle_change(struct Rope *rope, struct RopeCursor *cursor, void *userdata) {
	(void)rope;
	(void)cursor;
	struct RopeRange *range = userdata;
	if (cursor == &range->cursor_end &&
		rope_cursor_is_order(cursor, &range->cursor_start)) {

		// Moves the end pointer after the start pointer if they aren't in
		// order. As the cursors are updated back to front.
		rope_char_index_t char_index =
				rope_cursor_char_index(&range->cursor_start);
		rope_cursor_move_to_index(&range->cursor_end, char_index, 0);
		range->callback(rope, range, true, range->callback_userdata);
	} else if (cursor == &range->cursor_start) {
		range->callback(rope, range, false, range->callback_userdata);
	} else if (!is_collapsed(range)) {
		range->callback(rope, range, true, range->callback_userdata);
	}
}

int
rope_range_start_move_to(
		struct RopeRange *range, rope_index_t line, rope_char_index_t column) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;
	int rv = rope_cursor_move_to_line_col(start, line, column);
	if (rv < 0) {
		return rv;
	}
	if (!rope_cursor_is_order(start, end)) {
		rope_char_index_t char_index = rope_cursor_char_index(start);
		rv = rope_cursor_move_to_index(end, char_index, 0);
	}
	return rv;
}

int
rope_range_end_move_to(
		struct RopeRange *range, rope_index_t line, rope_char_index_t column) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;
	int rv = rope_cursor_move_to_line_col(end, line, column);
	if (rv < 0) {
		return rv;
	}
	if (!rope_cursor_is_order(start, end)) {
		rope_char_index_t char_index = rope_cursor_char_index(end);
		rv = rope_cursor_move_to_index(start, char_index, 0);
	}
	return rv;
}

int
rope_range_start_move_to_at(
		struct RopeRange *range, enum RopeUnit unit, size_t index, uint64_t tags) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;
	int rv = rope_cursor_move_to(start, unit, index, tags);
	if (rv < 0) {
		return rv;
	}
	if (!rope_cursor_is_order(start, end)) {
		size_t start_index = rope_cursor_index(start, unit);
		rv = rope_cursor_move_to(end, unit, start_index, 0);
	}
	return rv;
}

int
rope_range_start_move_to_index(
		struct RopeRange *range, rope_char_index_t index, uint64_t tags) {
	return rope_range_start_move_to_at(range, ROPE_CHAR, index, tags);
}

int
rope_range_end_move_to_at(
		struct RopeRange *range, enum RopeUnit unit, size_t index, uint64_t tags) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;
	int rv = rope_cursor_move_to(end, unit, index, tags);
	if (rv < 0) {
		return rv;
	}
	if (!rope_cursor_is_order(start, end)) {
		size_t end_index = rope_cursor_index(end, unit);
		rv = rope_cursor_move_to(start, unit, end_index, 0);
	}
	return rv;
}

int
rope_range_end_move_to_index(
		struct RopeRange *range, rope_char_index_t index, uint64_t tags) {
	return rope_range_end_move_to_at(range, ROPE_CHAR, index, tags);
}

int
rope_range_init(struct RopeRange *range, struct Rope *rope) {
	int rv = 0;
	range->rope = rope;
	range->callback = dummy_callback;
	rv = rope_cursor_init(&range->cursor_start, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_set_callback(&range->cursor_start, handle_change, range);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_init(&range->cursor_end, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_set_callback(&range->cursor_end, handle_change, range);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
rope_range_set_callback(
		struct RopeRange *range, rope_range_callback_t callback,
		void *userdata) {
	range->callback = callback;
	range->callback_userdata = userdata;
	return 0;
}

int
rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size,
		uint64_t tags) {
	int rv = 0;
	rv = rope_range_delete(range);
	if (rv < 0) {
		goto out;
	}

	struct RopeCursor *end = &range->cursor_end;

	rv = rope_cursor_insert(end, data, byte_size, tags);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
rope_range_insert_str(struct RopeRange *range, const char *str, uint64_t tags) {
	size_t byte_size = strlen(str);
	const uint8_t *data = (const uint8_t *)str;

	return rope_range_insert(range, data, byte_size, tags);
}

int
rope_range_delete(struct RopeRange *range) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;

	// Use byte-based deletion to avoid iterating to convert byte range to chars
	size_t byte_count = end->index - start->index;

	return rope_cursor_delete_at(start, ROPE_BYTE, byte_count);
}

int
rope_range_line(struct RopeRange *range, rope_index_t line) {
	struct RopeCursor *start = &range->cursor_start;
	struct RopeCursor *end = &range->cursor_end;

	int rv = rope_cursor_move_to_line_col(start, line, 0);
	if (rv < 0) {
		return rv;
	}

	rv = rope_cursor_move_to_line_col(end, line + 1, 0);
	if (rv < 0) {
		int size = rope_char_size(range->rope);
		if (size < 0) {
			return size;
		}
		rv = rope_cursor_move_to_index(end, (rope_char_index_t)size, 0);
	}

	return rv;
}

char *
rope_range_to_str(struct RopeRange *range, uint64_t tags) {
	struct RopeIterator it = {0};
	const uint8_t *data = NULL;
	size_t size = 0;
	size_t total = 0;
	int rv = rope_iterator_init(&it, range, tags);
	if (rv < 0) {
		return NULL;
	}
	while (rope_iterator_next(&it, &data, &size)) {
		total += size;
	}
	rope_iterator_cleanup(&it);

	char *res = calloc(total + 1, sizeof(char));
	if (res == NULL) {
		return NULL;
	}

	rv = rope_iterator_init(&it, range, tags);
	if (rv < 0) {
		free(res);
		return NULL;
	}
	size_t off = 0;
	while (rope_iterator_next(&it, &data, &size)) {
		memcpy(res + off, data, size);
		off += size;
	}
	rope_iterator_cleanup(&it);

	return res;
}

int
rope_range_cleanup(struct RopeRange *range) {
	if (range == NULL) {
		return 0;
	}
	rope_cursor_cleanup(&range->cursor_start);
	rope_cursor_cleanup(&range->cursor_end);
	return 0;
}
