#include <highlight.h>
#include <utest.h>

const TSLanguage *tree_sitter_markdown_inline(void);

const TSLanguage *(*language)(void) = tree_sitter_markdown_inline;

UTEST(highlight, init) {
	struct Highlighter hl = {0};
	int rv = highlighter_init(&hl, language(), "", 0, NULL, 0);
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
	int rv = highlighter_init(&hl, language(), "", 0, NULL, 0);
	ASSERT_EQ(rv, 0);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(rv, 0);

	highlight_iterator_cleanup(&it);
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST(highlight_iterator, next_once) {
	const char *source = "[***text***](https://example.com)";
	const size_t source_len = strlen(source);
	const char *query = "(link_text) @link_text\n"
						"(strong_emphasis) @strong_emphasis\n"
						"(emphasis) @emphasis\n";
	const size_t query_len = strlen(query);
	const char *captures[] = {"link_text", "strong_emphasis", "emphasis"};
	const size_t captures_len = sizeof(captures) / sizeof(captures[0]);

	struct HighlightEvent ev;
	const TSLanguage *lang = language();
	TSParser *parser = ts_parser_new();
	ts_parser_set_language(parser, lang);

	TSTree *tree = ts_parser_parse_string(parser, NULL, source, source_len);

	struct Highlighter hl = {0};
	struct HighlightIterator it = {0};
	int rv = highlighter_init(
			&hl, lang, query, query_len, captures, captures_len);
	ASSERT_EQ(0, rv);

	rv = highlight_iterator_init(&it, &hl, tree);
	ASSERT_EQ(0, rv);

	bool has_next = highlight_iterator_next(&it, &ev, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(has_next);

	highlight_iterator_cleanup(&it);
	highlighter_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
}

UTEST_MAIN()
