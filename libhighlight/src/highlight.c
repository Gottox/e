#include <cextras/memory.h>
#include <highlight.h>
#include <netinet/in.h>
#include <stdlib.h>

/*static const char *const standard_capture_names[] = {
		"attribute",
		"boolean",
		"carriage-return",
		"comment",
		"comment.documentation",
		"constant",
		"constant.builtin",
		"constructor",
		"constructor.builtin",
		"embedded",
		"error",
		"escape",
		"function",
		"function.builtin",
		"keyword",
		"markup",
		"markup.bold",
		"markup.heading",
		"markup.italic",
		"markup.link",
		"markup.link.url",
		"markup.list",
		"markup.list.checked",
		"markup.list.numbered",
		"markup.list.unchecked",
		"markup.list.unnumbered",
		"markup.quote",
		"markup.raw",
		"markup.raw.block",
		"markup.raw.inline",
		"markup.strikethrough",
		"module",
		"number",
		"operator",
		"property",
		"property.builtin",
		"punctuation",
		"punctuation.bracket",
		"punctuation.delimiter",
		"punctuation.special",
		"string",
		"string.escape",
		"string.regexp",
		"string.special",
		"string.special.symbol",
		"tag",
		"type",
		"type.builtin",
		"variable",
		"variable.builtin",
		"variable.member",
		"variable.parameter",
		NULL,
};*/

int
highlighter_init(
		struct Highlighter *highlighter, const struct HighlightConfig *config) {
	(void)config;
	highlighter->parser = ts_parser_new();
	if (!highlighter->parser) {
		return -1;
	}
	highlighter->cursors = NULL;
	highlighter->cursor_len = 0;
	return 0;
}

int
highlighter_cleanup(struct Highlighter *highlighter) {
	for (size_t i = 0; i < highlighter->cursor_len; i++) {
		ts_query_cursor_delete(highlighter->cursors[i]);
	}
	free(highlighter->cursors);
	ts_parser_delete(highlighter->parser);
	return 0;
}

int
highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlighter *highlighter,
		const struct HighlightConfig *config, const TSTree *tree,
		const TSInput *source) {
	(void)highlighter;
	(void)config;
	(void)tree;
	(void)source;
	(void)iterator;
	return 0;
}

int
highlight_iterator_cleanup(struct HighlightIterator *iterator) {
	(void)iterator;
	return 0;
}
