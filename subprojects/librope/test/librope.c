#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_librope_insert(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ(0, memcmp(data, "Hello", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);

	rv = rope_cleanup(&r);
}

static void
test_librope_split_insert(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_insert(&r, 3, (uint8_t *)"lo Hel", 6);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)9, size);
	ASSERT_EQ(0, memcmp(data, "Hello Hel", size));

	// size_t size = 0;
	// const uint8_t *data;
	// data = rope_node_value(node, &size);
	// ASSERT_EQ((size_t)3, size);
	// ASSERT_EQ(0, memcmp(data, "Hel", size));

	// data = rope_node_value(node, &size);
	// ASSERT_EQ((size_t)6, size);
	// ASSERT_EQ(0, memcmp(data, "lo Hel", size));

	node = rope_node_next(node);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "lo", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);

	rv = rope_cleanup(&r);
}

static void
test_librope_split_delete(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, 2, 2);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);

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

	rv = rope_cleanup(&r);
}

static void
test_librope_tail_delete(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, 2, 3);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "He", size));

	node = rope_node_next(node);

	rv = rope_cleanup(&r);
}

static void
test_librope_head_delete(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_delete(&r, 0, 3);
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "lo", size));

	node = rope_node_next(node);

	rv = rope_cleanup(&r);
}

static void
test_librope_insert_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"Hello 👋 World");
	ASSERT_EQ(0, rv);

	rope_byte_index_t index = 0;
	struct RopeNode *node = rope_find(&r, 8, &index);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)16, size);
	ASSERT_EQ((size_t)11, index);
	ASSERT_EQ(0, memcmp(&data[index], "World", size - index));

	rv = rope_cleanup(&r);
}

static void
test_librope_delete_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"Hello 👋 World");
	ASSERT_EQ(0, rv);

	rope_delete(&r, 6, 2);

	struct RopeNode *node = rope_first(&r);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "Hello ", size));

	node = rope_node_next(node);
	data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "World", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);
	rv = rope_cleanup(&r);
}

static void
test_librope_single_letter_insert(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	const char *first =
			u8"Hello WorldHello WorldHello WorldHello WorldHello WorldHello "
			u8"WorldHello WorldHello WorldHello World\n";
	rv = rope_append_str(&r, first);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"1");
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"2");
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, u8"3");
	ASSERT_EQ(0, rv);

	struct RopeNode *node = rope_first(&r);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, first, size));

	node = rope_node_next(node);
	data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "123", size));

	node = rope_node_next(node);
	ASSERT_TRUE(NULL == node);
	rv = rope_cleanup(&r);
}

DECLARE_TESTS
TEST(test_librope_insert)
TEST(test_librope_split_insert)
TEST(test_librope_split_delete)
TEST(test_librope_tail_delete)
TEST(test_librope_head_delete)
TEST(test_librope_insert_utf8)
TEST(test_librope_delete_utf8)
TEST(test_librope_single_letter_insert)
END_TESTS
