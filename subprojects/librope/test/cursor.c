#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
cursor_basic(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "This is a string");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, 0, 9);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "n awesome", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	ASSERT_TRUE(NULL != node);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)9, size);
	ASSERT_EQ(0, memcmp(value, "This is a", size));

	bool has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	ASSERT_TRUE(NULL != node);
	value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)9, size);
	ASSERT_EQ(0, memcmp(value, "n awesome", size));

	has_next = rope_node_next(&node);
	ASSERT_NE(false, has_next);
	ASSERT_TRUE(NULL != node);
	value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)7, size);
	ASSERT_EQ(0, memcmp(value, " string", size));

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋🦶");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, 0, 1);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, u8"🙂", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	ASSERT_TRUE(NULL != node);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)8, size);
	ASSERT_EQ(0, memcmp(value, u8"👋🙂", size));

	// struct RopeNode *node = rope_first(&r);
	// ASSERT_TRUE(NULL != node);
	// size_t size = 0;
	// const uint8_t *value = rope_node_value(node, &size);
	// ASSERT_TRUE(NULL != value);
	// ASSERT_EQ((size_t)4, size);
	// ASSERT_EQ(0, memcmp(value, u8"👋", size));

	// bool has_next = rope_node_next(&node);
	// ASSERT_EQ(true, has_next);
	// ASSERT_TRUE(NULL != node);
	// value = rope_node_value(node, &size);
	// ASSERT_TRUE(NULL != value);
	// ASSERT_EQ((size_t)4, size);
	// ASSERT_EQ(0, memcmp(value, u8"🙂", size));

	bool has_next = rope_node_next(&node);
	ASSERT_NE(false, has_next);
	ASSERT_TRUE(NULL != node);
	value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, u8"🦶", size));

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_event(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"This is a test");
	ASSERT_EQ(0, rv);

	struct RopeCursor c1 = {0};
	rv = rope_cursor_init(&c1, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c1, 0, 10);
	ASSERT_EQ(0, rv);

	struct RopeCursor c2 = {0};
	rv = rope_cursor_init(&c2, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c2, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c2, 7);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)3, c1.index);

	rv = rope_cursor_cleanup(&c1);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_cleanup(&c2);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_move_codepoint(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "A😃C");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_index(&c, 1, 0);
	ASSERT_EQ(0, rv);
	int32_t cp = rope_cursor_codepoint(&c);
	ASSERT_EQ(128515, cp);

	rv = rope_cursor_move(&c, -2);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((size_t)0, c.index);

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_insert_cursor_move(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeCursor c1 = {0};
	rv = rope_cursor_init(&c1, &r);
	ASSERT_EQ(0, rv);

	struct RopeCursor c2 = {0};
	rv = rope_cursor_init(&c2, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c1, "c1", 0);

	ASSERT_EQ(0, c2.index);
	ASSERT_EQ(2, c1.index);

	rv = rope_cursor_cleanup(&c1);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_cleanup(&c2);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_delete_collapses_following(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor start = {0};
	rv = rope_cursor_init(&start, &r);
	ASSERT_EQ(0, rv);
	struct RopeCursor follower = {0};
	rv = rope_cursor_init(&follower, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_index(&follower, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&start, 5);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)0, follower.index);
	ASSERT_EQ(0, rope_char_size(&r));

	rv = rope_cursor_cleanup(&start);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_cleanup(&follower);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_delete_updates_tagged_cursors(void) {
	const uint64_t TAG_RED = 1u << 0;
	const uint64_t TAG_BLUE = 1u << 1;
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	struct RopeCursor editor = {0};
	rv = rope_cursor_init(&editor, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&editor, "red", TAG_RED);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&editor, rope_char_size(&r), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_insert_str(&editor, "blue", TAG_BLUE);
	ASSERT_EQ(0, rv);

	struct RopeCursor tagged = {0};
	rv = rope_cursor_init(&tagged, &r);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&tagged, 0, TAG_BLUE);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((size_t)3, tagged.index);

	rv = rope_cursor_move_to_index(&editor, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&editor, 3);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)0, tagged.index);
	return;
	ASSERT_EQ(4, rope_char_size(&r));

	struct RopeNode *node = rope_first(&r);
	ASSERT_TRUE(NULL != node);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	ASSERT_STREQS("blue", (const char *)value, size);

	rv = rope_cursor_cleanup(&tagged);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_cleanup(&editor);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(cursor_basic)
TEST(cursor_utf8)
TEST(cursor_event)
TEST(cursor_move_codepoint)
TEST(cursor_insert_cursor_move)
TEST(cursor_delete_collapses_following)
TEST(cursor_delete_updates_tagged_cursors)
END_TESTS
