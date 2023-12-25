#include <assert.h>
#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_cursor(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(&r, "This is a string");
	assert(rv == 0);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c, 0, 9);
	assert(rv == 0);

	rv = rope_cursor_insert_str(&c, "n awesome");
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);
	assert(node != NULL);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 9);
	assert(memcmp(value, "This is a", size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == true);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 9);
	assert(memcmp(value, "n awesome", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next != false);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 7);
	assert(memcmp(value, " string", size) == 0);

	rv = rope_cursor_cleanup(&c);
	assert(rv == 0);
	rv = rope_cleanup(&r);
	assert(rv == 0);
}

static void
test_cursor_utf8(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(&r, u8"👋🦶");
	assert(rv == 0);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c, 0, 1);
	assert(rv == 0);

	rv = rope_cursor_insert_str(&c, u8"🙂");
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);
	assert(node != NULL);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 4);
	assert(memcmp(value, u8"👋", size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == true);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 4);
	assert(memcmp(value, u8"🙂", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next != false);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 4);
	assert(memcmp(value, u8"🦶", size) == 0);

	rv = rope_cursor_cleanup(&c);
	assert(rv == 0);
	rv = rope_cleanup(&r);
	assert(rv == 0);
}

static void
test_cursor_event(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(&r, u8"This is a test");
	assert(rv == 0);

	struct RopeCursor c1 = {0};
	rv = rope_cursor_init(&c1, &r, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c1, 0, 10);
	assert(rv == 0);

	struct RopeCursor c2 = {0};
	rv = rope_cursor_init(&c2, &r, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c2, 0, 0);
	assert(rv == 0);

	rv = rope_cursor_delete(&c2, 7);
	assert(rv == 0);

	assert(c1.index == 3);

	rv = rope_cursor_cleanup(&c1);
	assert(rv == 0);
	rv = rope_cursor_cleanup(&c2);
	assert(rv == 0);
	rv = rope_cleanup(&r);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(test_cursor)
TEST(test_cursor_utf8)
TEST(test_cursor_event)
END_TESTS
