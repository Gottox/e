#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_librope_insert(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);

	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(data, "Hello", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_librope_split_insert(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_insert(&r, ROPE_CHAR, 3, (uint8_t *)"lo Hel", 6);
	ASSERT_EQ(0, rv);

	size_t size = rope_size(&r, ROPE_BYTE);
	char *data = rope_to_str(&r, 0);
	ASSERT_EQ((size_t)11, size);
	ASSERT_EQ(0, memcmp(data, "Hello Hello", size));
	free(data);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_librope_split_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, ROPE_CHAR, 2, 2);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "He", size));

	node = rope_node_next(node);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)1, size);
	ASSERT_EQ(0, memcmp(data, "o", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_librope_tail_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, ROPE_CHAR, 2, 3);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "He", size));

	node = rope_node_next(node);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_librope_head_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, ROPE_CHAR, 0, 3);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_node_first(r.root);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "lo", size));

	node = rope_node_next(node);

	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

static void
test_librope_delete_utf8(void) {
	int rv = 0;
	struct RopePool pool = {0};
	struct Rope r = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	rv = rope_init(&r, &pool);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"Hello 👋 World");
	ASSERT_EQ(0, rv);

	rope_delete(&r, ROPE_CHAR, 6, 2);

	struct RopeNode *node = rope_node_first(r.root);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "Hello ", size));

	node = rope_node_next(node);
	data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "World", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);
	rope_cleanup(&r);
	rope_pool_cleanup(&pool);
}

DECLARE_TESTS
TEST(test_librope_insert)
TEST(test_librope_split_insert)
TEST(test_librope_split_delete)
TEST(test_librope_tail_delete)
TEST(test_librope_head_delete)
TEST(test_librope_delete_utf8)
END_TESTS
