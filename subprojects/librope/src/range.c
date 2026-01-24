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
	return range->cursor_start.byte_index == range->cursor_end.byte_index;
}

static void
range_ensure_order(struct RopeRange *range) {
	struct RopeCursor *start = rope_range_start(range);
	struct RopeCursor *end = rope_range_end(range);

	if (!rope_cursor_is_order(start, end)) {
		size_t start_index = rope_cursor_index(start, ROPE_BYTE, 0);
		rope_cursor_move_to(end, ROPE_BYTE, start_index, 0);
	}
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
		size_t byte_index =
				rope_cursor_index(&range->cursor_start, ROPE_BYTE, 0);
		rope_cursor_move_to(&range->cursor_end, ROPE_BYTE, byte_index, 0);
		range->callback(rope, range, true, range->callback_userdata);
	} else if (cursor == &range->cursor_start) {
		range->callback(rope, range, false, range->callback_userdata);
	} else if (!is_collapsed(range)) {
		range->callback(rope, range, true, range->callback_userdata);
	}
}

struct RopeCursor *
rope_range_start(struct RopeRange *range) {
	return &range->cursor_start;
}

struct RopeCursor *
rope_range_end(struct RopeRange *range) {
	return &range->cursor_end;
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
	struct RopeCursor *end = rope_range_end(range);
	rv = rope_range_delete(range);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_insert_data(end, data, byte_size, tags);
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
	struct RopeCursor *start = rope_range_start(range);
	struct RopeCursor *end = rope_range_end(range);

	range_ensure_order(range);

	size_t byte_count = end->byte_index - start->byte_index;

	int rv = rope_cursor_delete(start, ROPE_BYTE, byte_count);
	if (rv < 0) {
		return rv;
	}

	return 0;
}

int
rope_range_to_str(struct RopeRange *range, struct RopeStr *str, uint64_t tags) {
	struct RopeIterator it = {0};
	size_t total = 0;
	int rv = rope_iterator_init(&it, range, tags);
	if (rv < 0) {
		goto out;
	}
	struct RopeStr it_str = {0};
	while (rope_iterator_next(&it, &it_str)) {
		total += rope_str_size(&it_str, ROPE_BYTE);
	}
	rope_iterator_cleanup(&it);

	uint8_t *res = NULL;
	rv = rope_str_alloc(str, total, &res);
	if (res == NULL) {
		goto out;
	}

	rv = rope_iterator_init(&it, range, tags);
	if (rv < 0) {
		goto out;
	}
	size_t off = 0;
	while (rope_iterator_next(&it, &it_str)) {
		size_t size = 0;
		const uint8_t *data = rope_str_data(&it_str, &size);
		memcpy(res + off, data, size);
		off += size;
	}
	rope_iterator_cleanup(&it);

	str = NULL;
out:
	rope_str_cleanup(str);
	return rv;
}

char *
rope_range_to_cstr(struct RopeRange *range, uint64_t tags) {
	struct RopeStr str = {0};
	char *res = NULL;
	int rv = rope_range_to_str(range, &str, tags);
	if (rv < 0) {
		goto out;
	}
	size_t size = 0;
	const uint8_t *data = rope_str_data(&str, &size);
	res = malloc(size + 1);
	if (res == NULL) {
		goto out;
	}
	memcpy(res, data, size);
	res[size] = '\0';
out:
	rope_str_cleanup(&str);
	return res;
}

void
rope_range_cleanup(struct RopeRange *range) {
	if (range == NULL) {
		return;
	}
	rope_cursor_cleanup(&range->cursor_start);
	rope_cursor_cleanup(&range->cursor_end);
}
