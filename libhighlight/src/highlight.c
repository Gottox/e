#include <highlight.h>
#include <string.h>

const char *const standard_capture_names[] = {
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
};
static const int capture_names_count =
		sizeof(standard_capture_names) / sizeof(standard_capture_names[0]);

static int
setup_lookup_table(
		struct Highlighter *highlighter, const char *const captures[],
		size_t captures_count) {
	int rv = 0;
	TSQuery *query = highlighter->query;
	const size_t query_captures_count = ts_query_capture_count(query);

	uint32_t *capture_lookup_table =
			calloc(query_captures_count, sizeof(uint32_t));
	if (capture_lookup_table == NULL) {
		rv = -1;
		goto out;
	}

	for (size_t i = 0; i < query_captures_count; i++) {
		uint32_t name_len = 0;
		const char *name = ts_query_capture_name_for_id(query, i, &name_len);

		uint32_t j;
		capture_lookup_table[i] = UINT32_MAX;
		for (j = 0; j < captures_count; j++) {
			if (strncmp(name, captures[j], name_len) == 0) {
				capture_lookup_table[i] = j;
				break;
			}
		}
		if (capture_lookup_table[i] == UINT32_MAX) {
			ts_query_disable_capture(query, name, name_len);
		}
	}

	highlighter->capture_lookup_table = capture_lookup_table;
out:
	if (rv < 0) {
		free(capture_lookup_table);
	}
	return rv;
}

int
highlighter_init(
		struct Highlighter *highlighter, TSQuery *query,
		const char *const captures[], size_t captures_count) {
	int rv = 0;
	memset(highlighter, 0, sizeof(struct Highlighter));

	highlighter->query = query;

	if (captures == NULL) {
		captures = standard_capture_names;
		captures_count = capture_names_count;
	}

	rv = setup_lookup_table(highlighter, captures, captures_count);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		highlighter_cleanup(highlighter);
	}
	return rv;
}

int
highlighter_cleanup(struct Highlighter *highlighter) {
	if (highlighter->query) {
		ts_query_delete(highlighter->query);
	}
	free(highlighter->capture_lookup_table);
	return 0;
}
