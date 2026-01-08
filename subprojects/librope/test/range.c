#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
range_basic(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World This is a string");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
range_insert_delete(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_range_insert_str(&range, "Hello", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_STREQS("Hello", (char *)data, size);

	rv = rope_range_end_move_to_index(
			&range, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);
	rv = rope_range_start_move_to_index(&range, 0, 0);
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
