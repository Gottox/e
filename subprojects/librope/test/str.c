#include <rope_error.h>
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
	ASSERT_EQ(data, str.data.inplace);
	rope_str_cleanup(&str);
}

static void
test_str_split(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(0, rv);

	struct RopeStr new_str = {0};
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, 5);
	ASSERT_EQ(0, rv);

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
test_str_last_char_index(void) {
	struct RopeStr str = {0};
	int rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(rope_str_last_char_index(&str), 4);
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
	ASSERT_EQ(data, str.data.heap.data);
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
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, ROPE_STR_INLINE_SIZE);
	ASSERT_EQ(0, rv);

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
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, ROPE_STR_INLINE_SIZE + 1);
	ASSERT_EQ(0, rv);

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
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, ROPE_STR_INLINE_SIZE - 1);
	ASSERT_EQ(0, rv);

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
test_str_inline_append(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_insert(&str, 5, ROPE_BYTE, (const uint8_t *)"World", 5);
	ASSERT_EQ(rv, 0);
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 10);
	ASSERT_STREQS((const char *)data, "HelloWorld", 10);
	rope_str_cleanup(&str);
}

static void
test_str_inline_prepend(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"World", 5);
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_insert(&str, 0, ROPE_BYTE, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 10);
	ASSERT_STREQS((const char *)data, "HelloWorld", 10);
	rope_str_cleanup(&str);
}

static void
rope_str_strdup(void) {
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE * 2);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 2);
	ASSERT_NOT_NULL(buffer);
	rope_str_wrap(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 2);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ((uintptr_t)buffer, (uintptr_t)data);
	rope_str_cleanup(&str);
}

static void
rope_str_strdup_split_into_inline(void) {
	int rv = 0;
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE * 2);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 2);
	ASSERT_NOT_NULL(buffer);
	rope_str_wrap(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 2);

	struct RopeStr new_str = {0};
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, ROPE_STR_INLINE_SIZE);
	ASSERT_EQ(0, rv);
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE);
	ASSERT_NE((uintptr_t)buffer, (uintptr_t)data);

	rope_str_cleanup(&str);
	rope_str_cleanup(&new_str);
}

static void
rope_str_strdup_split_into_heap(void) {
	int rv = 0;
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE * 4);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 4);
	ASSERT_NOT_NULL(buffer);
	rope_str_wrap(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 4);

	struct RopeStr new_str = {0};
	rv = rope_str_split(&str, &new_str, ROPE_CHAR, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ(0, rv);
	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ((uintptr_t)buffer, (uintptr_t)data);
	ASSERT_NULL(str.data.heap.str);
	ASSERT_NOT_NULL(new_str.data.heap.str);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_utf16_split_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rv = rope_str_split(&str, &new_str, ROPE_UTF16, 5);
	ASSERT_EQ(0, rv);

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
test_str_utf16_split_after_surrogate(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "A" SMILING_FACE "B";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};

	rv = rope_str_split(&str, &new_str, ROPE_UTF16, 3);
	ASSERT_EQ(0, rv);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 5);
	ASSERT_STREQS((const char *)data, "A" SMILING_FACE, 5);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, 1);
	ASSERT_STREQS((const char *)data, "B", 1);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_cp_split_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rv = rope_str_split(&str, &new_str, ROPE_CP, 5);
	ASSERT_EQ(0, rv);

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
test_str_cp_split_multibyte(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "caf\xc3\xa9";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};

	rv = rope_str_split(&str, &new_str, ROPE_CP, 3);
	ASSERT_EQ(0, rv);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 3);
	ASSERT_STREQS((const char *)data, "caf", 3);

	data = rope_str_data(&new_str, &byte_size);
	ASSERT_EQ(byte_size, 2);
	ASSERT_STREQS((const char *)data, "\xc3\xa9", 2);

	rope_str_cleanup(&new_str);
	rope_str_cleanup(&str);
}

static void
test_str_move_heap(void) {
	int rv = 0;
	struct RopeStr src = {0};
	struct RopeStr dest = {0};

	uint8_t buffer[1024];
	memset(buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&src, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	size_t src_byte_size = 0;
	const uint8_t *src_data = rope_str_data(&src, &src_byte_size);
	ASSERT_EQ(src_byte_size, sizeof(buffer));

	rope_str_move(&dest, &src);

	size_t dest_byte_size = 0;
	const uint8_t *dest_data = rope_str_data(&dest, &dest_byte_size);
	ASSERT_EQ(dest_byte_size, sizeof(buffer));
	ASSERT_EQ((uintptr_t)src_data, (uintptr_t)dest_data);

	ASSERT_EQ(rope_str_dim(&src, ROPE_BYTE), 0);

	rope_str_cleanup(&dest);
}

static void
test_str_freeable_inline(void) {
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE);
	ASSERT_NOT_NULL(buffer);
	memset(buffer, 'X', ROPE_STR_INLINE_SIZE);

	rope_str_wrap(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE);

	ASSERT_EQ((uintptr_t)data, (uintptr_t)str.data.inplace);

	rope_str_cleanup(&str);
}

static void
test_str_inline_append_overflow(void) {
	int rv = 0;
	struct RopeStr str = {0};

	uint8_t buffer[ROPE_STR_INLINE_SIZE - 2];
	memset(buffer, 'A', sizeof(buffer));
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_insert(&str, SIZE_MAX, ROPE_BYTE, (const uint8_t *)"BB", 2);
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_insert(&str, SIZE_MAX, ROPE_BYTE, (const uint8_t *)"C", 1);
	ASSERT_EQ(rv, -ROPE_ERROR_OOB);

	rope_str_cleanup(&str);
}

static void
test_str_slow_str(void) {
	uint8_t buffer[8192];
	memset(buffer, 'A', sizeof(buffer));
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, buffer, sizeof(buffer));
	ASSERT_EQ(rv, 0);

	ASSERT_EQ(sizeof(buffer), rope_str_dim(&str, ROPE_BYTE));
	ASSERT_EQ(sizeof(buffer), rope_str_dim(&str, ROPE_CHAR));
	ASSERT_EQ(sizeof(buffer), rope_str_dim(&str, ROPE_UTF16));
	ASSERT_EQ(sizeof(buffer)-1, rope_str_last_char_index(&str));

	rope_str_cleanup(&str);
}

DECLARE_TESTS
TEST(test_str_init)
TEST(test_str_split)
TEST(test_str_last_char_index)
TEST(test_str_heap)
TEST(test_str_heap_split_to_inline_both)
TEST(test_str_heap_split_to_inline_right)
TEST(test_str_heap_split_to_inline_left)
TEST(test_str_inline_append)
TEST(test_str_inline_prepend)
TEST(rope_str_strdup)
TEST(rope_str_strdup_split_into_inline)
TEST(rope_str_strdup_split_into_heap)
TEST(test_str_utf16_split_ascii)
TEST(test_str_utf16_split_after_surrogate)
TEST(test_str_cp_split_ascii)
TEST(test_str_cp_split_multibyte)
TEST(test_str_move_heap)
TEST(test_str_freeable_inline)
TEST(test_str_inline_append_overflow)
TEST(test_str_slow_str)
END_TESTS
