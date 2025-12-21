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
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_range_end_move_to_index(&range, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range, 0);
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
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_range_start_move_to_index(&range, 2, 0);
	ASSERT_EQ(0, rv);
	rv = rope_range_end_move_to_index(&range, 8, 0);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range, 0);
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

	char *str = rope_range_to_str(&range, 0);
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
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range, 0);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	bool has = rope_iterator_next(&it, &data, &size);
	ASSERT_FALSE(has);

	char *str = rope_range_to_str(&range, 0);
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
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_range_end_move_to_index(&range, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range, 0);
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
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_range_start_move_to_index(&range, 3, 0);
	ASSERT_EQ(0, rv);
	rv = rope_range_end_move_to_index(&range, 4, 0);
	ASSERT_EQ(0, rv);

	struct RopeIterator it = {0};
	rv = rope_iterator_init(&it, &range, 0);
	ASSERT_EQ(0, rv);

	const uint8_t *data = NULL;
	size_t size = 0;
	bool has = rope_iterator_next(&it, &data, &size);
	ASSERT_TRUE(has);
	ASSERT_EQ(4, size);
	ASSERT_EQ(0, memcmp(data, u8"🙂", 4));
	has = rope_iterator_next(&it, &data, &size);
	ASSERT_FALSE(has);

	char *str = rope_range_to_str(&range, 0);
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

static void
iterator_tagged_filter() {
	const uint64_t TAG_RED = 1ULL << 0;
	const uint64_t TAG_BLUE = 1ULL << 1;

	int rv = 0;
	struct Rope r = {0};
	bool has_next;
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);
	struct RopeCursor cursor = {0};
	const uint8_t *data = NULL;
	size_t size = 0;

	rv = rope_cursor_init(&cursor, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&cursor, "red", TAG_RED);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&cursor, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_insert_str(&cursor, "blue", TAG_BLUE);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&cursor, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_insert_str(&cursor, "magenta", TAG_RED | TAG_BLUE);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&cursor, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_insert_str(&cursor, "plain", 0);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_range_start_move_to_index((&range), 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_range_end_move_to_index(
			(&range), rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);

	struct RopeIterator all_it = {0};
	rv = rope_iterator_init(&all_it, &range, 0);
	ASSERT_EQ(0, rv);
	has_next = rope_iterator_next(&all_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "red");
	ASSERT_EQ(3, size);
	has_next = rope_iterator_next(&all_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "blue");
	ASSERT_EQ(4, size);
	has_next = rope_iterator_next(&all_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "magenta");
	ASSERT_EQ(7, size);
	has_next = rope_iterator_next(&all_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "plain");
	ASSERT_EQ(5, size);
	has_next = rope_iterator_next(&all_it, &data, &size);
	ASSERT_FALSE(has_next);
	rv = rope_iterator_cleanup(&all_it);
	ASSERT_EQ(0, rv);

	struct RopeIterator red_it = {0};
	rv = rope_iterator_init(&red_it, &range, TAG_RED);
	ASSERT_EQ(0, rv);
	has_next = rope_iterator_next(&red_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "red");
	ASSERT_EQ(3, size);
	rv = rope_iterator_next(&red_it, &data, &size);
	ASSERT_TRUE(has_next);
	ASSERT_STREQ((const char *)data, "magenta");
	ASSERT_EQ(7, size);
	rv = rope_iterator_cleanup(&red_it);
	ASSERT_EQ(0, rv);

	char *str = rope_range_to_str(&range, TAG_RED);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(str, "redmagenta");
	free(str);

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
TEST(iterator_tagged_filter)
END_TESTS
