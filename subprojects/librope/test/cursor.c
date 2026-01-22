#include "common.h"
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

	rv = rope_cursor_move_to_line_col(&c, 0, 9);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "n awesome", 0);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['This is an awesome',' string']", r.root);

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

	rv = rope_cursor_move_to_line_col(&c, 0, 1);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, u8"🙂", 0);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	ASSERT_TRUE(NULL != node);
	size_t size = rope_byte_size(&r);
	char *value = rope_to_str(&r, 0);
	ASSERT_TRUE(NULL != value);
	ASSERT_EQ((size_t)12, size);
	ASSERT_EQ(0, memcmp(value, u8"👋🙂🦶", size));
	free(value);

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

	rv = rope_cursor_move_to_line_col(&c1, 0, 10);
	ASSERT_EQ(0, rv);

	struct RopeCursor c2 = {0};
	rv = rope_cursor_init(&c2, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_line_col(&c2, 0, 0);
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

static void
cursor_delete_at_eof(void) {
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	const char *hello_world = "Hello World";
	for (size_t i = 0; i < strlen(hello_world); i++) {
		rv = rope_cursor_insert(&c, (const uint8_t *)&hello_world[i], 1, i);
		ASSERT_EQ(0, rv);
		// rope_node_print(r.root, "/tmp/node.dot");
	}
	rv = rope_cursor_move_to_index(&c, 10, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&c, 1);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(result, "Hello Worl");
	free(result);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
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
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, before_good, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_move_to_index(&c, 326, 0);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_delete(&c, 10);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(after_good, result);
	free(result);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
}

static void
test_cursor_delete_multi_node(void) {
	struct Rope r = {0};
	struct RopeCursor c = {0};
	int rv = 0;

	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rope_node_free(r.root, &r.pool);

	r.root = from_str(
			&r.pool, "[[['HE','L'],['L','O']],[['W','O'],['R','LD']]]");

	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_index(&c, 1, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, 8);

	ASSERT_JSONEQ("['H','D']", r.root);

	rope_cursor_cleanup(&c);
	rope_cleanup(&r);
}

static void
test_cursor_delete_root_noninline_leaf(void) {
	int rv = 0;
	struct Rope rope = {0};
	struct RopeRange range = {0};

	rv = rope_init(&rope);
	ASSERT_EQ(0, rv);
	rv = rope_range_init(&range, &rope);
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
	rope_cleanup(&rope);
}

static void
test_cursor_editing_traces_error1(void) {
	int rv = 0;
	struct Rope rope = {0};
	struct RopeCursor c = {0};

	rv = rope_init(&rope);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_init(&c, &rope);
	ASSERT_EQ(0, rv);

	rope_node_free(rope.root, &rope.pool);
	rope.root = from_str(&rope.pool, STRFY([
		[ [ "\n\n\n", "When I see peop" ], "le again, they a" ], "lways"
	]));

	rv = rope_cursor_move_to_index(&c, 0, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_insert_str(&c, "C", 0);
	ASSERT_EQ(0, rv);

	char *str = rope_to_str(&rope, 0);
	ASSERT_STREQ("C\n\n\nWhen I see people again, they always", str);
	free(str);

	rope_cursor_cleanup(&c);
	rope_cleanup(&rope);
}

static void
test_cursor_editing_traces_error2(void) {
	int rv = 0;
	struct Rope rope = {0};
	struct RopeCursor c = {0};

	rv = rope_init(&rope);
	ASSERT_EQ(0, rv);
	rv = rope_cursor_init(&c, &rope);
	ASSERT_EQ(0, rv);

	rope_node_free(rope.root, &rope.pool);
	rope.root = from_str(
			&rope.pool,
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
	rv = rope_cursor_move_to_index(&c, 742, 0);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_delete(&c, 18);

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
}

static void
test_rope_size(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	// "Hello\nWorld" - 11 bytes, 11 chars, 1 newline
	rv = rope_append_str(&r, "Hello\nWorld");
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)11, rope_size(&r, ROPE_BYTE));
	ASSERT_EQ((size_t)11, rope_size(&r, ROPE_CHAR));
	ASSERT_EQ((size_t)1, rope_size(&r, ROPE_LINE));

	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_rope_size_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	// "👋🙂" - 8 bytes, 2 grapheme chars, 2 codepoints
	rv = rope_append_str(&r, u8"👋🙂");
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)8, rope_size(&r, ROPE_BYTE));
	ASSERT_EQ((size_t)2, rope_size(&r, ROPE_CHAR));
	ASSERT_EQ((size_t)2, rope_size(&r, ROPE_CP));

	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_cursor_delete_at_bytes(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_index(&c, 0, 0);
	ASSERT_EQ(0, rv);

	// Delete 5 bytes "Hello"
	rv = rope_cursor_delete_at(&c, ROPE_BYTE, 5);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(" World", result);
	free(result);

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_cursor_delete_at_lines(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Line1\nLine2\nLine3");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	rv = rope_cursor_move_to_index(&c, 0, 0);
	ASSERT_EQ(0, rv);

	// Delete 2 newlines (2 lines worth)
	rv = rope_cursor_delete_at(&c, ROPE_LINE, 2);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ("Line3", result);
	free(result);

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_cursor_move_at_bytes(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	// "👋" is 4 bytes
	rv = rope_append_str(&r, u8"👋Hello");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	// Move 4 bytes to skip the emoji
	rv = rope_cursor_move_by(&c, ROPE_BYTE, 4);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)4, c.index);

	// Insert at current position
	rv = rope_cursor_insert_str(&c, "!", 0);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(u8"👋!Hello", result);
	free(result);

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_cursor_move_at_lines(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Line1\nLine2\nLine3");
	ASSERT_EQ(0, rv);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r);
	ASSERT_EQ(0, rv);

	// Move forward 1 line (past the first \n)
	rv = rope_cursor_move_by(&c, ROPE_LINE, 1);
	ASSERT_EQ(0, rv);

	ASSERT_EQ((size_t)6, c.index); // "Line1\n" is 6 bytes

	rv = rope_cursor_cleanup(&c);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_rope_insert_at_bytes(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	// "👋" is 4 bytes
	rv = rope_append_str(&r, u8"👋World");
	ASSERT_EQ(0, rv);

	// Insert at byte 4 (after emoji)
	rv = rope_insert_at(&r, ROPE_BYTE, 4, (const uint8_t *)"Hello ", 6);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ(u8"👋Hello World", result);
	free(result);

	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_rope_delete_at_bytes(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World");
	ASSERT_EQ(0, rv);

	// Delete 6 bytes "Hello " from start
	rv = rope_delete_at(&r, ROPE_BYTE, 0, 6);
	ASSERT_EQ(0, rv);

	char *result = rope_to_str(&r, 0);
	ASSERT_STREQ("World", result);
	free(result);

	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

static void
test_range_move_to_at(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"👋Hello World");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r);
	ASSERT_EQ(0, rv);

	// Move start to byte 4 (after emoji)
	rv = rope_range_start_move_to_at(&range, ROPE_BYTE, 4, 0);
	ASSERT_EQ(0, rv);

	// Move end to byte 9 (after "Hello")
	rv = rope_range_end_move_to_at(&range, ROPE_BYTE, 9, 0);
	ASSERT_EQ(0, rv);

	char *result = rope_range_to_str(&range, 0);
	ASSERT_STREQ("Hello", result);
	free(result);

	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
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
END_TESTS
