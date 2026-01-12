#include "common.h"
#include "rope_node.h"
#include <assert.h>
#include <ctype.h>
#include <rope.h>
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

	assert_json("['HE','LO']", root);

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
	struct RopeNode *new_node = from_str(&pool, "'LO'");

	rv = rope_node_insert(root, new_node, &pool, ROPE_RIGHT);

	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));
	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	ASSERT_EQ(new_node, right);
	ASSERT_EQ(root, rope_node_parent(right));
	ASSERT_EQ(root, rope_node_parent(left));

	assert_json("['HE','LO']", root);

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

	assert_json("'LO'", root);

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

	assert_json("[['HE','LL'],'O']", root);

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

	assert_json("[['H','E'],[['L','L'],'O']]", root);

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

	assert_json("[['H',['E','L']],['L','O']]", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static bool
node_delete_while_cb(const struct RopeNode *node, void *userdata) {
	(void)userdata;
	size_t size;
	const uint8_t *data = rope_node_value(node, &size);
	return isupper(data[0]);
}

static void
test_node_delete_while_left(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "[[['H','E'],['L','L']],'o']");

	struct RopeNode *node = rope_node_first(root);
	node = rope_node_next(node);


	rope_node_delete_while(node, &pool, node_delete_while_cb, NULL);

	assert_json("['H','o']", root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

void
test_node_delete_crash1(void) {

}


DECLARE_TESTS
TEST(test_node_split_inline_middle)
TEST(test_node_insert_right)
TEST(test_node_delete)
TEST(test_node_rotate_right)
TEST(test_node_balance_left)
TEST(test_node_balance_right)
TEST(test_node_delete_while_left)
TEST(test_node_delete_crash1)
END_TESTS
