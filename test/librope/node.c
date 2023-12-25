#include <assert.h>
#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_node_insert(void) {
	int rv = 0;
	struct RopePool p = {0};
	rv = rope_pool_init(&p);
	assert(rv == 0);

	struct RopeNode *n = rope_node_new(&p);
	assert(n != NULL);
	rv = rope_node_set_value(n, (const uint8_t *)"hello", 5);
	assert(rv == 0);

	struct RopeNode *n2 = rope_node_new(&p);
	assert(n2 != NULL);
	rv = rope_node_set_value(n2, (const uint8_t *)"world", 5);
	assert(rv == 0);

	rv = rope_node_insert_right(n, n2, &p);

	assert(n->type == ROPE_NODE_BRANCH);

	struct RopeNode *left = rope_node_left(n);
	size_t size;
	const uint8_t *value = rope_node_value(left, &size);
	assert(size == 5);
	assert(memcmp(value, "hello", 5) == 0);

	struct RopeNode *right = rope_node_right(n);
	value = rope_node_value(right, &size);
	assert(size == 5);
	assert(memcmp(value, "world", 5) == 0);

	rv = rope_pool_cleanup(&p);
}

static void
test_node_split(void) {
	int rv = 0;
	size_t size = 0;
	const uint8_t *value = NULL;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	assert(rv == 0);

	struct RopeNode *node = rope_node_new(&pool);
	assert(node != NULL);
	rv = rope_node_set_value(node, (const uint8_t *)"helloworld", 10);
	assert(rv == 0);

	struct RopeNode *left = NULL;
	struct RopeNode *right = NULL;
	rv = rope_node_split(node, &pool, 5, &left, &right);

	assert(left != NULL);
	value = rope_node_value(left, &size);
	assert(size == 5);
	assert(memcmp(value, "hello", 5) == 0);
	assert(right != NULL);

	value = rope_node_value(right, &size);
	assert(size == 5);
	assert(memcmp(value, "world", 5) == 0);

	rv = rope_pool_cleanup(&pool);
}

static void
check_balanced(struct RopeNode *node) {
	if (node->type == ROPE_NODE_BRANCH) {
		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);

		assert(left->data.branch.leafs == right->data.branch.leafs);
		check_balanced(left);
		check_balanced(right);
	}
}

static void
test_node_balanced_tree_right(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	assert(rv == 0);

	struct RopeNode *root = rope_node_new(&pool);
	assert(root != NULL);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		assert(node != NULL);
		rv = rope_node_set_value(
				node, (const uint8_t *)"12345678901234567890123456789012",
				i + 1);
		assert(rv == 0);
		rv = rope_node_insert_right(root, node, &pool);
		assert(rv == 0);
	}

	assert(root->type == ROPE_NODE_BRANCH);

	check_balanced(root);

	rv = rope_pool_cleanup(&pool);
	assert(rv == 0);
}

static void
test_node_balanced_tree_left(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	assert(rv == 0);

	struct RopeNode *root = rope_node_new(&pool);
	assert(root != NULL);

	for (int i = 0; i < 32; i++) {
		struct RopeNode *node = rope_node_new(&pool);
		assert(node != NULL);
		rv = rope_node_set_value(
				node, (const uint8_t *)"12345678901234567890123456789012",
				i + 1);
		assert(rv == 0);
		rv = rope_node_insert_right(root, node, &pool);
		assert(rv == 0);
	}

	assert(root->type == ROPE_NODE_BRANCH);

	check_balanced(root);

	rv = rope_pool_cleanup(&pool);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(test_node_insert)
TEST(test_node_split)
TEST(test_node_balanced_tree_left)
TEST(test_node_balanced_tree_right)
END_TESTS
