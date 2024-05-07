#include <highlight.h>
#include <string.h>

static const char *const standard_capture_names[] = {
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
};
static const int standard_capture_names_len =
		sizeof(standard_capture_names) / sizeof(standard_capture_names[0]);

int
highlighter_init(
		struct Highlighter *highlighter, const TSLanguage *language,
		const char *highlight_query, size_t highlight_query_len,
		const char *const captures[], size_t captures_len) {
	int rv = 0;
	memset(highlighter, 0, sizeof(struct Highlighter));

	uint32_t error_offset = 0;
	TSQueryError error_type = TSQueryErrorNone;

	// Setup query
	highlighter->query = ts_query_new(
			language, highlight_query, highlight_query_len, &error_offset,
			&error_type);

	if (highlighter->query == NULL) {
		rv = -1;
		goto out;
	}

	if (captures == NULL) {
		captures = standard_capture_names;
		captures_len = standard_capture_names_len;
	}
	highlighter->captures = captures;
	highlighter->captures_len = captures_len;
	highlighter->language = language;

out:
	if (rv < 0) {
		highlighter_cleanup(highlighter);
	}
	return rv;
}

int
highlighter_cleanup(struct Highlighter *highlighter) {
	if (highlighter->query != NULL) {
		ts_query_delete(highlighter->query);
	}
	return 0;
}
