#include <highlight_private.h>
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
static const int capture_names_count = LENGTH(standard_capture_names);

static const char *special_capture_names[] = {
		"injection.content",      "injection.language", "local.definition",
		"local.definition-value", "local.reference",    "local.scope",
};

STATIC_ASSERT(LENGTH(special_capture_names) == HIGHLIGHT_SPECIAL_END);

static int
array_index_of(
		const char *const arr[], size_t arr_len, const char *key, int key_len) {
	for (size_t i = 0; i < arr_len; i++) {
		const char *name = arr[i];
		if (memcmp(name, key, key_len) == 0 && name[key_len] == '\0') {
			return i;
		}
	}
	return -1;
}

static int
setup_lookup_table(
		struct Highlight *highlight, const struct HighlightConfig *config) {
	int rv = 0;
	TSQuery *query = highlight->query;
	const size_t query_captures_count = ts_query_capture_count(query);

	uint32_t *capture_lookup_table =
			calloc(query_captures_count, sizeof(uint32_t));
	if (capture_lookup_table == NULL) {
		rv = -1;
		goto out;
	}

	const char *const *captures = config->captures;
	size_t captures_count = config->captures_count;
	if (captures == NULL) {
		captures = standard_capture_names;
		captures_count = capture_names_count;
	}
	for (size_t i = 0; i < query_captures_count; i++) {
		uint32_t name_len = 0;
		const char *name = ts_query_capture_name_for_id(query, i, &name_len);

		int idx = array_index_of(
				special_capture_names, LENGTH(special_capture_names), name,
				name_len);
		if (idx >= 0) {
			capture_lookup_table[i] = idx | HIGHLIGHT_SPECIAL_MASK;
			continue;
		}
		idx = array_index_of(captures, captures_count, name, name_len);
		if (idx >= 0) {
			capture_lookup_table[i] = idx;
			continue;
		}

		capture_lookup_table[i] = HIGHLIGHT_NO_CAPTURE;
		ts_query_disable_capture(query, name, name_len);
	}

	highlight->capture_lookup_table = capture_lookup_table;
	capture_lookup_table = NULL;
out:
	free(capture_lookup_table);
	return rv;
}

int
compile_query(
		struct Highlight *highlight, const struct HighlightConfig *config,
		uint32_t *error_offset, TSQueryError *error) {
	int rv = 0;
	char *global = NULL;
	size_t global_len = 0;

	for (size_t i = 0; i < HIGHLIGHT_SOURCE_END; i++) {
		global_len += config->source[i].len;
	}

	global = calloc(global_len + 1, sizeof(char));
	if (global == NULL) {
		rv = -1;
		goto out;
	}

	size_t offset = 0;
	for (size_t i = 0; i < HIGHLIGHT_SOURCE_END; i++) {
		const char *source = config->source[i].source;
		size_t len = config->source[i].len;
		if (config->source[i].source) {
			memcpy(&global[offset], source, len);
		}
		offset += len;
	}

	highlight->query = ts_query_new(
			config->language, global, global_len, error_offset, error);
	if (*error != TSQueryErrorNone) {
		rv = -1;
		goto out;
	}

	rv = setup_lookup_table(highlight, config);
	if (rv < 0) {
		goto out;
	}

out:
	free(global);
	return rv;
}

int
highlight_init(
		struct Highlight *highlight, const struct HighlightConfig *config) {
	int rv = 0;
	memset(highlight, 0, sizeof(struct Highlight));

	TSQueryError error = TSQueryErrorNone;
	uint32_t error_offset = 0;
	rv = compile_query(highlight, config, &error_offset, &error);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		highlight_cleanup(highlight);
	}
	return rv;
}

int
highlight_cleanup(struct Highlight *highlight) {
	if (highlight->query) {
		ts_query_delete(highlight->query);
	}
	free(highlight->capture_lookup_table);
	return 0;
}
