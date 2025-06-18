#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_librope_insert() {
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

	bool has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);
	ASSERT_TRUE(NULL == node);

	rv = rope_cleanup(&r);
}

static void
test_librope_split_insert() {
	bool has_next = false;
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
	ASSERT_EQ((size_t)3, size);
	ASSERT_EQ(0, memcmp(data, "Hel", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "lo Hel", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "lo", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);
	ASSERT_TRUE(NULL == node);

	rv = rope_cleanup(&r);
}

static void
test_librope_split_delete() {
	bool has_next = false;
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

	has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	data = rope_node_value(node, &size);
	ASSERT_EQ((size_t)1, size);
	ASSERT_EQ(0, memcmp(data, "o", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);
	ASSERT_TRUE(NULL == node);

	rv = rope_cleanup(&r);
}

static void
test_librope_tail_delete() {
	bool has_next = false;
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

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);

	rv = rope_cleanup(&r);
}

static void
test_librope_head_delete() {
	bool has_next = false;
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

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);

	rv = rope_cleanup(&r);
}

static void
test_librope_insert_multiline() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"World,\nHow are you?\n", 20);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"I am fine", 9);
	ASSERT_EQ(0, rv);

	rope_byte_index_t index = 0;
	struct RopeNode *node = rope_node_find(r.root, 0, 0, &index);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);

	ASSERT_EQ((size_t)5, size);
	ASSERT_EQ((size_t)0, index);
	ASSERT_EQ(0, memcmp(&data[index], "Hello", size - index));

	index = 0;
	node = rope_node_find(r.root, 1, 0, &index);
	size = 0;
	data = rope_node_value(node, &size);

	ASSERT_EQ((size_t)20, size);
	ASSERT_EQ((size_t)7, index);
	ASSERT_EQ(0, memcmp(&data[index], "How are you?\n", size - index));

	index = 0;
	node = rope_node_find(r.root, 2, 0, &index);
	size = 0;
	data = rope_node_value(node, &size);

	ASSERT_EQ((size_t)9, size);
	ASSERT_EQ((size_t)0, index);
	ASSERT_EQ(0, memcmp(&data[index], "I am fine", size - index));

	rv = rope_cleanup(&r);
}

static void
test_librope_insert_utf8() {
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
test_librope_delete_utf8() {
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

	bool has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "World", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);
	ASSERT_TRUE(NULL == node);
	rv = rope_cleanup(&r);
}

static void
test_librope_single_letter_insert() {
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

	bool has_next = rope_node_next(&node);
	ASSERT_EQ(true, has_next);
	data = rope_node_value(node, &size);
	ASSERT_EQ(0, memcmp(data, "123", size));

	has_next = rope_node_next(&node);
	ASSERT_EQ(false, has_next);
	ASSERT_TRUE(NULL == node);
	rv = rope_cleanup(&r);
}

DECLARE_TESTS
TEST(test_librope_insert)
TEST(test_librope_split_insert)
TEST(test_librope_split_delete)
TEST(test_librope_tail_delete)
TEST(test_librope_head_delete)
TEST(test_librope_insert_multiline)
TEST(test_librope_insert_utf8)
TEST(test_librope_delete_utf8)
TEST(test_librope_single_letter_insert)
END_TESTS
