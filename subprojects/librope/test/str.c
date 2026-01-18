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

	size_t byte_size = rope_str_bytes(&str1);
	ASSERT_EQ(2, byte_size);
	ASSERT_EQ(1, rope_str_chars(&str1));

	rv = rope_str_init(&str2, &buffer[2], 2);
	ASSERT_EQ(rv, 0);

	byte_size = rope_str_bytes(&str2);
	ASSERT_EQ(2, byte_size);
	ASSERT_EQ(2, rope_str_chars(&str2));

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

static void
rope_str_strdup(void) {
	int rv = 0;
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE * 2);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 2);
	ASSERT_NOT_NULL(buffer);
	rv = rope_str_freeable(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ(rv, 0);

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
	rv = rope_str_freeable(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, ROPE_STR_INLINE_SIZE);
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
	rv = rope_str_freeable(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE * 4);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_char_split(&str, &new_str, ROPE_STR_INLINE_SIZE * 2);
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
test_str_at_char_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	size_t byte_count = 0;
	const uint8_t *ptr = rope_str_at_char(&str, 2, &byte_count);
	ASSERT_EQ(byte_count, 3);
	ASSERT_STREQS((const char *)ptr, "llo", 3);

	rope_str_cleanup(&str);
}

static void
test_str_at_char_emoji(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "X" FEMALE_ASTRONAUT "Y";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	size_t byte_count = 0;
	const uint8_t *ptr = rope_str_at_char(&str, 1, &byte_count);
	ASSERT_EQ(byte_count, 12);
	ASSERT_STREQS((const char *)ptr, FEMALE_ASTRONAUT "Y", 12);

	rope_str_cleanup(&str);
}

static void
test_str_at_char_boundary(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"ABC", 3);
	ASSERT_EQ(rv, 0);

	size_t byte_count = 0;

	const uint8_t *ptr = rope_str_at_char(&str, 3, &byte_count);
	ASSERT_EQ(byte_count, 0);
	(void)ptr;

	rope_str_cleanup(&str);
}

static void
test_str_at_utf16_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	size_t byte_count = 0;
	const uint8_t *ptr = rope_str_at_utf16(&str, 2, &byte_count);
	ASSERT_EQ(byte_count, 3);
	ASSERT_STREQS((const char *)ptr, "llo", 3);

	rope_str_cleanup(&str);
}

static void
test_str_at_utf16_with_surrogates(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "A" SMILING_FACE "B";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	size_t byte_count = 0;

	const uint8_t *ptr = rope_str_at_utf16(&str, 3, &byte_count);
	ASSERT_EQ(byte_count, 1);
	ASSERT_STREQS((const char *)ptr, "B", 1);

	rope_str_cleanup(&str);
}

static void
test_str_at_cp_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *ptr = rope_str_at_cp(&str, 2, &byte_size);
	ASSERT_EQ(byte_size, 3);
	ASSERT_STREQS((const char *)ptr, "llo", 3);

	rope_str_cleanup(&str);
}

static void
test_str_at_cp_multibyte(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "caf\xc3\xa9";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;

	const uint8_t *ptr = rope_str_at_cp(&str, 3, &byte_size);
	ASSERT_EQ(byte_size, 2);
	ASSERT_STREQS((const char *)ptr, "\xc3\xa9", 2);

	rope_str_cleanup(&str);
}

static void
test_str_at_cp_emoji(void) {
	int rv = 0;
	struct RopeStr str = {0};

	rv = rope_str_init(
			&str, (const uint8_t *)FEMALE_ASTRONAUT,
			sizeof(FEMALE_ASTRONAUT) - 1);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;

	const uint8_t *ptr = rope_str_at_cp(&str, 1, &byte_size);
	ASSERT_EQ(byte_size, 7);
	(void)ptr;

	rope_str_cleanup(&str);
}

static void
test_str_utf16_split_ascii(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"HelloWorld", 10);
	ASSERT_EQ(rv, 0);

	struct RopeStr new_str = {0};
	rope_str_utf16_split(&str, &new_str, 5);

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

	rope_str_utf16_split(&str, &new_str, 3);

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
	rope_str_cp_split(&str, &new_str, 5);

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

	rope_str_cp_split(&str, &new_str, 3);

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

	ASSERT_EQ(rope_str_bytes(&src), 0);

	rope_str_cleanup(&dest);
}

static void
test_str_freeable_oob(void) {
	struct RopeStr str = {0};
	uint8_t *buffer = malloc(ROPE_STR_MAX_SIZE + 100);
	ASSERT_NOT_NULL(buffer);

	int rv = rope_str_freeable(&str, buffer, ROPE_STR_MAX_SIZE + 100);
	ASSERT_EQ(rv, -ROPE_ERROR_OOB);

	free(buffer);
}

static void
test_str_freeable_inline(void) {
	struct RopeStr str = {0};

	char *buffer = malloc(ROPE_STR_INLINE_SIZE);
	ASSERT_NOT_NULL(buffer);
	memset(buffer, 'X', ROPE_STR_INLINE_SIZE);

	int rv = rope_str_freeable(&str, (uint8_t *)buffer, ROPE_STR_INLINE_SIZE);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE);

	ASSERT_EQ((uintptr_t)data, (uintptr_t)str.data.inplace);

	rope_str_cleanup(&str);
}

static void
test_str_truncate_size_max(void) {
	int rv = 0;
	struct RopeStr str = {0};
	rv = rope_str_init(&str, (const uint8_t *)"Hello", 5);
	ASSERT_EQ(rv, 0);

	rope_str_truncate(&str, SIZE_MAX);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 5);
	ASSERT_STREQS((const char *)data, "Hello", 5);

	rope_str_cleanup(&str);
}

static void
test_str_truncate_multibyte(void) {
	int rv = 0;
	struct RopeStr str = {0};

	const uint8_t buffer[] = "caf\xc3\xa9";
	rv = rope_str_init(&str, buffer, sizeof(buffer) - 1);
	ASSERT_EQ(rv, 0);

	rope_str_truncate(&str, 3);

	size_t byte_size = 0;
	const uint8_t *data = rope_str_data(&str, &byte_size);
	ASSERT_EQ(byte_size, 3);
	ASSERT_STREQS((const char *)data, "caf", 3);

	ASSERT_EQ(rope_str_chars(&str), 3);
	ASSERT_EQ(rope_str_codepoints(&str), 3);

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

	rv = rope_str_inline_append(&str, (const uint8_t *)"BB", 2);
	ASSERT_EQ(rv, 0);

	rv = rope_str_inline_append(&str, (const uint8_t *)"C", 1);
	ASSERT_EQ(rv, -ROPE_ERROR_OOB);

	rope_str_cleanup(&str);
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
TEST(rope_str_strdup)
TEST(rope_str_strdup_split_into_inline)
TEST(rope_str_strdup_split_into_heap)
TEST(test_str_at_char_ascii)
TEST(test_str_at_char_emoji)
TEST(test_str_at_char_boundary)
TEST(test_str_at_utf16_ascii)
TEST(test_str_at_utf16_with_surrogates)
TEST(test_str_at_cp_ascii)
TEST(test_str_at_cp_multibyte)
TEST(test_str_at_cp_emoji)
TEST(test_str_utf16_split_ascii)
TEST(test_str_utf16_split_after_surrogate)
TEST(test_str_cp_split_ascii)
TEST(test_str_cp_split_multibyte)
TEST(test_str_move_heap)
TEST(test_str_freeable_oob)
TEST(test_str_freeable_inline)
TEST(test_str_truncate_size_max)
TEST(test_str_truncate_multibyte)
TEST(test_str_inline_append_overflow)
END_TESTS
