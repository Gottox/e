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
	rope_node_set_tags(n, 1);
	rv = rope_node_set_value(n, (const uint8_t *)"hello", 5);
	ASSERT_EQ(0, rv);

	struct RopeNode *n2 = rope_node_new(&p);
	ASSERT_TRUE(NULL != n2);
	rope_node_set_tags(n, 2);
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

static void
test_node_split() {
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

static void
test_node_balanced_tree_right() {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);
	ASSERT_TRUE(NULL != root);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		// Set unique tags to avoid auto-merging nodes
		rope_node_set_tags(node, i);
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

static void
test_node_balanced_tree_left() {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);
	ASSERT_TRUE(NULL != root);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		// Set unique tags to avoid auto-merging nodes
		rope_node_set_tags(node, i);
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

static void
test_node_merge() {
	int rv = 0;
	struct RopePool p = {0};
	rv = rope_pool_init(&p);
	ASSERT_EQ(0, rv);

	struct RopeNode *n = rope_node_new(&p);
	ASSERT_TRUE(NULL != n);
	rope_node_set_tags(n, 1);
	rv = rope_node_set_value(n, (const uint8_t *)"world", 5);
	ASSERT_EQ(0, rv);

	struct RopeNode *n2 = rope_node_new(&p);
	ASSERT_TRUE(NULL != n2);
	rope_node_set_tags(n, 2);
	rv = rope_node_set_value(n2, (const uint8_t *)"hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_node_insert_left(n, n2, &p);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(ROPE_NODE_BRANCH, (int)n->type);

	struct RopeNode *to_merge = rope_node_left(n);
	rv = rope_node_merge(to_merge, ROPE_NODE_RIGHT, &p);
	ASSERT_GT(0, rv);

	rope_node_set_tags(n, 1);
	rv = rope_node_merge(to_merge, ROPE_NODE_RIGHT, &p);
	ASSERT_EQ(0, rv);

	size_t size;
	const uint8_t *value = rope_node_value(n, &size);
	ASSERT_EQ((size_t)10, size);
	ASSERT_EQ(0, memcmp(value, "helloworld", 10));

	rv = rope_pool_cleanup(&p);
	ASSERT_EQ(0, rv);
}

static void
test_node_tags() {
	const uint64_t TAG_RED = 1 << 0;
	const uint64_t TAG_GREEN = 1 << 1;
	const uint64_t TAG_BLUE = 1 << 2;

	int rv = 0;
	struct RopePool p = {0};
	rv = rope_pool_init(&p);
	ASSERT_EQ(0, rv);

	struct RopeNode *red_node = rope_node_new(&p);
	ASSERT_NE(NULL, red_node);
	rope_node_set_tags(red_node, TAG_RED);
	rv = rope_node_set_value(red_node, (const uint8_t *)"red\n", 4);
	ASSERT_EQ(0, rv);

	struct RopeNode *green_node = rope_node_new(&p);
	ASSERT_NE(NULL, green_node);
	rope_node_set_tags(green_node, TAG_GREEN);
	rv = rope_node_set_value(green_node, (const uint8_t *)"green\n", 6);
	ASSERT_EQ(0, rv);

	struct RopeNode *blue_node = rope_node_new(&p);
	ASSERT_NE(NULL, blue_node);
	rope_node_set_tags(blue_node, TAG_BLUE);
	rv = rope_node_set_value(blue_node, (const uint8_t *)"blue\n", 5);
	ASSERT_EQ(0, rv);

	rv = rope_node_insert_right(red_node, green_node, &p);
	ASSERT_EQ(0, rv);
	rv = rope_node_insert_right(green_node, blue_node, &p);
	ASSERT_EQ(0, rv);

	rope_byte_index_t byte_index = 0;
	struct RopeNode *node;
	const uint8_t *value;
	size_t size;

	node = rope_node_find_char(red_node, 0, TAG_RED, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, "red\n", size));
	ASSERT_EQ(0, byte_index);

	node = rope_node_find_char(red_node, 0, TAG_GREEN, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(value, "green\n", size));
	ASSERT_EQ(0, byte_index);

	node = rope_node_find_char(red_node, 0, TAG_BLUE, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "blue\n", size));
	ASSERT_EQ(0, byte_index);

	node = rope_node_find_char(red_node, 0, 0, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(value, "red\n", size));
	ASSERT_EQ(0, byte_index);

	node = rope_node_find_char(red_node, 5, 0, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(value, "green\n", size));
	ASSERT_EQ(1, byte_index);

	node = rope_node_find_char(red_node, 11, 0, &byte_index);
	value = rope_node_value(node, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(value, "blue\n", size));
	ASSERT_EQ(1, byte_index);


	rv = rope_pool_recycle(&p, red_node);
	ASSERT_EQ(0, rv);
	rv = rope_pool_recycle(&p, green_node);
	ASSERT_EQ(0, rv);
	rv = rope_pool_recycle(&p, blue_node);
	ASSERT_EQ(0, rv);

	rv = rope_pool_cleanup(&p);
	ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(test_node_insert)
TEST(test_node_split)
TEST(test_node_balanced_tree_right)
TEST(test_node_balanced_tree_left)
TEST(test_node_merge)
TEST(test_node_tags)
END_TESTS
