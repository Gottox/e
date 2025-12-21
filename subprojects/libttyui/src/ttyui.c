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

int
ttyui_update_size(struct TtyUi *ui) {
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
	if (signum != SIGWINCH || ttyui == NULL) {
		return;
	}

	if (ttyui->sigwinch_pipe[1] < 0) {
		return;
	}

	ssize_t rv = 0;
	do {
		rv = write(ttyui->sigwinch_pipe[1], "w", 1);
	} while (rv < 0 && errno == EINTR);
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

	const char *true_color = getenv("COLORTERM");
	const char *no_color = getenv("NO_COLOR");
	if (no_color != NULL && no_color[0] != '\0') {
		ui->color_mode = TTYUI_COLOR_MODE_OFF;
	} else if (true_color != NULL && strcmp(true_color, "truecolor") == 0) {
		ui->color_mode = TTYUI_COLOR_MODE_TRUE;
	} else {
		ui->color_mode = TTYUI_COLOR_MODE_256;
	}

	rv = ttyui_update_size(ui);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

static void
close_signal_pipe(struct TtyUi *ui) {
	if (ui->sigwinch_pipe[0] >= 0) {
		close(ui->sigwinch_pipe[0]);
		ui->sigwinch_pipe[0] = -1;
	}

	if (ui->sigwinch_pipe[1] >= 0) {
		close(ui->sigwinch_pipe[1]);
		ui->sigwinch_pipe[1] = -1;
	}
}

static int
init_signals(struct TtyUi *ui) {
	int rv = pipe(ui->sigwinch_pipe);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	void (*old_sigwinch)(int) = signal(SIGWINCH, handle_sigwinch);
	if (old_sigwinch == SIG_ERR) {
		rv = -errno;
		goto out;
	}

	ui->old_sigwinch = old_sigwinch;

out:
	if (rv < 0) {
		close_signal_pipe(ui);
	}
	return rv;
}

int
ttyui_init(struct TtyUi *ui, int fd) {
	int rv = 0;

	ttyui = ui;

	ui->fd = fd;
	ui->sigwinch_pipe[0] = -1;
	ui->sigwinch_pipe[1] = -1;
	ui->input_buffer_len = 0;

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

out:
	if (rv < 0) {
		close_signal_pipe(ui);
		ttyui = NULL;
	}
	return rv;
}

int
ttyui_cleanup(struct TtyUi *ui) {
	int rv = 0;
	if (ui == NULL || ui->fd_file == NULL) {
		return 0;
	}

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
	close_signal_pipe(ui);
	ttyui = NULL;
out:
	return rv;
}

int
ttyui_move_cursor(struct TtyUi *ui, unsigned int row, unsigned int col) {
	int rv = fprintf(ui->fd_file, "\033[%u;%uH", row + 1, col + 1);
	if (rv < 0) {
		return -errno;
	}
	return 0;
}

int
ttyui_reset_cursor(struct TtyUi *ui) {
	int rv = fputs("\033[H", ui->fd_file);
	if (rv < 0) {
		return -errno;
	}
	return 0;
}

int
ttyui_clear_screen(struct TtyUi *ui) {
	int rv = fputs("\033[2J", ui->fd_file);
	if (rv < 0) {
		return -errno;
	}
	return 0;
}

int
ttyui_hide_cursor(struct TtyUi *ui) {
	int rv = fputs("\033[?25l", ui->fd_file);
	if (rv < 0) {
		return -errno;
	}
	return 0;
}

int
ttyui_show_cursor(struct TtyUi *ui) {
	int rv = fputs("\033[?25h", ui->fd_file);
	if (rv < 0) {
		return -errno;
	}
	return 0;
}

int
ttyui_flush(struct TtyUi *ui) {
	int rv = fflush(ui->fd_file);
	if (rv != 0) {
		return -errno;
	}
	return 0;
}
