#include <stdbool.h>
#include <stddef.h>
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
	TTYUI_EVENT_EOF,
};

enum TtyUiModifier {
	TTYUI_MODIFIER_NONE = 0,
	TTYUI_MODIFIER_SHIFT = 1,
	TTYUI_MODIFIER_ALT = 2,
	TTYUI_MODIFIER_CTRL = 4,
};

struct TtyUiEvent {
	enum TtyUiEventType type;
	union {
		struct {
			// 8 bytes so we can store UTF-8.
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

struct TtyUi {
	int fd;
	char input_buffer[128];
	size_t input_buffer_len;
	struct termios old_termios;
	ttyui_event_handler handler;
	void *user_data;
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
