#include <rope.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
iterator_full() {
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
	rv = rope_cursor_move_to_index(&range.cursors[1], rope_char_size(&r));
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	char buf[32];
	size_t off = 0;
	while (rope_iterator_next(&it, &data, &size)) {
		memcpy(buf + off, data, size);
		off += size;
	}
	buf[off] = '\0';
	ASSERT_STREQ(buf, "HelloWorld");

	rv = rope_iterator_cleanup(&it);
	ASSERT_EQ(0, rv);
	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
iterator_partial() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);
	rv = rope_append(&r, (uint8_t *)"World", 5);
	ASSERT_EQ(0, rv);
	rv = rope_append(&r, (uint8_t *)"Test", 4);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&range.cursors[0], 2);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&range.cursors[1], 8);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	char buf[32];
	size_t off = 0;
	while (rope_iterator_next(&it, &data, &size)) {
		memcpy(buf + off, data, size);
		off += size;
	}
	buf[off] = '\0';
	ASSERT_STREQ(buf, "lloWor");

	char *str = rope_range_to_str(&range);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(str, "lloWor");
	free(str);

	rv = rope_iterator_cleanup(&it);
	ASSERT_EQ(0, rv);
	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
iterator_empty() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);
	rv = rope_append_str(&r, "Hello");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	bool has = rope_iterator_next(&it, &data, &size);
	ASSERT_FALSE(has);

	char *str = rope_range_to_str(&range);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(str, "");
	free(str);

	rv = rope_iterator_cleanup(&it);
	ASSERT_EQ(0, rv);
	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
iterator_big_non_inline() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	const char *part1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	const char *part2 = "abcdefghijklmnopqrstuvwxyz0987654321";

	rv = rope_append_str(&r, part1);
	ASSERT_EQ(0, rv);
	rv = rope_append_str(&r, part2);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&range.cursors[1], rope_char_size(&r));
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	char buf[128];
	size_t off = 0;
	while (rope_iterator_next(&it, &data, &size)) {
		memcpy(buf + off, data, size);
		off += size;
	}
	buf[off] = '\0';
	ASSERT_STREQ(
			buf,
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz0987"
			"654321");

	rv = rope_iterator_cleanup(&it);
	ASSERT_EQ(0, rv);
	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
iterator_multibyte() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"abc🙂def");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&range.cursors[0], 3);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&range.cursors[1], 4);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	bool has = rope_iterator_next(&it, &data, &size);
	ASSERT_TRUE(has);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(data, u8"🙂", 4));
	has = rope_iterator_next(&it, &data, &size);
	ASSERT_FALSE(has);

	char *str = rope_range_to_str(&range);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(str, u8"🙂");
	free(str);

	rv = rope_iterator_cleanup(&it);
	ASSERT_EQ(0, rv);
	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(iterator_full)
TEST(iterator_partial)
TEST(iterator_empty)
TEST(iterator_big_non_inline)
TEST(iterator_multibyte)
END_TESTS
