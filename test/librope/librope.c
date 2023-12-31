#include <assert.h>
#include <rope.h>
#include <string.h>
#include <testlib.h>

void
test_librope_insert(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	assert(size == 5);
	assert(memcmp(data, "Hello", size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == false);
	assert(node == NULL);

	rv = rope_cleanup(&r);
}

void
test_librope_split_insert(void) {
	bool has_next = false;
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	rv = rope_insert(&r, 3, (uint8_t *)"lo Hel", 6);
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	assert(size == 3);
	assert(memcmp(data, "Hel", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == true);
	data = rope_node_value(node, &size);
	assert(size == 6);
	assert(memcmp(data, "lo Hel", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == true);
	data = rope_node_value(node, &size);
	assert(size == 2);
	assert(memcmp(data, "lo", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);
	assert(node == NULL);

	rv = rope_cleanup(&r);
}

void
test_librope_split_delete(void) {
	bool has_next = false;
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	rv = rope_delete(&r, 2, 2);
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	assert(size == 2);
	assert(memcmp(data, "He", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == true);
	data = rope_node_value(node, &size);
	assert(size == 1);
	assert(memcmp(data, "o", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);
	assert(node == NULL);

	rv = rope_cleanup(&r);
}

void
test_librope_tail_delete(void) {
	bool has_next = false;
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	rv = rope_delete(&r, 2, 3);
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	assert(size == 2);
	assert(memcmp(data, "He", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);

	rv = rope_cleanup(&r);
}

void
test_librope_head_delete(void) {
	bool has_next = false;
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	rv = rope_delete(&r, 0, 3);
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);

	size_t size = 0;
	const uint8_t *data;
	data = rope_node_value(node, &size);
	assert(size == 2);
	assert(memcmp(data, "lo", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);

	rv = rope_cleanup(&r);
}

static void
test_librope_insert_multiline(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"World,\nHow are you?\n", 20);
	assert(rv == 0);

	rv = rope_append(&r, (uint8_t *)"I am fine", 9);
	assert(rv == 0);

	rope_byte_index_t index = 0;
	struct RopeNode *node = rope_node_find(r.root, 0, 0, &index);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);

	assert(size == 12);
	assert(index == 0);
	assert(memcmp(&data[index], "HelloWorld,\n", size - index) == 0);

	index = 0;
	node = rope_node_find(r.root, 1, 0, &index);
	size = 0;
	data = rope_node_value(node, &size);

	assert(size == 13);
	assert(index == 0);
	assert(memcmp(&data[index], "How are you?\n", size - index) == 0);

	index = 0;
	node = rope_node_find(r.root, 2, 0, &index);
	size = 0;
	data = rope_node_value(node, &size);

	assert(size == 9);
	assert(index == 0);
	assert(memcmp(&data[index], "I am fine", size - index) == 0);

	rv = rope_cleanup(&r);
}

static void
test_librope_insert_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(&r, u8"Hello 👋 World");
	assert(rv == 0);

	rope_byte_index_t index = 0;
	struct RopeNode *node = rope_find(&r, 8, &index);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	assert(size == 16);
	assert(index == 11);
	assert(memcmp(&data[index], "World", size - index) == 0);

	rv = rope_cleanup(&r);
}

static void
test_librope_delete_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(&r, u8"Hello 👋 World");
	assert(rv == 0);

	rope_delete(&r, 6, 2);

	struct RopeNode *node = rope_first(&r);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	assert(memcmp(data, "Hello ", size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == true);
	data = rope_node_value(node, &size);
	assert(memcmp(data, "World", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);
	assert(node == NULL);
	rv = rope_cleanup(&r);
}

static void
test_librope_single_letter_insert(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	const char *first =
			u8"Hello WorldHello WorldHello WorldHello WorldHello WorldHello "
			u8"WorldHello WorldHello WorldHello World\n";
	rv = rope_append_str(&r, first);
	assert(rv == 0);

	rv = rope_append_str(&r, u8"1");
	assert(rv == 0);

	rv = rope_append_str(&r, u8"2");
	assert(rv == 0);

	rv = rope_append_str(&r, u8"3");
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);
	size_t size = 0;
	const uint8_t *data = rope_node_value(node, &size);
	assert(memcmp(data, first, size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == true);
	data = rope_node_value(node, &size);
	assert(memcmp(data, "123", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next == false);
	assert(node == NULL);
	rv = rope_cleanup(&r);
}

DECLARE_TESTS
TEST(test_librope_insert)
TEST(test_librope_split_insert)
TEST(test_librope_split_delete)
TEST(test_librope_head_delete)
TEST(test_librope_tail_delete)
TEST(test_librope_insert_multiline)
TEST(test_librope_insert_utf8)
TEST(test_librope_delete_utf8)
TEST(test_librope_single_letter_insert)
END_TESTS
