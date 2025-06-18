#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_iterator_simple() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);
	rv = rope_append(&r, (uint8_t *)"World", 5);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	rv = rope_range_cleanup(&range);
}

static void
test_iterator_collect() {
        int rv = 0;
        struct Rope r = {0};
        rv = rope_init(&r);
        ASSERT_EQ(0, rv);

        rv = rope_append_str(&r, "Hello ");
        ASSERT_EQ(0, rv);
        rv = rope_append_str(&r, "World");
        ASSERT_EQ(0, rv);

        struct RopeRange range = {0};
        rv = rope_range_init(&range, &r, NULL, NULL, NULL);
        ASSERT_EQ(0, rv);
        rv = rope_cursor_move_to_index(rope_range_end(&range), rope_char_size(&r));
        ASSERT_EQ(0, rv);

        struct RopeIterator iter = {0};
        rv = rope_iterator_init(&iter, &range);
        ASSERT_EQ(0, rv);

        char result[32] = {0};
        size_t offset = 0;
        const uint8_t *value = NULL;
        size_t size = 0;
        while (rope_iterator_next(&iter, &value, &size)) {
                memcpy(&result[offset], value, size);
                offset += size;
        }
        ASSERT_STREQ("Hello World", result);

        rv = rope_iterator_cleanup(&iter);
        ASSERT_EQ(0, rv);
        rv = rope_range_cleanup(&range);
        ASSERT_EQ(0, rv);
        rv = rope_cleanup(&r);
        ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(test_iterator_simple)
TEST(test_iterator_collect)
END_TESTS
