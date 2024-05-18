#include <highlight_private.h>
#include <string.h>

int
highlight_config_init(
		struct HighlightConfig *config, const TSLanguage *language) {
	config->language = language;

	return 0;
}

void
highlight_config_highlight(
		struct HighlightConfig *config, const char *source, size_t source_len) {
	config->source[HIGHLIGHT_SOURCE_HIGHLIGHT].source = source;
	config->source[HIGHLIGHT_SOURCE_HIGHLIGHT].len = source_len;
}

void
highlight_config_injection(
		struct HighlightConfig *config, const char *source, size_t source_len) {
	config->source[HIGHLIGHT_SOURCE_INJECTION].source = source;
	config->source[HIGHLIGHT_SOURCE_INJECTION].len = source_len;
}

void
highlight_config_local(
		struct HighlightConfig *config, const char *source, size_t source_len) {
	config->source[HIGHLIGHT_SOURCE_LOCAL].source = source;
	config->source[HIGHLIGHT_SOURCE_LOCAL].len = source_len;
}

void
highlight_config_captures(
		struct HighlightConfig *config, const char *const *captures,
		size_t captures_len) {
	config->captures = captures;
	config->captures_count = captures_len;
}

int
highlight_config_cleanup(struct HighlightConfig *config) {
	memset(config, 0, sizeof(*config));

	return 0;
}
