#include <rope_str.h>
#include <string.h>
#include <testlib.h>

#define FEMALE_ASTRONAUT "\360\237\221\251\342\200\215\360\237\232\200"
#define SMILING_FACE "\360\237\230\203"
#define LONG_GRAPHEME \
	"\xf0\x9f\x91\xa9\xf0\x9f\x8f\xbb\xe2\x80\x8d\xf0\x9f\xa4\x9d\xe2\x80\x8d" \
	"\xf0\x9f\x91\xa9\xf0\x9f\x8f\xbe\x0a"

static void
test_str_init(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 10);
	ASSERT_EQ(data, str.u.i.data);
	rope_str_cleanup(&str);
}

static void
test_str_split(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, 5);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 5);
	ASSERT_STREQS((const char *)data, "Hello", 5);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, 5);
	ASSERT_STREQS((const char *)data, "World", 5);
	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_heap(void) {
	int rv = 0;
	struct RopeStr str = {0};
	uint8_t buffer[1024];
	memset(&buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, sizeof(buffer));
	ASSERT_EQ(data, str.u.h.data);
	rope_str_cleanup(&str);
}

static void
test_str_heap_split_to_inline_both(void) {
	int rv = 0;
	struct RopeStr str = {0};
	uint8_t buffer[ROPE_STR_INLINE_SIZE * 2];
	memset(&buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, ROPE_STR_INLINE_SIZE);

	size_t byte_size = 0;

	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_heap_split_to_inline_right(void) {
	int rv = 0;
	struct RopeStr str = {0};
	uint8_t buffer[ROPE_STR_INLINE_SIZE * 2];
	memset(&buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, ROPE_STR_INLINE_SIZE + 1);

	size_t byte_size = 0;

	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE + 1);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE + 1);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE - 1);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE - 1);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_heap_split_to_inline_left(void) {
	int rv = 0;
	struct RopeStr str = {0};
	uint8_t buffer[ROPE_STR_INLINE_SIZE * 2];
	memset(&buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, ROPE_STR_INLINE_SIZE - 1);

	size_t byte_size = 0;

	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE - 1);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE - 1);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE + 1);
	ASSERT_STREQS(
			(const char *)data, (const char *)buffer, ROPE_STR_INLINE_SIZE + 1);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_should_merge_utf8_sequence(void) {
	int rv = 0;
	struct RopeStr str1 = {0}, str2 = {0};
	const uint8_t buffer[] = SMILING_FACE;
	rv = rope_str_init(&str1, buffer, 2);
	ASSERT_EQ(rv, 0);

	size_t byte_size = rope_str_byte_count(&str1);
	ASSERT_EQ(2, byte_size);
	ASSERT_EQ(1, rope_str_char_count(&str1));

	rv = rope_str_init(&str2, &buffer[2], 2);
	ASSERT_EQ(rv, 0);

	byte_size = rope_str_byte_count(&str2);
	ASSERT_EQ(2, byte_size);
	ASSERT_EQ(2, rope_str_char_count(&str2));

	bool should_merge = rope_str_should_merge(&str1, &str2);
	ASSERT_TRUE(should_merge);

	rope_str_cleanup(&str1);
	rope_str_cleanup(&str2);
}

static void
test_str_should_merge_grapheme(void) {
	const int split = 4;
	int rv = 0;
	struct RopeStr str1 = {0}, str2 = {0};

	const uint8_t buffer[] = FEMALE_ASTRONAUT;
	rv = rope_str_init(&str1, buffer, split);
	ASSERT_EQ(rv, 0);

	rv = rope_str_init(&str2, &buffer[split], sizeof(buffer) - 1 - split);
	ASSERT_EQ(rv, 0);

	bool should_merge = rope_str_should_merge(&str1, &str2);
	ASSERT_TRUE(should_merge);

	rope_str_cleanup(&str1);
	rope_str_cleanup(&str2);
}

static void
test_str_should_merge_at_fn(int split) {
	int rv = 0;

	const uint8_t buffer[] = LONG_GRAPHEME;

	struct RopeStr str1 = {0}, str2 = {0};

	rv = rope_str_init(&str1, buffer, split);
	ASSERT_EQ(rv, 0);

	rv = rope_str_init(&str2, &buffer[split], sizeof(buffer) - 1 - split);
	ASSERT_EQ(rv, 0);

	bool should_merge = rope_str_should_merge(&str1, &str2);
	ASSERT_TRUE(should_merge);

	rope_str_cleanup(&str1);
	rope_str_cleanup(&str2);
}

static void
test_str_should_merge_at_8(void) {
	test_str_should_merge_at_fn(8);
}

static void
test_str_should_merge_at_12(void) {
	test_str_should_merge_at_fn(12);
}

static void
test_str_should_merge_sliding(void) {
	for (size_t split = 1; split < sizeof(LONG_GRAPHEME) - 2; split++) {
		test_str_should_merge_at_fn(split);
	}
}

static void
test_str_inline_append(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_append(&str, (const uint8_t *)"World", 5);
	ASSERT_EQ(rv, 0);
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 10);
	ASSERT_STREQS((const char *)data, "HelloWorld", 10);
	rope_str_cleanup(&str);
}

static void
test_char_to_byte_index(void) {
	ASSERT_EQ(rope_str_char_to_byte_index((const uint8_t *)"😃", 4, 0), 0);
}

DECLARE_TESTS
TEST(test_str_init)
TEST(test_str_split)
TEST(test_str_heap)
TEST(test_str_should_merge_utf8_sequence)
TEST(test_str_should_merge_grapheme)
TEST(test_str_should_merge_sliding)
TEST(test_str_should_merge_at_8)
TEST(test_str_should_merge_at_12)
TEST(test_str_heap_split_to_inline_both)
TEST(test_str_heap_split_to_inline_right)
TEST(test_str_heap_split_to_inline_left)
TEST(test_str_inline_append)
TEST(test_char_to_byte_index)
END_TESTS
