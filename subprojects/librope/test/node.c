#include "common.h"
#include <assert.h>
#include <rope.h>
#include <rope_node.h>
#include <rope_str.h>
#include <testlib.h>

static void
test_node_split_inline_middle(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'HELO'");

	struct RopeNode *left, *right;
	rv = rope_node_split(root, &pool, 2, &left, &right);
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
	struct RopePool pool = {0};

	// must be larger than ROPE_STR_INLINE_SIZE
	uint8_t women_emoji[] =
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6";

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(
			root, women_emoji, ROPE_STR_INLINE_SIZE + 2, 0, &pool);
	rv = rope_node_insert_right(
			root, &women_emoji[2], ROPE_STR_INLINE_SIZE + 2, 0, &pool);
	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));
	size_t size;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(size, ROPE_STR_INLINE_SIZE * 2 + 4);
	ASSERT_STREQS((const char *)women_emoji, (const char *)data, size);
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
END_TESTS
