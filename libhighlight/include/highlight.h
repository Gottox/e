#include <stdbool.h>
#include <stdlib.h>
#include <tree_sitter/api.h>

enum HighlightEventsType {
	HIGHLIGHT_EVENTS_TYPE_SOURCE,
	HIGHLIGHT_EVENTS_TYPE_START,
	HIGHLIGHT_EVENTS_TYPE_END,
};

struct HighlightEvents {
	enum HighlightEventsType type;
	union {
		struct {
			size_t start;
			size_t end;
		} source;
		struct {
			size_t highlight_size;
		} start;
	};
};

struct HighlightConfig {
	TSLanguage *language;
	char *language_name;
	TSQuery *query;
};

struct Highlighter {
	TSParser *parser;
	TSQueryCursor **cursors;
	size_t cursor_len;
};

struct HighlightIterator {
	struct Highlighter *highlighter;
	TSParser *parser;
	TSQueryCursor *cursor;
	TSQueryMatch *match;
	size_t cursor_index;
	size_t match_index;
};

struct LocalDef {
	char *name;
	size_t value_range_start;
	size_t value_range_end;
	size_t highlight_size;
};

struct LocalScope {
	bool inherits;
	size_t range_start;
	size_t range_end;
	struct LocalDef *locals;
	size_t locals_len;
};

int highlighter_init(
		struct Highlighter *highlighter, const struct HighlightConfig *config);

int highlighter_cleanup(struct Highlighter *highlighter);
