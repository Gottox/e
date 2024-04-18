#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ttyui.h>
#include <unistd.h>

#define TERMINAL_MODES_ENABLE \
	"1049;" /* Alternate Screen */ \
	"1004;" /* Report Focus Change */ \
	"1003;" /* Report Mouse Movement */ \
	"1006" /* Mouse Reporting Format Digits */
#define TERMINAL_MODES_DISABLE "7" /* Autowrap Mode */

static const char INIT_SEQ[] = "\x1b[?" TERMINAL_MODES_ENABLE "h"
							   "\x1b[?" TERMINAL_MODES_DISABLE "l";
static const char EXIT_SEQ[] = "\x1b[?" TERMINAL_MODES_ENABLE "l"
							   "\x1b[?" TERMINAL_MODES_DISABLE "h";

static struct TtyUi *ttyui;

static int
update_size(struct TtyUi *ui) {
	struct winsize winsz;
	int rv = ioctl(0, TIOCGWINSZ, &winsz);
	if (rv < 0) {
		return rv;
	}
	ui->columns = winsz.ws_col;
	ui->rows = winsz.ws_row;

	return 0;
}
static void
handle_sigwinch(int signum) {
	if (signum != SIGWINCH) {
		return;
	}

	int rv = update_size(ttyui);
	if (rv < 0) {
		return;
	}

	struct TtyUiEvent event = {.type = TTYUI_EVENT_RESIZE};
	ttyui->handler(ttyui, &event, ttyui->user_data);
}

static int
init_terminal(struct TtyUi *ui) {
	struct termios tios = {0};
	int rv = 0;
	rv = tcgetattr(ui->fd, &ui->old_termios);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	memcpy(&tios, &ui->old_termios, sizeof(tios));

	/* no break, no CR to NL, no parity check, no strip char, no start/stop */
	tios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	/* disable output processing */
	tios.c_oflag &= ~(OPOST);
	/* 8 bits */
	tios.c_cflag |= (CS8);
	/* echo off, canonical off, no extended functions, no signal characters */
	tios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	/* return each byte, no delay */
	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;

	rv = tcsetattr(ui->fd, TCSAFLUSH, &tios);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	char *true_color = getenv("COLORTERM");
	if (true_color != NULL && strcmp(true_color, "truecolor") == 0) {
		ui->true_color = true;
	} else {
		ui->true_color = false;
	}

	rv = update_size(ui);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

static int
init_signals(struct TtyUi *ui) {
	void (*old_sigwinch)(int) = signal(SIGWINCH, handle_sigwinch);
	if (old_sigwinch == SIG_ERR) {
		return -errno;
	}

	ui->old_sigwinch = old_sigwinch;
	return 0;
}

int
ttyui_init(
		struct TtyUi *ui, int fd, ttyui_event_handler handler,
		void *user_data) {
	int rv = 0;

	ttyui = ui;

	ui->fd = fd;

	ui->fd_file = fdopen(ui->fd, "w");
	if (ui->fd_file == NULL) {
		rv = -errno;
		goto out;
	}

	rv = init_terminal(ui);
	if (rv < 0) {
		goto out;
	}

	rv = init_signals(ui);
	if (rv < 0) {
		goto out;
	}

	rv = write(ui->fd, INIT_SEQ, sizeof(INIT_SEQ) - 1);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	ui->handler = handler;
	ui->user_data = user_data;

out:
	return rv;
}

int
ttyui_cleanup(struct TtyUi *ui) {
	int rv = 0;
	rv = write(ui->fd, EXIT_SEQ, sizeof(EXIT_SEQ) - 1);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	rv = tcsetattr(ui->fd, TCSAFLUSH, &ui->old_termios);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	fclose(ui->fd_file);
	void (*sigwinch)(int) = signal(SIGWINCH, ui->old_sigwinch);
	if (sigwinch == SIG_ERR) {
		rv = -errno;
		goto out;
	}
	ttyui = NULL;
out:
	return rv;
}
