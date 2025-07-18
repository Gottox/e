#include <rope.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool
is_collapsed(struct RopeRange *range) {
	return range->cursors[0].index == range->cursors[1].index;
}

static void
handle_change(struct Rope *rope, struct RopeCursor *cursor, void *userdata) {
	(void)rope;
	(void)cursor;
	struct RopeRange *range = userdata;
	if (cursor == rope_range_start(range)) {
		range->offset_change_callback(rope, range, range->userdata);
	} else if (!is_collapsed(range)) {
		range->damage_callback(rope, range, range->userdata);
	}
}

int
rope_range_init(
		struct RopeRange *range, struct Rope *rope,
		rope_range_callback_t offset_change_callback,
		rope_range_callback_t damage_callback, void *userdata) {
	int rv = 0;
	range->rope = rope;
	range->offset_change_callback = offset_change_callback;
	range->damage_callback = damage_callback;
	range->userdata = userdata;
	rv = rope_cursor_init(&range->cursors[0], rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_set_callback(&range->cursors[0], handle_change, range);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_init(&range->cursors[1], rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_set_callback(&range->cursors[1], handle_change, range);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

struct RopeCursor *
rope_range_start(struct RopeRange *range) {
	if (rope_cursor_is_order(&range->cursors[0], &range->cursors[1])) {
		return &range->cursors[0];
	} else {
		return &range->cursors[1];
	}
}

struct RopeCursor *
rope_range_end(struct RopeRange *range) {
	if (rope_cursor_is_order(&range->cursors[0], &range->cursors[1])) {
		return &range->cursors[1];
	} else {
		return &range->cursors[0];
	}
}

int
rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	rv = rope_range_delete(range);
	if (rv < 0) {
		goto out;
	}

	struct RopeCursor *start = rope_range_start(range);
	struct RopeCursor *end = rope_range_end(range);
	rope_char_index_t start_index = start->index;

	rv = rope_cursor_insert(end, data, byte_size);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_move_to_index(start, start_index);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
rope_range_insert_str(struct RopeRange *range, const char *str) {
	size_t byte_size = strlen(str);
	const uint8_t *data = (const uint8_t *)str;

	return rope_range_insert(range, data, byte_size);
}

int
rope_range_delete(struct RopeRange *range) {
	struct RopeCursor *start = rope_range_start(range);
	struct RopeCursor *end = rope_range_end(range);

	size_t size = end->index - start->index;

	return rope_cursor_delete(start, size);
}

char *
rope_range_to_str(struct RopeRange *range) {
	struct RopeIterator it = {0};
	const uint8_t *data = NULL;
	size_t size = 0;
	size_t total = 0;
	int rv = rope_iterator_init(&it, range);
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

	rv = rope_iterator_init(&it, range);
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
	rope_cursor_cleanup(&range->cursors[0]);
	rope_cursor_cleanup(&range->cursors[1]);
	return 0;
}
