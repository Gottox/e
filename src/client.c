#include <quickjs.h>
#include <rope.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <tree.h>
#include <ttyui.h>

int
read_file(struct Rope *r, const char *file_name) {
	size_t read_size = 0;
	uint8_t buffer[512] = {0};
	FILE *fp = fopen(file_name, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open file: %s\n", file_name);
		return 1;
	}

	while ((read_size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
		int rv = rope_append(r, buffer, read_size);
		if (rv < 0) {
			fprintf(stderr, "Failed to append to rope\n");
			goto out;
		}
	}

out:
	fclose(fp);
	return 0;
}

int
handle_tty(struct TtyUi *ui, struct TtyUiEvent *ev, void *user_data) {
	(void)ui;
	(void)user_data;
	int rv = 0;
	switch (ev->type) {
	case TTYUI_EVENT_KEY:
		// printf("Key: %s (%x)\n\r", ev->key.seq, ev->key.seq[0]);
		if (strlen(ev->key.seq) != 1) {
			break;
		}
		switch (ev->key.seq[0]) {
		case 'q':
			rv = -1;
			goto out;
		case '1':
			ttyui_draw(
					ui, "😃", 4,
					&(struct TtyUiDrawOptions){
							.bg = 0x01c0c0c0,
							.fg = 0x01000000,
					},
					NULL);
			ttyui_draw_eol(
					ui,
					&(struct TtyUiDrawOptions){
							.bg = 0x01c0c0c0,
							.fg = 0x01000000,
					},
					NULL);
			break;
		}
		break;
	case TTYUI_EVENT_CURSOR:
		// printf("Cursor: %02x (%c)\r\n", ev->cursor.direction,
		// ev->cursor.direction);
		break;
	case TTYUI_EVENT_MOUSE:
		// printf("Mouse: X%d Y%d B%d P%d\r\n", ev->mouse.x, ev->mouse.y,
		// ev->mouse.button, ev->mouse.pressed);
		break;
	case TTYUI_EVENT_FOCUS:
		// printf("Focus: %d\r\n", ev->focus.focus);
		break;
	case TTYUI_EVENT_RESIZE:
		// printf("Resize: C%d R%d\r\n", ui->columns, ui->rows);
		break;
	}

out:
	return rv;
}

void
draw_content(struct TtyUi *ui, struct Rope *rope) {
	(void)ui;
	(void)rope;
}

int
e_client(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int rv = 0;
	struct TtyUi ui = {0};
	struct Rope rope = {0};

	rv = ttyui_init(&ui, 0, handle_tty, NULL);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize TTY UI\n");
		goto out;
	}

	while (1) {
		rv = ttyui_poll(&ui);
		if (rv < 0) {
			fprintf(stderr, "Failed to poll TTY UI\n");
			goto out;
		}
		draw_content(&ui, &rope);
	}

out:
	ttyui_cleanup(&ui);

	return 0;
}
