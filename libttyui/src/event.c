#include <cextras/unicode.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <ttyui.h>
#include <unistd.h>

#define CSI_PARAMETER_COUNT 10
#define INPUT_BUFFER_SIZE 128

struct Csi {
	int parameter[CSI_PARAMETER_COUNT];
	char command;
	char modifier;
};

static int
report_simple_key(
		struct TtyUi *ui, char *input_buffer, size_t *input_buffer_len) {
	struct TtyUiEvent event = {0};
	ssize_t seq_len =
			cx_utf8_bidx((uint8_t *)input_buffer, INPUT_BUFFER_SIZE, 1);
	if (seq_len < 0) {
		// invalid utf8 sequence, treat as a single byte
		seq_len = 1;
	}

	event.type = TTYUI_EVENT_KEY;
	memcpy(event.key.seq, input_buffer, seq_len);
	event.key.len = seq_len;
	memmove(input_buffer, &input_buffer[seq_len], *input_buffer_len - seq_len);
	*input_buffer_len -= seq_len;

	return ui->handler(ui, &event, ui->user_data);
}

static int
parse_csi(struct Csi *csi, const char *str, const size_t size) {
	int param_index = 0;
	if (size < 2) {
		return -1;
	}

	if (str[0] != '\x1b' || str[1] != '[') {
		return -1;
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
		return -1;
	} else {
		return index;
	}
}

static int
report_csi(struct TtyUi *ui, char *input_buffer, size_t *input_buffer_len) {
	struct Csi csi = {0};
	struct TtyUiEvent event = {0};
	int rv = parse_csi(&csi, input_buffer, *input_buffer_len);
	if (rv < 0) {
		return rv;
	}
	size_t csi_len = rv;
	rv = 0;

	switch (csi.command) {
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'H':
	case 'F':
		event.type = TTYUI_EVENT_CURSOR;
		event.key.seq[0] = csi.command;
		event.key.len = 1;
		break;
	case '~':
		switch (csi.parameter[0]) {
		case TTYUI_CURSOR_INSERT:
		case TTYUI_CURSOR_DELETE:
		case TTYUI_CURSOR_PAGE_UP:
		case TTYUI_CURSOR_PAGE_DOWN:
			event.type = TTYUI_EVENT_CURSOR;
			event.key.seq[0] = csi.parameter[0];
			event.key.len = 1;
			break;
		default:
			// Unknown CSI sequence. ignore.
			break;
		}
		break;
	case 'I':
	case 'O':
		event.type = TTYUI_EVENT_FOCUS;
		event.focus.focus = csi.command == 'I';
		break;
	case 'M':
	case 'm':
		if (csi.modifier == '<') {
			event.type = TTYUI_EVENT_MOUSE;
			event.mouse.button = csi.parameter[0];
			event.mouse.y = csi.parameter[1] - 1;
			event.mouse.x = csi.parameter[2] - 1;
			event.mouse.pressed = csi.command == 'M';
		}
		break;
	default:
		// Unknown CSI sequence. ignore.
		break;
	}

	memmove(input_buffer, &input_buffer[csi_len + 1],
			*input_buffer_len - csi_len - 1);
	*input_buffer_len -= csi_len + 1;

	return ui->handler(ui, &event, ui->user_data);
}

int
ttyui_process(struct TtyUi *ui) {
	int rv = 0;

	char input_buffer[INPUT_BUFFER_SIZE] = {0};
	size_t input_buffer_len = 0;

	rv = read(ui->fd, &input_buffer, INPUT_BUFFER_SIZE);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	input_buffer_len = rv;
	rv = 0;

	while (input_buffer_len > 0) {
		rv = report_csi(ui, input_buffer, &input_buffer_len);

		if (rv < 0) {
			rv = report_simple_key(ui, input_buffer, &input_buffer_len);
			goto out;
		}
	}

out:
	return rv;
}
