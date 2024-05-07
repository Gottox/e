#include <highlight.h>
#include <tree_sitter/api.h>
#include <utest.h>

const TSLanguage *tree_sitter_markdown_inline(void);

const TSLanguage *(*language)(void) = tree_sitter_markdown_inline;

static TSQuery *
query_new(const char *source) {
	uint32_t error_offset;
	TSQueryError error_type;
	return ts_query_new(
			language(), source, strlen(source), &error_offset, &error_type);
}

UTEST(highlight, init) {
	struct Highlighter hl = {0};
	int rv = highlighter_init(&hl, query_new(""), NULL, 0);
	ASSERT_EQ(rv, 0);
	highlighter_cleanup(&hl);
}

UTEST(highlight_iterator, init) {
	const char *source_code = "int main();";
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, tree_sitter_markdown_inline());

	TSTree *tree = ts_parser_parse_string(
			parser, NULL, source_code, strlen(source_code));

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(&hl, query_new(""), NULL, 0);
	ASSERT_EQ(rv, 0);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(rv, 0);

	highlight_iterator_cleanup(&it);
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST(highlight_iterator, plain_text) {
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

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(&hl, query_new(query), captures, captures_len);
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
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST(highlight_iterator, emphasis) {
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

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(&hl, query_new(query), captures, captures_len);
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
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST(highlight_iterator, nested) {
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

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(&hl, query_new(query), captures, captures_len);
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
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST(highlight, captures_filter) {
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

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(&hl, query_new(query), captures, captures_len);
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
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST_MAIN()
