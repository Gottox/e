#include "common.h"
#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
cursor_basic(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "This is a string");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 9, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "n awesome", 0);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['This is an awesome',' string']", r.root);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_utf8(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋🦶");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 1, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, u8"🙂", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);
	ASSERT_TRUE(NULL != node);
	size_t size = rope_size(&r, ROPE_BYTE);
	char *value = rope_to_str(&r, 0);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)12, size);
	ASSERT_EQ(0, memcmp(value, u8"👋🙂🦶", size));
	free(value);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_event(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"This is a test");
	ASSERT_EQ(0, rv);

	struct RopeCursor c1 = {0};
	rv = rope_cursor_init(&c1, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c1, ROPE_CHAR, 10, 0);
	ASSERT_EQ(0, rv);

	struct RopeCursor c2 = {0};
	rv = rope_cursor_init(&c2, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c2, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c2, ROPE_CHAR, 7);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)3, c1.byte_index);

	rope_cursor_cleanup(&c1);
	ASSERT_EQ(0, rv);
	rope_cursor_cleanup(&c2);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_insert_cursor_move(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
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

	ASSERT_EQ(0, c2.byte_index);
	ASSERT_EQ(2, c1.byte_index);

	rope_cursor_cleanup(&c1);
	ASSERT_EQ(0, rv);
	rope_cursor_cleanup(&c2);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_delete_collapses_following(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor start = {0};
	rv = rope_cursor_init(&start, &r);
	ASSERT_EQ(0, rv);
	struct RopeCursor follower = {0};
	rv = rope_cursor_init(&follower, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&follower, ROPE_CHAR, rope_size(&r, ROPE_BYTE), 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&start, ROPE_CHAR, 5);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)0, follower.byte_index);
	ASSERT_EQ(0, rope_size(&r, ROPE_BYTE));

	rope_cursor_cleanup(&start);
	ASSERT_EQ(0, rv);
	rope_cursor_cleanup(&follower);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_delete_updates_tagged_cursors(void) {
	const uint64_t TAG_RED = 1u << 0;
	const uint64_t TAG_BLUE = 1u << 1;
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	struct RopeCursor editor = {0};
	rv = rope_cursor_init(&editor, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&editor, "red", TAG_RED);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(&editor, ROPE_CHAR, rope_size(&r, ROPE_BYTE), 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_insert_str(&editor, "blue", TAG_BLUE);
	ASSERT_EQ(0, rv);

	struct RopeCursor tagged = {0};
	rv = rope_cursor_init(&tagged, &r);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(&tagged, ROPE_CHAR, 0, TAG_BLUE);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((size_t)3, tagged.byte_index);

	rv = rope_cursor_move_to(&editor, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&editor, ROPE_CHAR, 3);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)0, tagged.byte_index);
	ASSERT_EQ(4, rope_size(&r, ROPE_BYTE));

	struct RopeNode *node = rope_node_first(r.root);
	ASSERT_TRUE(NULL != node);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	ASSERT_STREQS("blue", (const char *)value, size);

	rope_cursor_cleanup(&tagged);
	ASSERT_EQ(0, rv);
	rope_cursor_cleanup(&editor);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_delete_at_eof(void) {
	struct RopePool pool = {0};
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	const char *hello_world = "Hello World";
	for (size_t i = 0; i < strlen(hello_world); i++) {
		rv = rope_cursor_insert_data(
				&c, (const uint8_t *)&hello_world[i], 1, i);
		ASSERT_EQ(0, rv);
	}
	rv = rope_cursor_move_to(&c, ROPE_CHAR, 10, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&c, ROPE_CHAR, 1);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(result, "Hello Worl");
	free(result);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
cursor_delete_edit_traces_error1(void) {
	const char before_good[] = {
			0x23, 0x20, 0x4a, 0x53, 0x4f, 0x4e, 0x20, 0x43, 0x52, 0x44, 0x54,
			0x20, 0x50, 0x61, 0x74, 0x63, 0x68, 0x0a, 0x0a, 0x50, 0x61, 0x74,
			0x63, 0x68, 0x20, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c,
			0x20, 0x73, 0x70, 0x65, 0x63, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
			0x69, 0x6f, 0x6e, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x4a, 0x53, 0x4f,
			0x4e, 0x20, 0x43, 0x52, 0x44, 0x54, 0x2e, 0x0a, 0x0a, 0x0a, 0x23,
			0x23, 0x20, 0x53, 0x65, 0x72, 0x69, 0x61, 0x6c, 0x69, 0x7a, 0x61,
			0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x0a, 0x23, 0x23, 0x23, 0x20, 0x4a,
			0x53, 0x4f, 0x4e, 0x20, 0x45, 0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e,
			0x67, 0x0a, 0x0a, 0x23, 0x23, 0x23, 0x20, 0x43, 0x72, 0x65, 0x61,
			0x74, 0x65, 0x20, 0x4f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6f,
			0x6e, 0x0a, 0x0a, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x63,
			0x72, 0x65, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x65, 0x78, 0x61,
			0x6d, 0x70, 0x6c, 0x65, 0x3a, 0x0a, 0x0a, 0x60, 0x60, 0x60, 0x6a,
			0x73, 0x6f, 0x6e, 0x0a, 0x7b, 0x0a, 0x20, 0x20, 0x22, 0x6f, 0x70,
			0x22, 0x3a, 0x20, 0x22, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x22,
			0x2c, 0x0a, 0x20, 0x20, 0x22, 0x74, 0x79, 0x70, 0x65, 0x22, 0x3a,
			0x20, 0x22, 0x73, 0x74, 0x72, 0x22, 0x2c, 0x0a, 0x7d, 0x0a, 0x60,
			0x60, 0x60, 0x0a, 0x0a, 0x54, 0x79, 0x70, 0x65, 0x20, 0x64, 0x65,
			0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x0a, 0x0a,
			0x60, 0x60, 0x60, 0x74, 0x79, 0x70, 0x65, 0x73, 0x63, 0x72, 0x69,
			0x70, 0x74, 0x0a, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63,
			0x65, 0x20, 0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x4f, 0x70, 0x65,
			0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x7b, 0x0a, 0x20, 0x20,
			0x6f, 0x70, 0x3a, 0x20, 0x22, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65,
			0x22, 0x3b, 0x0a, 0x20, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x20,
			0x22, 0x76, 0x61, 0x6c, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x6f, 0x62,
			0x6a, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x73, 0x74, 0x72, 0x22, 0x20,
			0x7c, 0x20, 0x22, 0x61, 0x72, 0x72, 0x22, 0x20, 0x7c, 0x20, 0x22,
			0x74, 0x75, 0x70, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x63, 0x6f, 0x6e,
			0x73, 0x74, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x62, 0x69, 0x6e, 0x22,
			0x3b, 0x0a, 0x7d, 0x0a, 0x60, 0x60, 0x60, 0x0a, 0,
	};
	const char after_good[] = {
			0x23, 0x20, 0x4a, 0x53, 0x4f, 0x4e, 0x20, 0x43, 0x52, 0x44, 0x54,
			0x20, 0x50, 0x61, 0x74, 0x63, 0x68, 0x0a, 0x0a, 0x50, 0x61, 0x74,
			0x63, 0x68, 0x20, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c,
			0x20, 0x73, 0x70, 0x65, 0x63, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
			0x69, 0x6f, 0x6e, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x4a, 0x53, 0x4f,
			0x4e, 0x20, 0x43, 0x52, 0x44, 0x54, 0x2e, 0x0a, 0x0a, 0x0a, 0x23,
			0x23, 0x20, 0x53, 0x65, 0x72, 0x69, 0x61, 0x6c, 0x69, 0x7a, 0x61,
			0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x0a, 0x23, 0x23, 0x23, 0x20, 0x4a,
			0x53, 0x4f, 0x4e, 0x20, 0x45, 0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e,
			0x67, 0x0a, 0x0a, 0x23, 0x23, 0x23, 0x20, 0x43, 0x72, 0x65, 0x61,
			0x74, 0x65, 0x20, 0x4f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6f,
			0x6e, 0x0a, 0x0a, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x63,
			0x72, 0x65, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x65, 0x78, 0x61,
			0x6d, 0x70, 0x6c, 0x65, 0x3a, 0x0a, 0x0a, 0x60, 0x60, 0x60, 0x6a,
			0x73, 0x6f, 0x6e, 0x0a, 0x7b, 0x0a, 0x20, 0x20, 0x22, 0x6f, 0x70,
			0x22, 0x3a, 0x20, 0x22, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x22,
			0x2c, 0x0a, 0x20, 0x20, 0x22, 0x74, 0x79, 0x70, 0x65, 0x22, 0x3a,
			0x20, 0x22, 0x73, 0x74, 0x72, 0x22, 0x2c, 0x0a, 0x7d, 0x0a, 0x60,
			0x60, 0x60, 0x0a, 0x0a, 0x54, 0x79, 0x70, 0x65, 0x20, 0x64, 0x65,
			0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x0a, 0x0a,
			0x60, 0x60, 0x60, 0x74, 0x79, 0x70, 0x65, 0x73, 0x63, 0x72, 0x69,
			0x70, 0x74, 0x0a, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63,
			0x65, 0x20, 0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x4f, 0x70, 0x65,
			0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x7b, 0x0a, 0x20, 0x20,
			0x6f, 0x70, 0x3a, 0x20, 0x22, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65,
			0x22, 0x3b, 0x0a, 0x20, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x20,
			0x22, 0x76, 0x61, 0x6c, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x6f, 0x62,
			0x6a, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x73, 0x74, 0x72, 0x22, 0x20,
			0x7c, 0x20, 0x22, 0x61, 0x72, 0x72, 0x22, 0x20, 0x7c, 0x20, 0x22,
			0x74, 0x75, 0x70, 0x22, 0x20, 0x7c, 0x20, 0x22, 0x62, 0x69, 0x6e,
			0x22, 0x3b, 0x0a, 0x7d, 0x0a, 0x60, 0x60, 0x60, 0x0a, 0,
	};
	struct RopePool pool = {0};
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, before_good, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to(&c, ROPE_CHAR, 326, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&c, ROPE_CHAR, 10);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(after_good, result);
	free(result);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_delete_multi_node(void) {
	struct RopePool pool = {0};
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rope_node_free(r.root, &pool);

	r.root = from_str(&pool, "[[['HE','L'],['L','O']],[['W','O'],['R','LD']]]");

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 1, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, ROPE_CHAR, 8);

	ASSERT_JSONEQ("['H','D']", r.root);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_delete_root_noninline_leaf(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	struct RopeRange range = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	rv = rope_range_insert_str(
			&range,
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
			0x0);
	ASSERT_EQ(0, rv);
	rv = rope_range_delete(&range);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&range);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_editing_traces_error1(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope rope = {0};
	struct RopeCursor c = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&rope, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_init(&c, &rope);
	ASSERT_EQ(0, rv);

	rope_node_free(rope.root, &pool);
	rope.root = from_str(&pool, STRFY([
		[ [ "\n\n\n", "When I see peop" ], "le again, they a" ], "lways"
	]));

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "C", 0);
	ASSERT_EQ(0, rv);

	char *str = rope_to_str(&rope, 0);
	ASSERT_STREQ("C\n\n\nWhen I see people again, they always", str);
	free(str);

	rope_cursor_cleanup(&c);
	rope_cleanup(&rope);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_editing_traces_error2(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope rope = {0};
	struct RopeCursor c = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&rope, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_init(&c, &rope);
	ASSERT_EQ(0, rv);

	rope_node_free(rope.root, &pool);
	rope.root = from_str(
			&pool,
			"[[[[[[\"<script>\",\"\\n\"],\"\\n\"],[[[\"\\texport let "
			"\",\"room\"],\"\\n\"],[[\"\\texport let "
			"\",\"connection\"],\"\\n\"]]],[[\"\\texport let "
			"\",\"state\"],[\"\\n\",[[\"\\texport let "
			"\",\"players\"],\"\\n\"]]]],[[[[[\"\\texport let "
			"\",\"rounds\"],[\"\\n\",[\"\\texport let "
			"sec\",\"ond\"]]],\"s_per_round\\n\"],[[\"\\texport let "
			"\",\"_active_sessions\"],[[\"\\n\\t\",\"// \"],\"export let "
			"state\"]]],[\"\\n\\nconst update_s\",[\"tate = async \",\"patch "
			"=> \"]]]],[[[[[[[\"{\\n\\tawait "
			"\",[\"fetch(`${room}/c\",\"onfigure`, "
			"\"]],\"{\\n\\t\\tmethod\"],[[[\": 'POST',\\n\\t\\tunit\",\": "
			"'same-origin',\"],[\"\\n\\t\\theaders: "
			"{\",\"\\n\\t\\t\\t'content-typ\"]],[\"e': "
			"'application\",\"/json',\\n\\t\\t},\\n\\t\\tb\"]]],[[[\"ody: "
			"JSON.\",\"stringify(patch)\"],\"\\n\\t})\"],[[[\"\\n}\\n\\nconst "
			"upd = \",\"patch => () => "
			"\"],[\"update_state(pat\",\"ch)\"]],\"\\n\"]]],[[[[[\"\\n// "
			"\",\"const start = \"],[\"() => {\\n// "
			"\",\"\\tconsole\"]],[[\".log('start!')\\n\",\"// "
			"\"],\"\\t\"]],[[\"update_state({st\",\"ate: "
			"'playing'})\"],[\"\\n// "
			"\",\"}\"]]],[[\"\\n\",\"\\n</"
			"script>\\n\\n<main>\"],[[\"\\n\\t<h1>Glass Bead\",\" Game "
			"Timer\"],\"</h1>\"]]]],[[[[\"\\n\\t<h\",\"4\"],\">Room:\"],[[\" "
			"<em>\",\"{room\"],\"}</em>\"]],[[\"</"
			"h\",\"4\"],\">\\n\\t<div>\"]]],[[[[[[\"{connection} \",\"/\"],\" "
			"{state}\"],[[\"</div>\",\"\\n\"],[\"\\n\\t<div "
			"id='\",\"config\"]]],[[\"'>\\n\\t\\t<h2>Config<\",\"/"
			"h2>\\n\\t\\t{#\"],[\"if state == "
			"'wai\",\"ting'\"]]],[[[[\"}\",\"\\n\"],\"\\t\\t\\t<button "
			"on:click={\"],[[[\"() => \",\"update_state({state: "
			"'playing'})\"],\"}>Start</button>\\n\"],[[\"\\t\\t{:else if state "
			"== 'playing'}\\n\\t\\t\\t<button on:click={\",\"u\"],\"({state: "
			"'paused'})}>Pause</button>\\n\"]]],[\"\\t\\t\",[[\"{:else if "
			"state == "
			"'\",\"paused\"],\"'}\\n\"]]]],[[[[[\"\\t\\t\\t\",\"<button "
			"\"],\"on:click={u\"],[\"({state: "
			"'\",[\"playing\",\"'})\"]]],[[\"}>Resume\",\"</"
			"button>\"],[[\"\\n\\t\\t{/"
			"if}\\n\",\"\\n\\t\\t<div>\\n\\t\\t\\t\"],\"<label>Number\"]]],[[[["
			"[\" of players<inpu\",\"t type='\"],\"number' "
			"\"],[\"on:input=\",\"></label>\"]],[[\"\\n\\t\\t\",\"</"
			"div>\"],[\"\\n\\t\",[\"</div>\",\"\\n</"
			"main>\\n\\n<style>\"]]]],[[[\"\\n\\n#config "
			"{\\n\\tmar\",[\"gin-top: "
			"2em;\",\"\\n}\"]],[\"\\n\",\"\\n\\t\"]],[\"/* \",[\"main "
			"{\\n\\t\\ttext-align: center;\\n\\t\\tpadding: "
			"1em;\\n\\t\\tmax-width: 240px;\\n\\t\\tmargin: 0 "
			"auto;\\n\\t}\\n\\n\\th1 {\\n\\t\\tcolor: "
			"#ff3e00;\\n\\t\\ttext-transform: uppercase;\\n\\t\\tfont-size: "
			"4em;\\n\\t\\tfont-weight: 100;\\n\\t}\\n\\n\\t@media (min-width: "
			"640px) {\\n\\t\\tmain {\\n\\t\\t\\tmax-width: "
			"none;\\n\\t\\t}\\n\\t}\",[\" */\",\"\\n</style>\"]]]]]]]]]");

	// [ 742, 18, "u" ]
	rv = rope_cursor_move_to(&c, ROPE_CHAR, 742, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, ROPE_CHAR, 18);

	rv = rope_cursor_insert_str(&c, "u", 0);
	ASSERT_EQ(0, rv);

	char *str = rope_to_str(&rope, 0);
	ASSERT_STREQ(
			"<script>\n\n\texport let room\n\texport let connection\n\texport "
			"let state\n\texport let players\n\texport let rounds\n\texport "
			"let seconds_per_round\n\texport let _active_sessions\n\t// export "
			"let state\n\nconst update_state = async patch => {\n\tawait "
			"fetch(`${room}/configure`, {\n\t\tmethod: 'POST',\n\t\tunit: "
			"'same-origin',\n\t\theaders: {\n\t\t\t'content-type': "
			"'application/json',\n\t\t},\n\t\tbody: "
			"JSON.stringify(patch)\n\t})\n}\n\nconst upd = patch => () => "
			"update_state(patch)\n\n// const start = () => {\n// "
			"\tconsole.log('start!')\n// \tupdate_state({state: "
			"'playing'})\n// }\n\n</script>\n\n<main>\n\t<h1>Glass Bead Game "
			"Timer</h1>\n\t<h4>Room: <em>{room}</em></h4>\n\t<div>{connection} "
			"/ {state}</div>\n\n\t<div "
			"id='config'>\n\t\t<h2>Config</h2>\n\t\t{#if state == "
			"'waiting'}\n\t\t\t<button on:click={u({state: "
			"'playing'})}>Start</button>\n\t\t{:else if state == "
			"'playing'}\n\t\t\t<button on:click={u({state: "
			"'paused'})}>Pause</button>\n\t\t{:else if state == "
			"'paused'}\n\t\t\t<button on:click={u({state: "
			"'playing'})}>Resume</button>\n\t\t{/"
			"if}\n\n\t\t<div>\n\t\t\t<label>Number of players<input "
			"type='number' "
			"on:input=></label>\n\t\t</div>\n\t</div>\n</"
			"main>\n\n<style>\n\n#config {\n\tmargin-top: 2em;\n}\n\n\t/* main "
			"{\n\t\ttext-align: center;\n\t\tpadding: 1em;\n\t\tmax-width: "
			"240px;\n\t\tmargin: 0 auto;\n\t}\n\n\th1 {\n\t\tcolor: "
			"#ff3e00;\n\t\ttext-transform: uppercase;\n\t\tfont-size: "
			"4em;\n\t\tfont-weight: 100;\n\t}\n\n\t@media (min-width: 640px) "
			"{\n\t\tmain {\n\t\t\tmax-width: none;\n\t\t}\n\t} "
			"*/\n</style>",
			str);
	free(str);

	rope_cursor_cleanup(&c);
	rope_cleanup(&rope);
	rope_pool_cleanup(&pool);
}

static void
test_rope_size(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello\nWorld");
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)11, rope_size(&r, ROPE_BYTE));
	ASSERT_EQ((size_t)11, rope_size(&r, ROPE_CHAR));
	ASSERT_EQ((size_t)1, rope_size(&r, ROPE_LINE));

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_rope_size_utf8(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋🙂");
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)8, rope_size(&r, ROPE_BYTE));
	ASSERT_EQ((size_t)2, rope_size(&r, ROPE_CHAR));
	ASSERT_EQ((size_t)2, rope_size(&r, ROPE_CP));

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_delete_at_bytes(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, ROPE_BYTE, 5);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(" World", result);
	free(result);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_delete_at_lines(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Line1\nLine2\nLine3");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_CHAR, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, ROPE_LINE, 2);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ("Line3", result);
	free(result);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_move_at_bytes(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋Hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	// Move 4 bytes to skip the emoji
	rv = rope_cursor_move_by(&c, ROPE_BYTE, 4);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)4, c.byte_index);

	// Insert at current position
	rv = rope_cursor_insert_str(&c, "!", 0);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(u8"👋!Hello", result);
	free(result);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_move_at_lines(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Line1\nLine2\nLine3");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_by(&c, ROPE_LINE, 1);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)6, c.byte_index);

	rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_rope_insert_at_bytes(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋World");
	ASSERT_EQ(0, rv);

	rv = rope_insert(&r, ROPE_BYTE, 4, (const uint8_t *)"Hello ", 6);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(u8"👋Hello World", result);
	free(result);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_rope_delete_at_bytes(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, ROPE_BYTE, 0, 6);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ("World", result);
	free(result);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_range_move_to_at(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋Hello World");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	struct RopeCursor *start = rope_range_start(&range);
	rv = rope_cursor_move_to(start, ROPE_BYTE, 4, 0);
	ASSERT_EQ(0, rv);

	struct RopeCursor *end = rope_range_end(&range);
	rv = rope_cursor_move_to(end, ROPE_BYTE, 9, 0);
	ASSERT_EQ(0, rv);

	char *result = rope_range_to_cstr(&range, 0);
	ASSERT_STREQ("Hello", result);
	free(result);

	rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_move_by_oob_forward(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_append_str(&r, "hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_by(&c, ROPE_BYTE, 5);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_by(&c, ROPE_BYTE, 1);
	ASSERT_EQ(-ROPE_ERROR_OOB, rv);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_move_by_oob_backward(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_append_str(&r, "hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_by(&c, ROPE_BYTE, -1);
	ASSERT_EQ(-ROPE_ERROR_OOB, rv);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_move_to_oob(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_append_str(&r, "hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to(&c, ROPE_BYTE, 6, 0);
	ASSERT_EQ(-ROPE_ERROR_OOB, rv);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_cursor_integrity_fail1(void) {
	// clang-format off
	static const char *tree = STRFY([[[[[["\\documentclass[a4paper,twocolumn,10pt]{article}\n\\usepackage[utf8]{inputenc}\n\\usepackage{amsmath} % align environment\n\\usepackage{mathptmx} % times roman, including math\n\\usepackage[hyphens]{url}\n\\usepackage{doi}\n\\usepackage{hyperref}\n\\usepackage[numbers,sort]{natbib}\n\\hyphenation{da-ta-cen-ter da-ta-cen-ters}\n\\frenchspacing\n\n% Placeholder character like \\textvisiblespace, but works in math mode\n\\newcommand\\placeholder{%\n  \\makebox[0.7em]{%\n    \\kern.07em\n    \\vrule height.3ex\n    \\hrulefill\n    \\vrule height.3ex\n    \\kern.07em\n  }%\n}\n\n\\begin{document}\n\\sloppy\n\\title{Composing Data Structures for Collaborative Document Editing}\n\\aut","hor{}\n\\maketitle\n\n\\subsection*{Abstract}\n\n\\section{Introduction}\n\nCRDTs~\\cite{Shapiro:2011wy,Roh:2011dw}\n\n\\section{Operational Semantics}\n\nWe assume that each peer has a unique identifier (for example, the hash of its public key). Whenever an edit to a document is made at one of the peers, we generate a unique identifier for that edit operation using Lamport timestamps~\\cite{Lamport:1978jq}.\n\nA Lamport timestamp is a pair $(c, p)$ where",9223372036854775809],[" $p$ is the unique identifier of the peer on which the edit is made. $c$ is a counter that is stored at each peer an","d incremented for every operation. Whenever an operation is sent to other peers, the Lamport timestamp $(c, p)$ for that operation is included in the message.\n\nIf a peer receives ",9223372036854775809],9223372036854775810],[[["an operation with a counter value $c$ that is greater than the locally stored counter value, the local counter is increased to the value of the incoming counter. This ensures that if operation $o_1$ happened before $o_2$ (that is, the peer that generated $o_2$ had received and processed $o_1$ before $o_2$ was generated), then $o_2$ must have a greater counter value than $o_1$. Only concurrent operations can have equal counter values.\n\nWe can thus define a total ordering $<$ for Lamport timestamps:\n$$(c_1, p_1) < (c_2, p_2) \\;\\text{ iff }\\; (c_1 < c_2) \\vee (c_1 = c_2 \\wedge p_1 < p_2).$$\nIf one operation happened before another, this ordering is consistent with causality (the earlier operation has a lower timesetamp). If two operations are concurrent, their order according to $<$ is arbitrary but deterministic.","\n\n\\subsection{Ordered List Operations}\n\nThe \\emph{Replicated Growable Array} (RGA) datatype was originally defined by Roh et al.\\ using pseudocode~\\cite{Roh:2011dw}. In this section we present the algorithm in an alternative form which is more amenable to correctness proofs.\n\nEach peer that maintains a replica of an RGA stores a copy of its state $A$, which is a partial function from $\\mathit{id}$ to pairs of $(\\mathit{value}, \\mathit{next})$. The parameter $\\mathit{id}$ is either a Lamport timestamp or the special symbol $\\mathsf{head}$, indicating the head of the list. $\\mathit{next}$ is either a Lamport timestamp or the symbol $\\mathsf{tail}$, denoting the end of the list. $\\mathit{value}$ is any value of the datatype of list elements, or the special value $\\bot$, indicating the absence of a value.\n\nThe empty list $A_\\emptyset$ is represented as a function whose domain is the single value $\\mathsf{head}$:\n$$ ",9223372036854775809],"A_\\emptyset = \\{\\mathsf{head} \\mapsto (\\bot, \\mathsf{tail})\\}. $$",9223372036854775810],"\n\nWe use the notation $A[\\,a \\mapsto (b, c)\\,]$ to denote a function identical to $A$, ",9223372036854775811],9223372036854775812],[[[["except that $A(a)=(b,c)$.\n\nA list element is cre","ated with an $\\mathsf{insert}$ operation. Since ",9223372036854775809],["each operation has a Lamport timestamp, a list e","lement",9223372036854775809],9223372036854775810],[" is uni","quely identified by its",9223372036854775809],9223372036854775811],[[" timestamp, which remains immutable for the life","time of the document",9223372036854775809],[". We call that Lamport timestamp the ID of the l","ist element.",9223372036854775809],9223372036854775810],9223372036854775812],9223372036854775813],[[[[["\n\nThe operation $\\mathsf{insert}(\\mathit{id}, \\m","athit{prev}, v)$ is an instruction to insert the",9223372036854775809],[" value $v$ into a list at a position following t","he existing list element with ID $\\mathit{prev}$",9223372036854775809],9223372036854775810],[". An insertion at the head of the list is expres","sed as $\\mathsf{insert}(\\mathit{id}, \\mathsf{hea",9223372036854775809],9223372036854775811],[["d}, v)$. The $\\mathit{id}$ is the unique ID of t","his operation.\n\nWhen the operation is applied to",9223372036854775809]," the list state $A$, it produces a modified list",9223372036854775810],9223372036854775812],[[[[" state $A'","$ as follows:\n\\begin{align",9223372036854775809],"*}\n",9223372036854775810],["A'",[" &","= \\mathrm{apply}(A, \\mathsf{insert}(\\mathi",9223372036854775809],9223372036854775810],9223372036854775811],[[["t{id}, \\mathit{prev}, v)) ",["\\\\ ","&=",9223372036854775809],9223372036854775810],["\n\\begin{cases}\n\\qu","ad ",9223372036854775809],9223372036854775811],[["\\ma","thrm{apply}(A, \\mathsf{insert}(\\mathit{id}, n, v",9223372036854775809],["))"," \\\\\n    ",9223372036854775809],9223372036854775810],9223372036854775812],9223372036854775813],9223372036854775814],9223372036854775815],[[[[[["\\q","quad\\quad",9223372036854775809],["\\text{if ","}\\;",9223372036854775809],9223372036854775810],[[" A(\\mathit{prev}) = (\\placeholder, n) \\;","\\wedge\\;",9223372036854775809],[" \\ma","thit{id} < n",9223372036854775809],9223372036854775810],9223372036854775811],[[" \\\\\n\\quad ","A[\\,",9223372036854775809],["\\mat",["hit{prev} \\mapsto (v_p, \\mathit{id}),\\;"," \\mathit{id",9223372036854775809],9223372036854775810],9223372036854775811],9223372036854775812],[[["} \\mapsto (v, n)\\,","]",9223372036854775809],[[" \\\\\n   "," \\q",9223372036854775809],["quad\\quad","\\text{if ",9223372036854775809],9223372036854775810],9223372036854775811],[["}\\; A(\\mathit{prev}) = (v_p, n) \\;","\\wedge\\;",9223372036854775809],[" n < \\math","it{id}",9223372036854775809],9223372036854775810],9223372036854775812],9223372036854775813],[[[[[["\n\\en","d{cases}\n\\end{align",9223372036854775809],"*}\nwhere $<$ is the total order on Lamport times",9223372036854775810],[["tamps, with the additional req"," that $\\mathsf{tail} < (",9223372036854775809],["c, p) < \\mathsf{head}$ for any Lamport timestamp"," $(c, p)$.",9223372036854775809],9223372036854775810],9223372036854775811],["\n\nA",["pplying an $\\mathsf{i","nsert}$ operation is like inserting an element i",9223372036854775809],9223372036854775810],9223372036854775812],[[["nto a linked list, except that the function firs","t skips over list elements with an ID greater th",9223372036854775809],["an the ID of the new element being inserted. Thi","s has the effect of deterministically ordering c",9223372036854775809],9223372036854775810],["oncurrent insertions made at the same position o","f the list. This property is proved formally bel",9223372036854775809],9223372036854775811],9223372036854775813],[[[[["ow.\n\nThe operation $\\mathsf{delete}(\\mathit{id})","$ is an instr",9223372036854775809],"uction to delete the element with ID $\\mathit{id",9223372036854775810],["}$ from the list. T","he operation has the following semantics:\n\\begin",9223372036854775809],9223372036854775811],[["{align*}\nA' &= \\mathrm{apply}(\\mathsf{delete}(","\\m",9223372036854775809],[["athit{id})) \\\\ &","=\n",9223372036854775809],["A[\\,\\mathit{id} \\mapsto (\\bot, n)\\",",]\n",9223372036854775809],9223372036854775810],9223372036854775811],9223372036854775812],[[["\\quad","\\text{if ",9223372036854775809],["}\\;"," A(\\mathit{id}) = (\\placeholder, n)",9223372036854775809],9223372036854775810],[["\n\\end",["{align*}\n","\n{\\footnotesize\n\\bibliographys",9223372036854775809],9223372036854775810],["tyle{plainnat}\n\\bibliography{references}{}}","\n\\end{document}\n",9223372036854775809],9223372036854775811],9223372036854775812],9223372036854775813],9223372036854775814],9223372036854775815],9223372036854775816]
);
	// clang-format on

	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rope_node_free(r.root, &pool);
	r.root = from_str(&pool, tree);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	check_integrity(r.root);

	// Applied patch at pos 4571: delete 0, insert "q"
	rv = rope_cursor_move_to(&c, ROPE_CHAR, 4571, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, ROPE_CHAR, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_data(&c, (const uint8_t *)"q", 1, 0);
	ASSERT_EQ(0, rv);

	check_integrity(r.root);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

DECLARE_TESTS
TEST(cursor_basic)
TEST(cursor_utf8)
TEST(cursor_event)
TEST(cursor_insert_cursor_move)
TEST(cursor_delete_collapses_following)
TEST(cursor_delete_updates_tagged_cursors)
TEST(cursor_delete_at_eof)
TEST(cursor_delete_edit_traces_error1)
TEST(test_cursor_delete_multi_node)
TEST(test_cursor_delete_root_noninline_leaf)
TEST(test_cursor_editing_traces_error1)
TEST(test_cursor_editing_traces_error2)
TEST(test_rope_size)
TEST(test_rope_size_utf8)
TEST(test_cursor_delete_at_bytes)
TEST(test_cursor_delete_at_lines)
TEST(test_cursor_move_at_bytes)
TEST(test_cursor_move_at_lines)
TEST(test_rope_insert_at_bytes)
TEST(test_rope_delete_at_bytes)
TEST(test_range_move_to_at)
TEST(test_cursor_move_by_oob_forward)
TEST(test_cursor_move_by_oob_backward)
TEST(test_cursor_move_to_oob)
TEST(test_cursor_integrity_fail1)
END_TESTS
