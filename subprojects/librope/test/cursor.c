#include <rope.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static char *
rope_to_cstr(struct Rope *rope) {
	const size_t byte_size = rope_byte_size(rope);
	char *buffer = malloc(byte_size + 1);
	ASSERT_TRUE(NULL != buffer);
	size_t offset = 0;
	for (struct RopeNode *node = rope_first(rope); node; rope_node_next(&node)) {
		size_t chunk_size = 0;
		const uint8_t *value = rope_node_value(node, &chunk_size);
		if (chunk_size > 0) {
			memcpy(buffer + offset, value, chunk_size);
			offset += chunk_size;
		}
	}
	buffer[offset] = '\0';
	ASSERT_EQ(byte_size, offset);
	return buffer;
}

static void
cursor_basic() {
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
cursor_utf8() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, u8"👋🦶", 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, 0, 1);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, u8"🙂", 1);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	ASSERT_TRUE(NULL != node);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, u8"👋", size));
	ASSERT_EQ(0, rope_node_tags(node));

	bool has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	ASSERT_TRUE(NULL != node);
	value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, u8"🙂", size));
	ASSERT_EQ(1, rope_node_tags(node));

	has_next = rope_node_next(&node);
	ASSERT_NE(false, has_next);
	ASSERT_TRUE(NULL != node);
	value = rope_node_value(node, &size);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, u8"🦶", size));
	ASSERT_EQ(1, rope_node_tags(node));

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_move_to_multiline() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	const char *initial = u8"alpha\nβeta🙂\n猫🙂\nfinal line";
	rv = rope_append_str(&r, initial);
	ASSERT_EQ(0, rv);

	struct RopeCursor cursor = {0};
	rv = rope_cursor_init(&cursor, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&cursor, 1, 0);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((rope_char_index_t)6, cursor.index);
	ASSERT_EQ((rope_index_t)1, cursor.line);
	ASSERT_EQ((rope_char_index_t)0, cursor.column);

        rv = rope_cursor_insert_str(&cursor, u8"🔥", 0);
        ASSERT_EQ(0, rv);
        ASSERT_EQ((rope_char_index_t)7, cursor.index);
        ASSERT_EQ((rope_index_t)1, cursor.line);
        ASSERT_EQ((rope_char_index_t)1, cursor.column);
        char *rope_contents = rope_to_cstr(&r);
        ASSERT_STREQ(u8"alpha\n🔥βeta🙂\n猫🙂\nfinal line", rope_contents);
        free(rope_contents);

        rv = rope_cursor_move_to(&cursor, 2, 0);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((rope_char_index_t)13, cursor.index);
	ASSERT_EQ((rope_index_t)2, cursor.line);
	ASSERT_EQ((rope_char_index_t)0, cursor.column);

        rv = rope_cursor_insert_str(&cursor, u8"雪", 0);
        ASSERT_EQ(0, rv);
        ASSERT_EQ((rope_char_index_t)14, cursor.index);
        ASSERT_EQ((rope_index_t)2, cursor.line);
        ASSERT_EQ((rope_char_index_t)1, cursor.column);
        rope_contents = rope_to_cstr(&r);
        ASSERT_STREQ(u8"alpha\n🔥βeta🙂\n雪猫🙂\nfinal line", rope_contents);
        free(rope_contents);

	rv = rope_cursor_cleanup(&cursor);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
cursor_event() {
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
cursor_move_codepoint() {
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

DECLARE_TESTS
TEST(cursor_basic)
TEST(cursor_utf8)
TEST(cursor_move_to_multiline)
TEST(cursor_event)
TEST(cursor_move_codepoint)
END_TESTS
