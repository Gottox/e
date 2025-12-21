#include <cextras/unicode.h>
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ttyui.h>
#include <unistd.h>

#define CSI_PARAMETER_COUNT 10

struct Csi {
	int parameter[CSI_PARAMETER_COUNT];
	char command;
	char modifier;
};

static int
report_simple_key(
		struct TtyUiEvent *event, char *input_buffer,
		size_t *input_buffer_len) {
	ssize_t seq_len =
			cx_utf8_bidx((uint8_t *)input_buffer, TTYUI_INPUT_BUFFER_SIZE, 1);
	if (seq_len < 0) {
		// invalid utf8 sequence, treat as a single byte
		seq_len = 1;
	}

	event->type = TTYUI_EVENT_KEY;
	memcpy(event->key.seq, input_buffer, seq_len);
	event->key.len = seq_len;
	memmove(input_buffer, &input_buffer[seq_len], *input_buffer_len - seq_len);
	*input_buffer_len -= seq_len;

	return 0;
}

static int
parse_csi(struct Csi *csi, const char *str, const size_t size) {
	int param_index = 0;
	if (size < 2) {
		return -TTYUI_ERROR_CSI_PARSE;
	}

	if (str[0] != '\x1b' || str[1] != '[') {
		return -TTYUI_ERROR_CSI_PARSE;
	}

	memset(csi, 0, sizeof(struct Csi));

	size_t index = 2;
	switch (str[index]) {
	case '<':
		csi->modifier = str[index];
		index++;
		break;
	default:
		break;
	}

	for (; index < size; index++) {
		const char c = str[index];
		if (c >= '0' && c <= '9') {
			if (param_index < CSI_PARAMETER_COUNT) {
				const int nbr = c - '0';
				csi->parameter[param_index] =
						csi->parameter[param_index] * 10 + nbr;
			}
		} else if (c == ';') {
			param_index++;
		} else {
			csi->command = c;
			break;
		}
	}

	if (csi->command == 0) {
		return -TTYUI_ERROR_CSI_PARSE;
	}
	return index;
}

static int
report_csi(
		struct TtyUiEvent *event, char *input_buffer,
		size_t *input_buffer_len) {
	struct Csi csi = {0};
	int rv = parse_csi(&csi, input_buffer, *input_buffer_len);
	if (rv < 0) {
		return rv;
	}
	size_t csi_len = rv;
	rv = 0;

	switch (csi.command) {
	case TTYUI_CURSOR_UP:
	case TTYUI_CURSOR_DOWN:
	case TTYUI_CURSOR_RIGHT:
	case TTYUI_CURSOR_LEFT:
	case TTYUI_CURSOR_HOME:
	case TTYUI_CURSOR_END:
		event->type = TTYUI_EVENT_CURSOR;
		event->key.seq[0] = csi.command;
		event->key.len = 1;
		break;
	case '~':
		switch (csi.parameter[0]) {
		case TTYUI_CURSOR_INSERT:
		case TTYUI_CURSOR_DELETE:
		case TTYUI_CURSOR_PAGE_UP:
		case TTYUI_CURSOR_PAGE_DOWN:
			event->type = TTYUI_EVENT_CURSOR;
			event->key.seq[0] = csi.parameter[0];
			event->key.len = 1;
			break;
		default:
			// Unknown CSI sequence. ignore.
			break;
		}
		break;
	case 'I':
	case 'O':
		event->type = TTYUI_EVENT_FOCUS;
		event->focus.focus = csi.command == 'I';
		break;
	case 'M':
	case 'm':
		if (csi.modifier == '<') {
			event->type = TTYUI_EVENT_MOUSE;
			event->mouse.button = csi.parameter[0];
			event->mouse.y = csi.parameter[1] - 1;
			event->mouse.x = csi.parameter[2] - 1;
			event->mouse.pressed = csi.command == 'M';
		}
		break;
	default:
		// Unknown CSI sequence. ignore.
		break;
	}

	memmove(input_buffer, &input_buffer[csi_len + 1],
			*input_buffer_len - csi_len - 1);
	*input_buffer_len -= csi_len + 1;

	return 0;
}

static int
process_input_buffer(struct TtyUi *ui, struct TtyUiEvent *event) {
	if (ui->input_buffer_len == 0) {
		return 0;
	}

	int rv = report_csi(event, ui->input_buffer, &ui->input_buffer_len);

	if (rv < 0 && ui->input_buffer_len > 0) {
		rv = report_simple_key(event, ui->input_buffer, &ui->input_buffer_len);
	}

	return rv;
}

static int
read_tty_input(struct TtyUi *ui) {
	ssize_t rv =
			read(ui->fd, &ui->input_buffer[ui->input_buffer_len],
				 TTYUI_INPUT_BUFFER_SIZE - ui->input_buffer_len);
	if (rv < 0) {
		return -errno;
	}

	ui->input_buffer_len += (size_t)rv;
	return 0;
}

static int
handle_resize_pipe(struct TtyUi *ui, struct TtyUiEvent *event) {
	char ch;
	ssize_t rv = read(ui->sigwinch_pipe[0], &ch, 1);
	if (rv < 0) {
		return -errno;
	}

	rv = ttyui_update_size(ui);
	if (rv < 0) {
		return rv;
	}

	event->type = TTYUI_EVENT_RESIZE;
	return 0;
}

int
ttyui_event_next(struct TtyUi *ui, struct TtyUiEvent *event) {
	int rv = 0;
	memset(event, 0, sizeof(struct TtyUiEvent));

	if (ui->input_buffer_len > 0) {
		return process_input_buffer(ui, event);
	}

	struct pollfd pollfds[2];
	nfds_t nfds = 0;

	int resize_idx = -1;
	if (ui->sigwinch_pipe[0] >= 0) {
		resize_idx = (int)nfds;
		pollfds[nfds].fd = ui->sigwinch_pipe[0];
		pollfds[nfds].events = POLLIN;
		nfds++;
	}

	int tty_idx = -1;
	if (ui->fd >= 0) {
		tty_idx = (int)nfds;
		pollfds[nfds].fd = ui->fd;
		pollfds[nfds].events = POLLIN;
		nfds++;
	}

	if (nfds == 0) {
		return -EINVAL;
	}

	do {
		rv = poll(pollfds, nfds, -1);
	} while (rv < 0 && errno == EINTR);

	if (rv < 0) {
		return -errno;
	}

	if (resize_idx >= 0 &&
		(pollfds[resize_idx].revents & (POLLIN | POLLERR | POLLHUP))) {
		return handle_resize_pipe(ui, event);
	}

	if (tty_idx >= 0 &&
		(pollfds[tty_idx].revents & (POLLIN | POLLERR | POLLHUP))) {
		rv = read_tty_input(ui);
		if (rv < 0) {
			return rv;
		}

		return process_input_buffer(ui, event);
	}

	return 0;
}

int
ttyui_event_fds(struct TtyUi *ui, int *pty_fd, int *sig_fd) {
	if (ui->fd >= 0) {
		*pty_fd = ui->fd;
	} else {
		*pty_fd = -1;
	}

	if (ui->sigwinch_pipe[0] >= 0) {
		*sig_fd = ui->sigwinch_pipe[0];
	} else {
		*sig_fd = -1;
	}

	return 0;
}
