#include <rope.h>
#include <string.h>
#include <testlib.h>

static void noop_range_cb(struct Rope *r, struct RopeRange *range, void *ud);

static void
range_basic() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World This is a string");
	ASSERT_EQ(0, rv);

        struct RopeRange range = {0};
        rv = rope_range_init(&range, &r, noop_range_cb, noop_range_cb, NULL);
	ASSERT_EQ(0, rv);

	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void noop_range_cb(struct Rope *r, struct RopeRange *range, void *ud) {
        (void)r;
        (void)range;
        (void)ud;
}

static void
range_insert_delete() {
        int rv = 0;
        struct Rope r = {0};
        rv = rope_init(&r);
        ASSERT_EQ(0, rv);

        struct RopeRange range = {0};
        rv = rope_range_init(&range, &r, noop_range_cb, noop_range_cb, NULL);
        ASSERT_EQ(0, rv);

        rv = rope_range_insert_str(&range, "Hello");
        ASSERT_EQ(0, rv);

        struct RopeNode *node = rope_first(&r);
        size_t size = 0;
        const uint8_t *data = rope_node_value(node, &size);
        ASSERT_EQ(0, memcmp(data, "Hello", size));

        rv = rope_cursor_move_to_index(rope_range_end(&range), rope_char_size(&r));
        ASSERT_EQ(0, rv);
        rv = rope_cursor_move_to_index(rope_range_start(&range), 0);
        ASSERT_EQ(0, rv);
        rv = rope_range_delete(&range);
        ASSERT_EQ(0, rv);

        node = rope_first(&r);
        data = rope_node_value(node, &size);
        ASSERT_EQ((size_t)0, size);

        rv = rope_range_cleanup(&range);
        ASSERT_EQ(0, rv);
        rv = rope_cleanup(&r);
        ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(range_basic)
TEST(range_insert_delete)
END_TESTS
