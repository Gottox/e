#include <cextras/unicode.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ttyui.h>
#include <unistd.h>

#define TERMINAL_MODES \
	"1049;" /* Alternate Screen */ \
	"1004;" /* Report Focus Change */ \
	"1003;" /* Report Mouse Movement */ \
	"1006" /* Mouse Reporting Format Digits */

static const char INIT_SEQ[] = "\x1b[?" TERMINAL_MODES "h";
static const char EXIT_SEQ[] = "\x1b[?" TERMINAL_MODES "l";

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

	// TODO: document this
	tios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	tios.c_oflag &= ~(OPOST);
	tios.c_cflag |= (CS8);
	tios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;

	rv = tcsetattr(ui->fd, TCSAFLUSH, &tios);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

out:
	return rv;
}

int
ttyui_init(
		struct TtyUi *ui, int fd, ttyui_event_handler handler,
		void *user_data) {
	int rv = 0;

	ui->fd = fd;

	rv = init_terminal(ui);
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
out:
	return rv;
}
