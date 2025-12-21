#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#define TTYUI_INPUT_BUFFER_SIZE 128

#define MAX_OPTIONS 16

enum TtyUiError {
	TTYUI_ERROR_SUCCESS = 0,
	TTYUI_ERROR_BEGIN = 1 << 8,
	TTYUI_ERROR_CSI_PARSE = 1 << 8,
};

struct TtyUi;
struct TtyUiEvent;


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
	TTYUI_EVENT_NONE,
	TTYUI_EVENT_KEY,
	TTYUI_EVENT_CURSOR,
	TTYUI_EVENT_MOUSE,
	TTYUI_EVENT_FOCUS,
	TTYUI_EVENT_RESIZE,
};

enum TtyUiModifier {
	TTYUI_MODIFIER_NONE = 0,
	TTYUI_MODIFIER_SHIFT = 1 << 0,
	TTYUI_MODIFIER_ALT = 1 << 1,
	TTYUI_MODIFIER_CTRL = 1 << 2,
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

enum TtyUiDrawOptionsFlags {
	TTYUI_DRAW_BOLD = 1 << 0,
	TTYUI_DRAW_UNDERLINE = 1 << 1,
	TTYUI_DRAW_STRIKETHROUGH = 1 << 2,
	TTYUI_DRAW_OVERLINE = 1 << 3,
	TTYUI_DRAW_ITALIC = 1 << 4,
	TTYUI_DRAW_INVERSE = 1 << 5,
	TTYUI_DRAW_STRIKE = 1 << 6,
};

struct TtyUiDrawOptions {
	uint32_t fg;
	uint32_t bg;
	uint8_t flags;
};

struct TtyUi {
	int fd;
	int sigwinch_pipe[2];
	FILE *fd_file;
	struct termios old_termios;
	void (*old_sigwinch)(int);
	unsigned int columns;
	unsigned int rows;
	enum TtyColorMode color_mode;
	char input_buffer[TTYUI_INPUT_BUFFER_SIZE];
	size_t input_buffer_len;
};

/***************************************
 * ttyui.c
 */
int ttyui_init(struct TtyUi *ttyui, int fd);
int ttyui_cleanup(struct TtyUi *ttyui);
int ttyui_update_size(struct TtyUi *ttyui);

/***************************************
 * cursor.c
 */
int ttyui_move_cursor(struct TtyUi *ttyui, unsigned int row, unsigned int col);
int ttyui_reset_cursor(struct TtyUi *ttyui);
int ttyui_clear_screen(struct TtyUi *ttyui);
int ttyui_hide_cursor(struct TtyUi *ttyui);
int ttyui_show_cursor(struct TtyUi *ttyui);
int ttyui_flush(struct TtyUi *ttyui);

/***************************************
 * event.c
 */

int ttyui_event_next(struct TtyUi *ttyui, struct TtyUiEvent *event);
int ttyui_event_fds(struct TtyUi *ttyui, int *pty_fd, int *sig_fd);

/***************************************
 * draw.c
 */

int ttyui_draw(
		struct TtyUi *ttyui, const char *str, size_t len,
		const struct TtyUiDrawOptions *options, struct TtyUiState *state);

int ttyui_draw_eol(
		struct TtyUi *ttyui, const struct TtyUiDrawOptions *options,
		struct TtyUiState *state);

int ttyui_draw_scroll_region(
		struct TtyUi *ui, int top, int bottom, struct TtyUiState *state);
