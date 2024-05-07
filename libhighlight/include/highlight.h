#include <stdbool.h>
#include <stdlib.h>
#include <tree_sitter/api.h>

typedef size_t highlight_index_t;

struct Highlighter {
	TSQuery *query;
	uint32_t *capture_lookup_table;
};

enum HighlightEventType {
	HIGHLIGHT_END,
	HIGHLIGHT_TEXT,
	HIGHLIGHT_START,
};

struct HighlightEvent {
	enum HighlightEventType type;
	union {
		struct {
			uint32_t capture_id;
		} start;
		struct {
			uint32_t start;
			uint32_t end;
		} text;
	};
};

struct HighlightMarker {
	uint32_t offset;
	uint32_t capture_id;
	struct HighlightMarker *next;
};

struct HighlightMarkerPool {
	struct HighlightMarker *recycle;
	struct HighlightMarker **pools;
	size_t pool_size;
};

struct HighlightIterator {
	struct Highlighter *highlighter;
	TSQueryCursor *cursor;
	const TSTree *tree;
	uint_fast32_t current_offset;
	uint_fast32_t tree_completed_offset;
	uint_fast32_t end_offset;
	uint_fast32_t current_capture_id;
	struct HighlightMarkerPool marker_pool;
	struct HighlightMarker *markers;
};

int highlight_marker_pool_init(struct HighlightMarkerPool *pool);

int highlight_marker_pool_cleanup(struct HighlightMarkerPool *pool);

struct HighlightMarker *
highlight_marker_pool_new(struct HighlightMarkerPool *pool);

int highlight_marker_pool_recycle(
		struct HighlightMarkerPool *pool, struct HighlightMarker *marker);

int highlighter_init(
		struct Highlighter *highlighter, TSQuery *query,
		const char *const captures[], size_t captures_len);

int highlighter_cleanup(struct Highlighter *highlighter);

int highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlighter *highlighter,
		const TSTree *tree);

bool highlight_iterator_next(
		struct HighlightIterator *iterator, struct HighlightEvent *event,
		int *err);

int highlight_iterator_cleanup(struct HighlightIterator *iterator);
