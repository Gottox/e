#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>

#define MAX_OPTIONS 16

struct TtyUi;
struct TtyUiEvent;

typedef int (*ttyui_event_handler)(
		struct TtyUi *ttyui, struct TtyUiEvent *event, void *user_data);

enum TtyUiCursorKey {
	TTYUI_CURSOR_UP = 'A',
	TTYUI_CURSOR_DOWN = 'B',
	TTYUI_CURSOR_RIGHT = 'C',
	TTYUI_CURSOR_LEFT = 'D',
	TTYUI_CURSOR_HOME = 'H',
	TTYUI_CURSOR_END = 'F',
	TTYUI_CURSOR_INSERT = 2,
	TTYUI_CURSOR_DELETE = 3,
	TTYUI_CURSOR_PAGE_UP = 5,
	TTYUI_CURSOR_PAGE_DOWN = 6,
};

enum TtyUiEventType {
	TTYUI_EVENT_KEY,
	TTYUI_EVENT_CURSOR,
	TTYUI_EVENT_MOUSE,
	TTYUI_EVENT_FOCUS,
	TTYUI_EVENT_RESIZE,
};

enum TtyUiModifier {
	TTYUI_MODIFIER_NONE = 0,
	TTYUI_MODIFIER_SHIFT = 1,
	TTYUI_MODIFIER_ALT = 2,
	TTYUI_MODIFIER_CTRL = 4,
};

enum TtyColorMode {
	TTYUI_COLOR_MODE_OFF,
	TTYUI_COLOR_MODE_256,
	TTYUI_COLOR_MODE_TRUE,
};

struct TtyUiEvent {
	enum TtyUiEventType type;
	union {
		struct {
			// 8 bytes so we can store UTF-8 sequences.
			char seq[8];
			size_t len;
			enum TtyUiModifier modifier;
		} key;
		struct {
			char direction;
		} cursor;
		struct {
			bool focus;
		} focus;
		struct {
			unsigned int x;
			unsigned int y;
			unsigned int button;
			bool pressed;
		} mouse;
	};
};

struct TtyUiState {
	unsigned int columns;
	unsigned int rows;
};

struct TtyUiDrawOptions {
	uint32_t fg_color;
	uint32_t bg_color;
	bool bold;
	bool underline;
	bool strikethrough;
	bool overline;
	bool italic;
	bool inverse;
	bool strike;
};

struct TtyUi {
	int fd;
	FILE *fd_file;
	struct termios old_termios;
	void (*old_sigwinch)(int);
	ttyui_event_handler handler;
	unsigned int columns;
	unsigned int rows;
	void *user_data;
	enum TtyColorMode color_mode;
};

/***************************************
 * ttyui.c
 */
int ttyui_init(
		struct TtyUi *ttyui, int fd, ttyui_event_handler event_cb,
		void *user_data);

int ttyui_cleanup(struct TtyUi *ttyui);

/***************************************
 * event.c
 */

int ttyui_process(struct TtyUi *ttyui);

/***************************************
 * draw.c
 */

int ttyui_draw(
		struct TtyUi *ttyui, const char *str, size_t len,
		const struct TtyUiDrawOptions *options, struct TtyUiState *state);

int ttyui_draw_eol(
		struct TtyUi *ttyui, const struct TtyUiDrawOptions *options,
		struct TtyUiState *state);
