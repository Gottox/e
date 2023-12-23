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

	rv = rope_append_str(
			&r,
			"Hello World\n"
			"This is a multiline string\n");
	assert(rv == 0);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r, 0, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c, 1, 9);
	assert(rv == 0);

	rv = rope_cursor_insert_str(&c, "n awesome");
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);
	assert(node != NULL);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 21);
	assert(memcmp(value, "Hello World\nThis is a", size) == 0);

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
	assert(size == 18);

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

	rv = rope_append_str(&r, u8"👋\n🦶\n");
	assert(rv == 0);

	struct RopeCursor c = {0};
	rv = rope_cursor_init(&c, &r, 0, NULL, NULL);
	assert(rv == 0);

	rv = rope_cursor_set(&c, 1, 0);
	assert(rv == 0);

	rv = rope_cursor_insert_str(&c, u8"🙂\n");
	assert(rv == 0);

	struct RopeNode *node = rope_first(&r);
	assert(node != NULL);
	size_t size = 0;
	const uint8_t *value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 5);
	assert(memcmp(value, u8"👋\n", size) == 0);

	bool has_next = rope_node_next(&node);
	assert(has_next == true);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 5);
	assert(memcmp(value, u8"🙂\n", size) == 0);

	has_next = rope_node_next(&node);
	assert(has_next != false);
	assert(node != NULL);
	value = rope_node_value(node, &size);
	assert(value != NULL);
	assert(size == 5);
	assert(memcmp(value, u8"🦶\n", size) == 0);

	rv = rope_cursor_cleanup(&c);
	assert(rv == 0);
	rv = rope_cleanup(&r);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(test_cursor)
TEST(test_cursor_utf8)
END_TESTS
