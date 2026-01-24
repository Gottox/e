#include "common.h"
#include <rope.h>
#include <stdbool.h>
#include <string.h>
#include <testlib.h>

static void
range_basic(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World This is a string");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
	ASSERT_EQ(0, rv);
}

static void
range_insert_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_range_insert_str(&range, "Hello", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_STREQS("Hello", (char *)data, size);

	rv = rope_cursor_move_to(
			rope_range_end(&range), ROPE_CHAR, rope_size(&r, ROPE_CHAR), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_range_delete(&range);
	ASSERT_EQ(0, rv);

	node = rope_node_first(r.root);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)0, size);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_to_str(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 6, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 11, 0);
	ASSERT_EQ(0, rv);

	char *str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("World", str);
	free(str);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 11, 0);
	ASSERT_EQ(0, rv);

	str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("Hello World", str);
	free(str);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 5, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 5, 0);
	ASSERT_EQ(0, rv);

	str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("", str);
	free(str);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_select_portion(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World This is a Test");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 6, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 11, 0);
	ASSERT_EQ(0, rv);

	char *str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("World", str);
	free(str);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 12, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 19, 0);
	ASSERT_EQ(0, rv);

	str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("This is", str);
	free(str);

	rv = rope_range_delete(&range);
	ASSERT_EQ(0, rv);

	str = rope_to_str(&r, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("Hello World  a Test", str);
	free(str);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

struct callback_data {
	int call_count;
	bool was_damaged;
};

static void
test_callback(
		struct Rope *rope, struct RopeRange *range, bool damaged,
		void *userdata) {
	(void)rope;
	(void)range;
	struct callback_data *data = userdata;
	data->call_count++;
	data->was_damaged = damaged;
}

static void
range_callback(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 6, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 11, 0);
	ASSERT_EQ(0, rv);

	struct callback_data data = {0};
	rv = rope_range_set_callback(&range, test_callback, &data);
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(&c, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "Hi ", 0);
	ASSERT_EQ(0, rv);

	ASSERT_TRUE(data.call_count > 0);

	ASSERT_EQ((size_t)9, range.cursor_start.byte_index);

	rope_cursor_cleanup(&c);
	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_ordering(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 5, 0);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((size_t)0, rope_range_start(&range)->byte_index);
	ASSERT_EQ((size_t)5, rope_range_end(&range)->byte_index);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 8, 0);
	ASSERT_EQ(0, rv);

	// trigger rerdering by deleting the range, which will collapse to the start
	rv = rope_range_delete(&range);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)8, rope_range_start(&range)->byte_index);
	ASSERT_EQ((size_t)8, rope_range_end(&range)->byte_index);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 5, 0);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((size_t)0, rope_range_start(&range)->byte_index);
	ASSERT_EQ((size_t)5, rope_range_end(&range)->byte_index);

	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)0, rope_range_start(&range)->byte_index);
	ASSERT_EQ((size_t)0, rope_range_end(&range)->byte_index);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_insert_raw(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	const uint8_t data[] = {'H', 'e', 'l', 'l', 'o'};
	rv = rope_range_insert(&range, data, sizeof(data), 0);
	ASSERT_EQ(0, rv);

	char *str = rope_to_str(&r, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("Hello", str);
	free(str);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(
			rope_range_end(&range), ROPE_CHAR, rope_size(&r, ROPE_CHAR), 0);
	ASSERT_EQ(0, rv);

	const uint8_t data2[] = {' ', 'W', 'o', 'r', 'l', 'd'};
	rv = rope_range_insert(&range, data2, sizeof(data2), 0);
	ASSERT_EQ(0, rv);

	str = rope_to_str(&r, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(" World", str);
	free(str);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_utf8(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"Héllo Wörld");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 6, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 11, 0);
	ASSERT_EQ(0, rv);

	char *str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ(u8"Wörld", str);
	free(str);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
range_multinode(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rope_node_free(r.root, &pool);
	r.root = from_str(&pool, "[['Hello',' '],['Wor','ld']]");

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(rope_range_start(&range), ROPE_CHAR, 4, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(rope_range_end(&range), ROPE_CHAR, 9, 0);
	ASSERT_EQ(0, rv);

	char *str = rope_range_to_cstr(&range, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("o Wor", str);
	free(str);

	rv = rope_range_delete(&range);
	ASSERT_EQ(0, rv);

	str = rope_to_str(&r, 0);
	ASSERT_TRUE(str != NULL);
	ASSERT_STREQ("Hellld", str);
	free(str);

	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

DECLARE_TESTS
TEST(range_basic)
TEST(range_insert_delete)
TEST(range_to_str)
TEST(range_select_portion)
TEST(range_callback)
TEST(range_ordering)
TEST(range_insert_raw)
TEST(range_utf8)
TEST(range_multinode)
END_TESTS
