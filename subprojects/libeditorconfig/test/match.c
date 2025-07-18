#include <editorconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static int
match(const char *pat_str, const char *str_str) {
	const char *pat = pat_str;
	const char *str = str_str;
	size_t str_len = strlen(str_str);
	return editorconfig_match(pat, str, str_len);
}

static void
test_simple_match_basic(void) {
	ASSERT_EQ(match("hello", "hello"), 1);
	ASSERT_EQ(match("h?llo", "hello"), 1);
	ASSERT_EQ(match("h*o", "hello"), 1);
	ASSERT_EQ(match("h*llo", "hello"), 1);
	ASSERT_EQ(match("*", "hello"), 1);
	ASSERT_EQ(match("**", "hello"), 1);
	ASSERT_EQ(match("hel*xyz", "hello"), 0);
	ASSERT_EQ(match("hel?o", "hello"), 1);
	ASSERT_EQ(match("hel??", "hello"), 1);
	ASSERT_EQ(match("hel???", "hello"), 0);
}

static void
test_simple_match_brackets(void) {
	ASSERT_EQ(match("h[ae]llo", "hello"), 1);
	ASSERT_EQ(match("h[!b]llo", "hello"), 1);
	ASSERT_EQ(match("h[!e]llo", "hello"), 0);
	ASSERT_EQ(match("h[a-z]llo", "hello"), 1);
	ASSERT_EQ(match("h[z-a]llo", "hello"), 0); // invalid range
}

static void
test_simple_match_braces_set(void) {
	ASSERT_EQ(match("h{ello,allo}", "hello"), 1);
	ASSERT_EQ(match("h{ello,allo}", "hallo"), 1);
	ASSERT_EQ(match("h{ello,allo}", "hullo"), 0);
	ASSERT_EQ(match("prefix-{one,two,three}-suffix", "prefix-two-suffix"), 1);
	ASSERT_EQ(match("prefix-{one,two,three}-suffix", "prefix-four-suffix"), 0);
}

static void
test_simple_match_braces_range(void) {
	ASSERT_EQ(match("{1..5}", "3"), 1);
	ASSERT_EQ(match("{1..5}", "6"), 0);
	ASSERT_EQ(match("{-2..2}", "-1"), 1);
	ASSERT_EQ(match("{-2..2}", "0"), 1);
	ASSERT_EQ(match("foo{10..12}bar", "foo11bar"), 1);
	ASSERT_EQ(match("foo{10..12}bar", "foo13bar"), 0);
}

static void
test_simple_match_slash_and_double_star(void) {
	ASSERT_EQ(match("**/bar", "foo/bar"), 1);
	ASSERT_EQ(match("*/bar", "foo/bar"), 1);
	ASSERT_EQ(match("*/bar", "foo/baz/bar"), 0);
	ASSERT_EQ(match("**/bar", "foo/baz/bar"), 1);
	ASSERT_EQ(match("**", "foo/bar/baz"), 1);
	ASSERT_EQ(match("*", "foo/bar/baz"), 0);
}

DECLARE_TESTS
TEST(test_simple_match_basic)
TEST(test_simple_match_brackets)
TEST(test_simple_match_braces_set)
TEST(test_simple_match_braces_range)
TEST(test_simple_match_slash_and_double_star)
END_TESTS
