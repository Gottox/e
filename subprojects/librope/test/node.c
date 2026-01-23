#include "common.h"
#include <assert.h>
#include <grapheme.h>
#include <rope.h>
#include <rope_node.h>
#include <rope_str.h>
#include <rope_util.h>
#include <stdbool.h>
#include <testlib.h>

static void
chechk_valid_utf8(struct RopeNode *node) {
	node = rope_node_first(node);
	do {
		size_t byte_size = 0;
		const uint8_t *data = rope_node_value(node, &byte_size);
		for (size_t i = 0; i < byte_size;) {
			uint_least32_t cp = 0;
			grapheme_decode_utf8((const char *)&data[i], byte_size - i, &cp);
			ASSERT_NE(GRAPHEME_INVALID_CODEPOINT, cp);
			size_t char_size = grapheme_next_character_break_utf8(
					(const char *)&data[i], byte_size - i);
			i += char_size;
		}
	} while ((node = rope_node_next(node)) != NULL);
}

static void
test_node_split_inline_middle(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'HELO'");

	struct RopeNode *left, *right;
	rv = rope_node_split(root, &pool, 2, ROPE_BYTE, &left, &right);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['HE','LO']", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'HE'");

	rv = rope_node_insert_right(root, (const uint8_t *)"LO", 2, 0xFF, &pool);

	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));
	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	ASSERT_EQ(root, rope_node_parent(right));
	ASSERT_EQ(root, rope_node_parent(left));

	ASSERT_JSONEQ("['HE','LO']", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['HE','LO']");

	rope_node_delete(rope_node_left(root), &pool);

	ASSERT_JSONEQ("'LO'", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_rotate_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['HE',['LL','O']]");

	rope_node_rotate(root, ROPE_LEFT);

	ASSERT_JSONEQ("[['HE','LL'],'O']", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_balance_left(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "[[[['H','E'],'L'],'L'],'O']");

	struct RopeNode *hel_node = rope_node_left(root);
	hel_node = rope_node_left(hel_node);

	rope_node_balance_up(hel_node);

	ASSERT_JSONEQ("[['H','E'],[['L','L'],'O']]", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_balance_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['H',['E',['L',['L','O']]]]");

	struct RopeNode *lo_node = rope_node_right(root);
	lo_node = rope_node_right(lo_node);

	rope_node_balance_up(lo_node);

	ASSERT_JSONEQ("[['H',['E','L']],['L','O']]", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

void
test_node_merge(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['1',['2',['3','4']]]");

	struct RopeNode *node = rope_node_first(root);

	rv = rope_node_merge(node, 2, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['123','4']", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}
static void
test_node_insert_incomplete_utf8(void) {
	int rv = 0;

	// must be larger than ROPE_STR_INLINE_SIZE
	uint8_t women_emoji[] =
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6";

	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(
			root, women_emoji, ROPE_STR_INLINE_SIZE + 2, 0, &pool);
	rv = rope_node_insert_right(
			root, &women_emoji[2], ROPE_STR_INLINE_SIZE + 2, 0, &pool);

	chechk_valid_utf8(root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_right_malloc(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	uint8_t *buffer = malloc(ROPE_STR_INLINE_SIZE * 2);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 2);
	ASSERT_NOT_NULL(buffer);

	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_heap_right(
			root, buffer, ROPE_STR_INLINE_SIZE * 2, 0, &pool);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *data = rope_node_value(root, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ((uintptr_t)buffer, (uintptr_t)data);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_big(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	uint8_t buffer[65536] = {0};
	memset(buffer, 'A', sizeof(buffer));

	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_right(root, buffer, sizeof(buffer), 0, &pool);
	ASSERT_EQ(rv, 0);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_stitch_overflow_leaf_size(void) {
	int rv = 0;

	char smiley[] = "\xf0\x9f\x98\x83";
	char buffer1[ROPE_STR_FAST_SIZE] = {0};
	memset(buffer1, 'A', sizeof(buffer1));
	char buffer2[ROPE_STR_FAST_SIZE] = {0};
	memset(buffer2, 'B', sizeof(buffer2));

	memcpy(&buffer1[ROPE_STR_FAST_SIZE - 2], smiley, 2);
	memcpy(buffer2, &smiley[2], 2);

	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer1, sizeof(buffer1), 0, &pool);
	ASSERT_EQ(rv, 0);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer2, sizeof(buffer2), 0, &pool);
	ASSERT_EQ(rv, 0);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_insert_grapheme(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(
			root, (const uint8_t *)"e\xCC\x8A\xCC\x8A", 5, 0, &pool);
	ASSERT_EQ(0, rv);
	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(size, 5);
	ASSERT_STREQ("e\xCC\x8A\xCC\x8A", (const char *)data);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_insert_large_grapheme(void) {
	char buffer[ROPE_STR_FAST_SIZE * 2] = {0};
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	buffer[0] = 'e';
	const uint8_t diaeresis[] = {0xCC, 0x8A};
	const size_t d_size = sizeof(diaeresis);
	size_t size = 1;
	for (; size < sizeof(buffer) - d_size; size += d_size) {
		memcpy(&buffer[size], diaeresis, d_size);
	}

	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(root, (const uint8_t *)buffer, size, 0, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_node_insert_right(root, (const uint8_t *)buffer, size, 0, &pool);
	ASSERT_EQ(0, rv);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_utf8_counter(void) {
	struct RopeUtf8Counter counter = {0};
	const char *str = "e\xCC\x8A\xCC\x8A"; // e with two ring above
	size_t first_break;
	first_break =
			rope_utf8_char_break(&counter, (const uint8_t *)str, strlen(str));
	ASSERT_EQ(first_break, 0);
	first_break =
			rope_utf8_char_break(&counter, (const uint8_t *)str, strlen(str));
	ASSERT_EQ(first_break, 0);
}

static void
test_unbreak_utf8_sequence(void) {
	const char *str = "e\xCC\x8A\xCC\x8A";
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['dummy','dummy']");
	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	rv = rope_str_init(&left->data.leaf, (const uint8_t *)str, 2);
	ASSERT_EQ(0, rv);
	rv = rope_str_init(&right->data.leaf, (const uint8_t *)&str[3], 2);
	ASSERT_EQ(0, rv);

	rv = rope_node_insert_right(left, (const uint8_t *)&str[2], 1, 0, &pool);
	ASSERT_EQ(0, rv);

	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(size, 5);
	ASSERT_STREQ("e\xCC\x8A\xCC\x8A", (const char *)data);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}
DECLARE_TESTS
TEST(test_node_split_inline_middle)
TEST(test_node_insert_right)
TEST(test_node_delete)
TEST(test_node_rotate_right)
TEST(test_node_balance_left)
TEST(test_node_balance_right)
TEST(test_node_merge)
TEST(test_node_insert_incomplete_utf8)
TEST(test_node_insert_right_malloc)
TEST(test_node_insert_big)
TEST(test_node_stitch_overflow_leaf_size)
TEST(test_insert_grapheme)
TEST(test_insert_large_grapheme)
TEST(test_utf8_counter)
TEST(test_unbreak_utf8_sequence)
END_TESTS
