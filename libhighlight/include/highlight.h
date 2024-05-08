#include <stdbool.h>
#include <stdlib.h>
#include <tree_sitter/api.h>

typedef size_t highlight_index_t;

enum HighlightSourceType {
	HIGHLIGHT_SOURCE_HIGHLIGHT,
	HIGHLIGHT_SOURCE_INJECTION,
	HIGHLIGHT_SOURCE_LOCAL,
	HIGHLIGHT_SOURCE_END,
};

struct HighlightSource {
	size_t len;
	const char *source;
};

struct HighlightConfig {
	const TSLanguage *language;

	const char *const *captures;
	size_t captures_count;

	struct HighlightSource source[HIGHLIGHT_SOURCE_END];
};

struct Highlight {
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

struct HighlightMarkerList {
	struct HighlightMarker *head;
	struct HighlightMarkerPool *pool;
};

struct HighlightMarkerPool {
	struct HighlightMarker *recycle;
	struct HighlightMarker **pools;
	size_t pool_size;
};

struct HighlightIterator {
	struct Highlight *highlight;
	TSQueryCursor *cursor;
	const TSTree *tree;
	uint_fast32_t current_offset;
	uint_fast32_t tree_completed_offset;
	uint_fast32_t end_offset;
	uint_fast32_t current_capture_id;
	struct HighlightMarkerPool marker_pool;
	struct HighlightMarkerList markers;
};

int highlight_config_init(
		struct HighlightConfig *config, const TSLanguage *language);

void highlight_config_highlight(
		struct HighlightConfig *config, const char *source, size_t source_len);

void highlight_config_injection(
		struct HighlightConfig *config, const char *source, size_t source_len);
void highlight_config_local(
		struct HighlightConfig *config, const char *source, size_t source_len);

void highlight_config_captures(
		struct HighlightConfig *config, const char *const *captures,
		size_t captures_len);

int highlight_config_cleanup(struct HighlightConfig *config);

int highlight_marker_pool_init(struct HighlightMarkerPool *pool);

int highlight_marker_pool_cleanup(struct HighlightMarkerPool *pool);

int highlight_marker_list_init(
		struct HighlightMarkerList *list, struct HighlightMarkerPool *pool);

int highlight_marker_list_cleanup(struct HighlightMarkerList *list);

struct HighlightMarker *highlight_marker_new(struct HighlightMarkerPool *pool);

int highlight_marker_recycle(
		struct HighlightMarker *marker, struct HighlightMarkerList *list);

int highlight_init(
		struct Highlight *highlight, const struct HighlightConfig *config);

int highlight_cleanup(struct Highlight *highlight);

int highlight_iterator_init(
		struct HighlightIterator *iterator, struct Highlight *highlight,
		const TSTree *tree);

bool highlight_iterator_next(
		struct HighlightIterator *iterator, struct HighlightEvent *event,
		int *err);

int highlight_iterator_cleanup(struct HighlightIterator *iterator);
