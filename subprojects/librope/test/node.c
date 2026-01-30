#include "common.h"
#include <assert.h>
#include <grapheme.h>
#include <rope.h>
#include <rope_node.h>
#include <rope_str.h>
#include <stdbool.h>
#include <testlib.h>

static void
check_valid_utf8(struct RopeNode *node) {
	node = rope_node_first(node);
	do {
		size_t byte_size = 0;
		const uint8_t *data = rope_node_value(node, &byte_size);
		for (size_t i = 0; i < byte_size;) {
			uint_least32_t cp = 0;
			grapheme_decode_utf8((const char *)&data[i], byte_size - i, &cp);
			ASSERT_NE(GRAPHEME_INVALID_CODEPOINT, cp);
			size_t char_size = grapheme_next_character_break_utf8(
					(const char *)&data[i], byte_size - i);
			i += char_size;
		}
	} while ((node = rope_node_next(node)) != NULL);
}

static void
test_node_split_inline_middle(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'HELO'");

	struct RopeNode *left, *right;
	rv = rope_node_split(root, &pool, 2, ROPE_BYTE, &left, &right);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['HE','LO']", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'HE'");

	rv = rope_node_insert_right(root, (const uint8_t *)"LO", 2, 0xFF, &pool);

	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));
	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	ASSERT_EQ(root, rope_node_parent(right));
	ASSERT_EQ(root, rope_node_parent(left));

	ASSERT_JSONEQ("['HE','LO']", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_delete(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['HE','LO']");

	rope_node_delete(rope_node_left(root), &pool);

	ASSERT_JSONEQ("'LO'", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_rotate_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['HE',['LL','O']]");

	rope_node_rotate(root, ROPE_LEFT);

	ASSERT_JSONEQ("[['HE','LL'],'O']", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_balance_left(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "[[[['H','E'],'L'],'L'],'O']");

	struct RopeNode *hel_node = rope_node_left(root);
	hel_node = rope_node_left(hel_node);

	rope_node_balance_up2(hel_node);

	ASSERT_JSONEQ("[['H','E'],[['L','L'],'O']]", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_balance_right(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['H',['E',['L',['L','O']]]]");

	struct RopeNode *lo_node = rope_node_right(root);
	lo_node = rope_node_right(lo_node);

	rope_node_balance_up2(lo_node);

	ASSERT_JSONEQ("[['H',['E','L']],['L','O']]", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

void
test_node_merge(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['1',['2',['3','4']]]");

	struct RopeNode *node = rope_node_first(root);

	rv = rope_node_merge(node, 2, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_JSONEQ("['123','4']", root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}
static void
test_node_insert_incomplete_utf8(void) {
	int rv = 0;

	// must be larger than ROPE_STR_INLINE_SIZE
	uint8_t women_emoji[] =
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6";

	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);
	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(
			root, women_emoji, ROPE_STR_INLINE_SIZE + 2, 0, &pool);
	rv = rope_node_insert_right(
			root, &women_emoji[2], ROPE_STR_INLINE_SIZE + 2, 0, &pool);

	check_valid_utf8(root);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_right_malloc(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	uint8_t *buffer = malloc(ROPE_STR_INLINE_SIZE * 2);
	memset(buffer, 'A', ROPE_STR_INLINE_SIZE * 2);
	ASSERT_NOT_NULL(buffer);

	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_heap_right(
			root, buffer, ROPE_STR_INLINE_SIZE * 2, 0, &pool);
	ASSERT_EQ(rv, 0);

	size_t byte_size = 0;
	const uint8_t *data = rope_node_value(root, &byte_size);
	ASSERT_EQ(byte_size, ROPE_STR_INLINE_SIZE * 2);
	ASSERT_EQ((uintptr_t)buffer, (uintptr_t)data);
	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_insert_big(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	uint8_t buffer[65536] = {0};
	memset(buffer, 'A', sizeof(buffer));

	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_right(root, buffer, sizeof(buffer), 0, &pool);
	ASSERT_EQ(rv, 0);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_stitch_overflow_leaf_size(void) {
	int rv = 0;

	char smiley[] = "\xf0\x9f\x98\x83";
	char buffer1[ROPE_STR_FAST_SIZE] = {0};
	memset(buffer1, 'A', sizeof(buffer1));
	char buffer2[ROPE_STR_FAST_SIZE] = {0};
	memset(buffer2, 'B', sizeof(buffer2));

	memcpy(&buffer1[ROPE_STR_FAST_SIZE - 2], smiley, 2);
	memcpy(buffer2, &smiley[2], 2);

	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer1, sizeof(buffer1), 0, &pool);
	ASSERT_EQ(rv, 0);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer2, sizeof(buffer2), 0, &pool);
	ASSERT_EQ(rv, 0);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_insert_grapheme(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(
			root, (const uint8_t *)"e\xCC\x8A\xCC\x8A", 5, 0, &pool);
	ASSERT_EQ(0, rv);
	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(size, 5);
	ASSERT_STREQ("e\xCC\x8A\xCC\x8A", (const char *)data);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_insert_large_grapheme(void) {
	char buffer[ROPE_STR_FAST_SIZE * 2] = {0};
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	buffer[0] = 'e';
	const uint8_t diaeresis[] = {0xCC, 0x8A};
	const size_t d_size = sizeof(diaeresis);
	size_t size = 1;
	for (; size < sizeof(buffer) - d_size; size += d_size) {
		memcpy(&buffer[size], diaeresis, d_size);
	}

	struct RopeNode *root = rope_node_new(&pool);

	rv = rope_node_insert_right(root, (const uint8_t *)buffer, size, 0, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_node_insert_right(root, (const uint8_t *)buffer, size, 0, &pool);
	ASSERT_EQ(0, rv);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_unbreak_utf8_sequence(void) {
	const char *str = "e\xCC\x8A\xCC\x8A";
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['dummy','dummy']");
	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	rv = rope_str_init(&left->data.leaf, (const uint8_t *)str, 2);
	ASSERT_EQ(0, rv);
	rv = rope_str_init(&right->data.leaf, (const uint8_t *)&str[3], 2);
	ASSERT_EQ(0, rv);

	rv = rope_node_insert_right(left, (const uint8_t *)&str[2], 1, 0, &pool);
	ASSERT_EQ(0, rv);

	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(size, 5);
	ASSERT_STREQ("e\xCC\x8A\xCC\x8A", (const char *)data);
	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_test_utf8_sequence_sliding_right(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	const uint8_t women_emoji[] =
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6";
	for (size_t i = 0; i < sizeof(women_emoji); i++) {
		struct RopeNode *root = rope_node_new(&pool);

		rv = rope_node_insert_right(root, women_emoji, i, 0, &pool);
		ASSERT_EQ(0, rv);

		rv = rope_node_insert_right(
				root, &women_emoji[i], sizeof(women_emoji) - i, 0, &pool);
		ASSERT_EQ(0, rv);

		check_integrity(root);
		rope_node_free(root, &pool);
	}
	rope_pool_cleanup(&pool);
}

static void
test_test_utf8_sequence_sliding_left(void) {
	int rv = 0;
	struct RopePool pool = {0};
	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	const uint8_t women_emoji[] =
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6"
			"\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6\xF0\x9F\x91\xA6";
	for (size_t i = 0; i < sizeof(women_emoji); i++) {
		struct RopeNode *root = rope_node_new(&pool);

		rv = rope_node_insert_left(
				root, &women_emoji[i], sizeof(women_emoji) - i, 0, &pool);
		ASSERT_EQ(0, rv);

		rv = rope_node_insert_left(root, women_emoji, i, 0, &pool);
		ASSERT_EQ(0, rv);

		check_integrity(root);
		rope_node_free(root, &pool);
	}
	rope_pool_cleanup(&pool);
}

static void
test_node_balance_preserves_sizes(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['A',['B',['C',['D','E']]]]");

	ASSERT_EQ(5, rope_node_size(root, ROPE_BYTE));

	struct RopeNode *deep_node = rope_node_right(root);
	deep_node = rope_node_right(deep_node);

	rope_node_balance_up2(deep_node);

	ASSERT_EQ(5, rope_node_size(root, ROPE_BYTE));

	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);
	ASSERT_EQ(
			5,
			rope_node_size(left, ROPE_BYTE) + rope_node_size(right, ROPE_BYTE));

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_node_rotate_preserves_sizes(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['AB',['CD','EF']]");

	// Verify sizes before rotation
	ASSERT_EQ(6, rope_node_size(root, ROPE_BYTE));
	struct RopeNode *right = rope_node_right(root);
	ASSERT_EQ(4, rope_node_size(right, ROPE_BYTE));

	rope_node_rotate(root, ROPE_LEFT);
	ASSERT_JSONEQ("[['AB','CD'],'EF']", root);

	ASSERT_EQ(6, rope_node_size(root, ROPE_BYTE));
	struct RopeNode *left = rope_node_left(root);
	ASSERT_EQ(4, rope_node_size(left, ROPE_BYTE));
	right = rope_node_right(root);
	ASSERT_EQ(2, rope_node_size(right, ROPE_BYTE));

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_simple_branch(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['Hello','World']");
	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));

	rv = rope_node_compact(root, &pool);
	ASSERT_EQ(0, rv);

	// After compact, should be a single leaf with concatenated content
	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));

	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(10, size);
	ASSERT_STREQ("HelloWorld", (const char *)data);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_deeply_nested_tree(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	// Create deeply nested tree: [[[['A','B'],'C'],'D'],'E']
	struct RopeNode *root = from_str(&pool, "[[[['A','B'],'C'],'D'],'E']");
	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));

	rv = rope_node_compact(root, &pool);
	ASSERT_EQ(0, rv);

	// After compact, should be a single leaf
	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));

	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(5, size);
	ASSERT_STREQ("ABCDE", (const char *)data);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

// Test: compact is a no-op when total size exceeds ROPE_STR_FAST_SIZE
static void
test_compact_size_boundary_exceeds_limit(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	// Create buffers that together exceed ROPE_STR_FAST_SIZE (~1020 bytes)
	char buffer1[600] = {0};
	char buffer2[600] = {0};
	memset(buffer1, 'A', sizeof(buffer1) - 1);
	memset(buffer2, 'B', sizeof(buffer2) - 1);

	struct RopeNode *root = rope_node_new(&pool);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer1, sizeof(buffer1) - 1, 0, &pool);
	ASSERT_EQ(0, rv);
	rv = rope_node_insert_right(
			root, (const uint8_t *)buffer2, sizeof(buffer2) - 1, 0, &pool);
	ASSERT_EQ(0, rv);

	// Verify it's a branch before compact
	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));

	// Total size is 1198 bytes > ROPE_STR_FAST_SIZE (~1020)
	size_t total = rope_node_size(root, ROPE_BYTE);
	ASSERT_GT(total, ROPE_STR_FAST_SIZE);

	rv = rope_node_compact(root, &pool);
	ASSERT_EQ(0, rv);

	// Should still be a branch (compact is no-op for large trees)
	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(root));

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_on_leaf_is_noop(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "'Hello'");
	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));

	uint64_t original_tags = 0x123;
	rope_node_add_tags(root, original_tags);
	ASSERT_EQ(original_tags, rope_node_tags(root));

	rv = rope_node_compact(root, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));

	size_t size = 0;
	const uint8_t *data = rope_node_value(root, &size);
	ASSERT_EQ(5, size);
	ASSERT_STREQ("Hello", (const char *)data);

	ASSERT_EQ(original_tags, rope_node_tags(root));

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_tags_persist(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root = from_str(&pool, "['Left','Right']");

	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	uint64_t tag_a = 0x01;
	uint64_t tag_b = 0x02;
	rope_node_add_tags(left, tag_a);
	rope_node_add_tags(right, tag_b);

	ASSERT_EQ(tag_a, rope_node_tags(left));
	ASSERT_EQ(tag_b, rope_node_tags(right));

	rv = rope_node_compact(root, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(root));

	uint64_t expected_tags = tag_a | tag_b;
	uint64_t actual_tags = rope_node_tags(root);

	ASSERT_EQ(expected_tags, actual_tags);

	check_integrity(root);
	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_subtree_avl_consistency(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root =
			from_str(&pool, "[['A','B'],[['D','E'],'C']]");

	struct RopeNode *small = rope_node_left(root);

	rv = rope_node_compact(small, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(small));

	size_t size = 0;
	const uint8_t *data = rope_node_value(small, &size);
	ASSERT_EQ(2, size);
	ASSERT_STREQ("AB", (const char *)data);

	check_integrity(root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

static void
test_compact_subtree_with_deep_sibling(void) {
	int rv = 0;
	struct RopePool pool = {0};

	rv = rope_pool_init(&pool);
	ASSERT_EQ(0, rv);

	struct RopeNode *root =
			from_str(&pool, "[['A','B'],[[[['C','D'],'E'],'F'],'G']]");

	struct RopeNode *left = rope_node_left(root);
	struct RopeNode *right = rope_node_right(root);

	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(left));
	ASSERT_EQ(ROPE_NODE_BRANCH, rope_node_type(right));

	size_t left_depth = rope_node_depth(left);
	size_t right_depth = rope_node_depth(right);

	// Right subtree should be deeper
	ASSERT_GT(right_depth, left_depth);

	// Compact the shallow left subtree
	rv = rope_node_compact(left, &pool);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(ROPE_NODE_LEAF, rope_node_type(left));
	ASSERT_EQ(0u, rope_node_depth(left));

	check_integrity(root);

	rope_node_free(root, &pool);
	rope_pool_cleanup(&pool);
}

DECLARE_TESTS
TEST(test_node_split_inline_middle)
TEST(test_node_insert_right)
TEST(test_node_delete)
TEST(test_node_rotate_right)
TEST(test_node_balance_left)
TEST(test_node_balance_right)
TEST(test_node_merge)
TEST(test_node_insert_incomplete_utf8)
TEST(test_node_insert_right_malloc)
TEST(test_node_insert_big)
TEST(test_node_stitch_overflow_leaf_size)
TEST(test_insert_grapheme)
TEST(test_insert_large_grapheme)
TEST(test_unbreak_utf8_sequence)
TEST(test_test_utf8_sequence_sliding_right)
TEST(test_test_utf8_sequence_sliding_left)
TEST(test_node_balance_preserves_sizes)
TEST(test_node_rotate_preserves_sizes)
NO_TEST(test_compact_simple_branch)
NO_TEST(test_compact_deeply_nested_tree)
NO_TEST(test_compact_size_boundary_exceeds_limit)
NO_TEST(test_compact_on_leaf_is_noop)
NO_TEST(test_compact_tags_persist)
NO_TEST(test_compact_subtree_avl_consistency)
NO_TEST(test_compact_subtree_with_deep_sibling)
END_TESTS
