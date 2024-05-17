#include <rope.h>
#include <stdbool.h>

static bool
is_collapsed(struct RopeRange *range) {
	return range->left->index == range->right->index;
}

static void
handle_change(struct Rope *rope, struct RopeCursor *cursor, void *userdata) {
	(void)rope;
	(void)cursor;
	struct RopeRange *range = userdata;
	if (cursor == range->left) {
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
	range->rope = rope;
	range->offset_change_callback = offset_change_callback;
	range->damage_callback = damage_callback;
	range->userdata = userdata;
	rope_cursor_init(&range->cursors[0], rope);
	rope_cursor_set_callback(&range->cursors[0], handle_change, range);
	rope_cursor_init(&range->cursors[1], rope);
	rope_cursor_set_callback(&range->cursors[1], handle_change, range);
	range->left = &range->cursors[0];
	range->right = &range->cursors[1];

	return 0;
}

static int
check_order(struct RopeRange *range) {
	rope_char_index_t left_index = range->left->index;
	rope_char_index_t right_index = range->right->index;
	if (left_index > right_index) {
		struct RopeCursor *tmp = range->left;
		range->left = range->right;
		range->right = tmp;
	} else if (is_collapsed(range)) {
		// A neutral move will reattach the cursor and make sure that left is
		// ordered before right.
		rope_cursor_move(range->left, 0);
	}
	return 0;
}

struct RopeCursor *
rope_range_left(struct RopeRange *range) {
	check_order(range);
	return range->left;
}

struct RopeCursor *
rope_range_right(struct RopeRange *range) {
	check_order(range);
	return range->right;
}

int
rope_range_insert(
		struct RopeRange *range, const uint8_t *data, size_t byte_size) {
	int rv = 0;
	rv = rope_range_delete(range);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_insert(range->left, data, byte_size);
out:
	return rv;
}

int
rope_range_insert_str(struct RopeRange *range, const char *str) {
	int rv = 0;
	rv = rope_range_delete(range);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_insert_str(range->left, str);

out:
	return rv;
}

int
rope_range_delete(struct RopeRange *range) {
	check_order(range);

	size_t size = range->right->index - range->left->index;

	return rope_cursor_delete(range->left, size);
}

int
rope_range_cleanup(struct RopeRange *range) {
	rope_cursor_cleanup(&range->cursors[0]);
	rope_cursor_cleanup(&range->cursors[1]);
	return 0;
}
