#include <stdbool.h>
#include <stdlib.h>
#include <tree_sitter/api.h>
typedef size_t highlight_index_t;

struct Highlighter {
	const TSLanguage *language;
	TSQuery *query;
	const char *const *captures;
	size_t captures_len;
};

enum HighlightEventType {
	HIGHLIGHT_START,
	HIGHLIGHT_TEXT,
	HIGHLIGHT_END,
};

struct HighlightEvent {
	enum HighlightEventType type;
	union {
		struct {
			const char *capture;
		} start;
		struct {
			uint32_t start;
			uint32_t end;
		} text;
	};
};

struct HighlightPosition;

struct HighlightIterator {
	struct Highlighter *highlighter;
	TSQueryCursor *cursor;
	const TSTree *tree;
	size_t last_start_byte;
	struct HighlightPosition *positions;
	struct HighlightPosition *positions_recycle;
	struct HighlightPosition **position_pools;
	size_t position_pool_index;
};

int highlighter_init(
		struct Highlighter *highlighter, const TSLanguage *language,
		const char *highlight_query, size_t highlight_query_len,
		const char *const captures[], size_t captures_len);

int highlighter_cleanup(struct Highlighter *highlighter);

int highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlighter *highlighter,
		const TSTree *tree);

bool highlight_iterator_next(
		struct HighlightIterator *iterator, struct HighlightEvent *event,
		int *err);

int highlight_iterator_cleanup(struct HighlightIterator *iterator);
