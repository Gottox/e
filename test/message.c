#include <e_konstrukt.h>
#include <e_struktur.h>
#include <testlib.h>
#include <string.h>

static void
test_message_parse_field(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "field1 field2 field3\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field1", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field2", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field3", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);



	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_double_quoted(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "\"hello world\" \"second field\"\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("hello world", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("second field", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_single_quoted(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "'hello world' 'second field'\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("hello world", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("second field", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_escape_unquoted(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "hello\\ world field2\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("hello world", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field2", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_escape_quoted(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "\"hello \\\"world\\\"\" field2\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("hello \"world\"", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field2", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_sized(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "abc @5 @3\nabcde\n123\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("abc", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("abcde", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("123", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_mixed(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "field1 \"quoted field\" @5 'single'\n12345\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field1", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("quoted field", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("12345", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("single", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_empty(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_multiple_spaces(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "field1    field2\n");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field1", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field2", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_missing_newline(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "field1 field2");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("field1", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_LT(rv, 0);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

static void
test_message_parse_sized_missing_newline(void) {
	int rv = 0;

	struct RopePool pool = {0};
	struct Rope message = {0};
	struct EMessageParser parser = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	rv = rope_init(&message, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&message, "abc @5\nabcde");
	ASSERT_EQ(0, rv);

	rv = e_message_parser_init(&parser, &message);
	ASSERT_EQ(0, rv);

	bool has_next = false;
	struct RopeRange field = {0};

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(0, rv);
	char *buf = rope_range_to_cstr(&field, 0);
	ASSERT_STREQ("abc", buf);
	free(buf);

	has_next = e_message_parse_next(&parser, &field, &rv);
	ASSERT_TRUE(has_next);
	ASSERT_LT(rv, 0);
	rope_range_cleanup(&field);

	e_message_parse_cleanup(&parser);
	rope_cleanup(&message);
	rope_pool_cleanup(&pool);
}

DECLARE_TESTS
TEST(test_message_parse_field)
TEST(test_message_parse_double_quoted)
TEST(test_message_parse_single_quoted)
TEST(test_message_parse_escape_unquoted)
TEST(test_message_parse_escape_quoted)
TEST(test_message_parse_sized)
TEST(test_message_parse_mixed)
TEST(test_message_parse_empty)
TEST(test_message_parse_multiple_spaces)
TEST(test_message_parse_missing_newline)
TEST(test_message_parse_sized_missing_newline)
END_TESTS
