#include <highlight.h>
#include <tree_sitter/api.h>
#include <testlib.h>
#include <string.h>
#include <stdio.h>

const TSLanguage *tree_sitter_markdown_inline(void);

const TSLanguage *(*language)(void) = tree_sitter_markdown_inline;

#define UTEST(x, y) static void test_ ## x ## _ ##y(void)

static struct HighlightConfig *
new_config(
		const char *source, const char *const *captures, size_t captures_len,
		struct HighlightConfig *config) {
	highlight_config_init(config, language());
	highlight_config_captures(config, captures, captures_len);
	highlight_config_highlight(config, source, strlen(source));

	return config;
}

#define NEW_CONFIG(source, captures, captures_len) \
	new_config(source, captures, captures_len, &(struct HighlightConfig){0})

static void
test_highlight_init(void) {
	struct Highlight hl = {0};
	int rv = highlight_init(&hl, NEW_CONFIG("", NULL, 0));
	ASSERT_EQ(rv, 0);
	highlight_cleanup(&hl);
}

static void
test_highlight_iterator_init(void) {
	const char *source_code = "int main();";
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, tree_sitter_markdown_inline());

	TSTree *tree = ts_parser_parse_string(
			parser, NULL, source_code, strlen(source_code));

	struct Highlight hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlight_init(&hl, NEW_CONFIG("", NULL, 0));
	ASSERT_EQ(rv, 0);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(rv, 0);

	highlight_iterator_cleanup(&it);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

static void
test_highlight_iterator_plain_text(void) {
	const char *source = "text";
	const size_t source_len = strlen(source);
	const char *query = "(emphasis) @emphasis\n";
	const char *captures[] = {"emphasis"};
	const size_t captures_len = sizeof(captures) / sizeof(captures[0]);

	struct HighlightEvent ev;
	const TSLanguage *lang = language();
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, lang);

	TSTree *tree = ts_parser_parse_string(parser, NULL, source, source_len);

	struct Highlight hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlight_init(&hl, NEW_CONFIG(query, captures, captures_len));
	ASSERT_EQ(0, rv);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(0, rv);

	bool has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)0, ev.text.start);
	ASSERT_EQ((uint32_t)4, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);

	highlight_iterator_cleanup(&it);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

static void
test_highlight_iterator_emphasis(void) {
	const char *source = "*text*";
	const size_t source_len = strlen(source);
	const char *query = "(emphasis) @emphasis\n";
	const char *captures[] = {"emphasis"};
	const size_t captures_len = sizeof(captures) / sizeof(captures[0]);

	struct HighlightEvent ev;
	const TSLanguage *lang = language();
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, lang);

	TSTree *tree = ts_parser_parse_string(parser, NULL, source, source_len);

	struct Highlight hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlight_init(&hl, NEW_CONFIG(query, captures, captures_len));
	ASSERT_EQ(0, rv);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(0, rv);

	bool has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_START, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)0, ev.text.start);
	ASSERT_EQ((uint32_t)6, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_END, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);

	highlight_iterator_cleanup(&it);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

static void
test_highlight_iterator_nested(void) {
	const char *source = "***text***";
	const size_t source_len = strlen(source);
	const char *query = "(strong_emphasis) @strong_emphasis\n"
						"(emphasis) @emphasis\n";
	const char *captures[] = {"strong_emphasis", "emphasis"};
	const size_t captures_len = sizeof(captures) / sizeof(captures[0]);

	struct HighlightEvent ev;
	const TSLanguage *lang = language();
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, lang);

	TSTree *tree = ts_parser_parse_string(parser, NULL, source, source_len);

	struct Highlight hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlight_init(&hl, NEW_CONFIG(query, captures, captures_len));
	ASSERT_EQ(0, rv);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(0, rv);

	bool has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_START, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)0, ev.text.start);
	ASSERT_EQ((uint32_t)1, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_END, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_START, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)1, ev.text.start);
	ASSERT_EQ((uint32_t)9, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_END, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_START, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)9, ev.text.start);
	ASSERT_EQ((uint32_t)10, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_END, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);

	highlight_iterator_cleanup(&it);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

static void
test_highlight_captures_filter(void) {
	const char *source = "***text***";
	const size_t source_len = strlen(source);
	const char *query = "(strong_emphasis) @strong_emphasis\n"
						"(emphasis) @emphasis\n";
	const char *captures[] = {"strong_emphasis"};
	const size_t captures_len = sizeof(captures) / sizeof(captures[0]);

	struct HighlightEvent ev;
	const TSLanguage *lang = language();
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, lang);

	TSTree *tree = ts_parser_parse_string(parser, NULL, source, source_len);

	struct Highlight hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlight_init(&hl, NEW_CONFIG(query, captures, captures_len));
	ASSERT_EQ(0, rv);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(0, rv);

	bool has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)0, ev.text.start);
	ASSERT_EQ((uint32_t)1, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_START, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)1, ev.text.start);
	ASSERT_EQ((uint32_t)9, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_END, (int)ev.type);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);
	ASSERT_EQ(HIGHLIGHT_TEXT, (int)ev.type);
	ASSERT_EQ((uint32_t)9, ev.text.start);
	ASSERT_EQ((uint32_t)10, ev.text.end);

	has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_FALSE(has_next);
	ASSERT_EQ(0, rv);

	highlight_iterator_cleanup(&it);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

DECLARE_TESTS
TEST(test_highlight_init)
TEST(test_highlight_iterator_init)
TEST(test_highlight_iterator_plain_text)
TEST(test_highlight_iterator_emphasis)
TEST(test_highlight_iterator_nested)
TEST(test_highlight_captures_filter)
END_TESTS
