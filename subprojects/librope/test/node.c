#include <assert.h>
#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_node_insert() {
	int rv = 0;
	struct RopePool p = {0};
	rv = rope_pool_init(&p);
	ASSERT_EQ(0, rv);

	struct RopeNode *n = rope_node_new(&p);
	ASSERT_TRUE(NULL != n);
	rv = rope_node_set_value(n, (const uint8_t *)"hello", 5);
	ASSERT_EQ(0, rv);

	struct RopeNode *n2 = rope_node_new(&p);
	ASSERT_TRUE(NULL != n2);
	rv = rope_node_set_value(n2, (const uint8_t *)"world", 5);
	ASSERT_EQ(0, rv);

	rv = rope_node_insert_right(n, n2, &p);

	ASSERT_EQ(ROPE_NODE_BRANCH, (int)n->type);

	struct RopeNode *left = rope_node_left(n);
	size_t size;
	const uint8_t *value = rope_node_value(left, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "hello", 5));

	struct RopeNode *right = rope_node_right(n);
	value = rope_node_value(right, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "world", 5));

	rv = rope_pool_cleanup(&p);
}

static void test_node_split() {
	int rv = 0;
	size_t size = 0;
	const uint8_t *value = NULL;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_new(&pool);
	ASSERT_TRUE(NULL != node);
	rv = rope_node_set_value(node, (const uint8_t *)"helloworld", 10);
	ASSERT_EQ(0, rv);

	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;
	rv = rope_node_split(node, &pool, 5, &left, &right);

	ASSERT_TRUE(NULL != left);
	value = rope_node_value(left, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "hello", 5));
	ASSERT_TRUE(NULL != right);

	value = rope_node_value(right, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "world", 5));

	rv = rope_pool_cleanup(&pool);
}

void
check_balanced(struct RopeNode *node) {
	if (node->type == ROPE_NODE_BRANCH) {
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);

		printf("left: %zu, right: %zu\n", left->data.branch.leafs,
				right->data.branch.leafs);
		if (left->type == ROPE_NODE_BRANCH && right->type == ROPE_NODE_BRANCH) {
			ASSERT_EQ(left->data.branch.leafs, right->data.branch.leafs);
		}
	}
}

static void test_node_balanced_tree_right() {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);
	ASSERT_TRUE(NULL != root);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		ASSERT_TRUE(NULL != node);
		rv = rope_node_set_value(
				node, (const uint8_t *)"12345678901234567890123456789012",
				i + 1);
		ASSERT_EQ(0, rv);
		rv = rope_node_insert_right(root, node, &pool);
		ASSERT_EQ(0, rv);
	}

	ASSERT_EQ(ROPE_NODE_BRANCH, (int)root->type);

	check_balanced(root);

	rv = rope_pool_cleanup(&pool);
	ASSERT_EQ(0, rv);
}

static void test_node_balanced_tree_left() {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);
	ASSERT_TRUE(NULL != root);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		ASSERT_TRUE(NULL != node);
		rv = rope_node_set_value(
				node, (const uint8_t *)"12345678901234567890123456789012",
				i + 1);
		ASSERT_EQ(0, rv);
		rv = rope_node_insert_right(root, node, &pool);
		ASSERT_EQ(0, rv);
	}

	ASSERT_EQ(ROPE_NODE_BRANCH, (int)root->type);

	check_balanced(root);

	rv = rope_pool_cleanup(&pool);
	ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(test_node_insert)
TEST(test_node_split)
TEST(test_node_balanced_tree_right)
TEST(test_node_balanced_tree_left)
END_TESTS
