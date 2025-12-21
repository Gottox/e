#include <errno.h>
#include <rope.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttyui.h>
#include <unistd.h>

struct Editor {
	struct TtyUi ui;
	struct TtyUiEvent ev;
	struct Rope rope;
	struct RopeCursor cursor;
	struct RopeRange line_range;
	rope_index_t top_line;
	bool running;
	bool needs_render;
};

static int
load_file(struct Editor *ed, const char *path) {
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		return -errno;
	}

	uint8_t buffer[1024];
	size_t nread = 0;
	int rv = 0;
	while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
		rv = rope_append(&ed->rope, buffer, nread);
		if (rv < 0) {
			break;
		}
	}

	fclose(fp);
	return rv;
}

static void
clamp_view(struct Editor *ed) {
	unsigned int height = ed->ui.rows;
	if (ed->cursor.line < ed->top_line) {
		ed->top_line = ed->cursor.line;
	}
	if (height > 0 && ed->cursor.line >= ed->top_line + (rope_index_t)height) {
		ed->top_line = ed->cursor.line - height + 1;
	}
}

static void
render_line(
		struct Editor *ed, rope_index_t line_no, unsigned int row,
		struct TtyUiDrawOptions *opts) {
	int rv = rope_range_line(&ed->line_range, line_no);
	if (rv < 0) {
		ttyui_move_cursor(&ed->ui, row, 0);
		ttyui_draw_eol(&ed->ui, opts, NULL);
		return;
	}

	char *line = rope_range_to_str(&ed->line_range, 0);
	if (line == NULL) {
		ttyui_move_cursor(&ed->ui, row, 0);
		ttyui_draw_eol(&ed->ui, opts, NULL);
		return;
	}

	size_t len = strcspn(line, "\n");
	if (len > ed->ui.columns) {
		len = ed->ui.columns;
	}

	ttyui_move_cursor(&ed->ui, row, 0);
	ttyui_draw(&ed->ui, line, len, opts, NULL);
	if (len < ed->ui.columns) {
		ttyui_draw_eol(&ed->ui, opts, NULL);
	}

	free(line);
}

static int
render(struct Editor *ed) {
	struct TtyUiDrawOptions opts = {0};

	ttyui_hide_cursor(&ed->ui);
	ttyui_clear_screen(&ed->ui);

	for (unsigned int row = 0; row < ed->ui.rows; row++) {
		render_line(ed, ed->top_line + row, row, &opts);
	}

	int rv = 0;
	rope_index_t cursor_row = 0;
	if (ed->cursor.line >= ed->top_line) {
		cursor_row = ed->cursor.line - ed->top_line;
	}
	if (cursor_row >= ed->ui.rows) {
		cursor_row = ed->ui.rows ? ed->ui.rows - 1 : 0;
	}
	unsigned int cursor_col = ed->cursor.column;
	if (cursor_col >= ed->ui.columns) {
		cursor_col = ed->ui.columns ? ed->ui.columns - 1 : 0;
	}
	rv = ttyui_move_cursor(&ed->ui, (unsigned int)cursor_row, cursor_col);
	if (rv < 0) {
		return rv;
	}
	ttyui_show_cursor(&ed->ui);

	ed->needs_render = false;
	return ttyui_flush(&ed->ui);
}

static int
handle_cursor_move(struct Editor *ed, struct TtyUiEvent ev) {
	int rv = 0;
	switch (ev.cursor.direction) {
	case TTYUI_CURSOR_UP:
		if (ed->cursor.line > 0) {
			rv = rope_cursor_move_to(
					&ed->cursor, ed->cursor.line - 1, ed->cursor.column);
		}
		break;
	case TTYUI_CURSOR_DOWN:
		rv = rope_cursor_move_to(
				&ed->cursor, ed->cursor.line + 1, ed->cursor.column);
		break;
	case TTYUI_CURSOR_LEFT:
		rv = rope_cursor_move(&ed->cursor, -1);
		break;
	case TTYUI_CURSOR_RIGHT:
		rv = rope_cursor_move(&ed->cursor, 1);
		break;
	default:
		break;
	}

	if (rv >= 0) {
		clamp_view(ed);
		ed->needs_render = true;
	}

	return rv;
}

static int
handle_event(struct Editor *ed) {
	int rv = 0;

	switch (ed->ev.type) {
	case TTYUI_EVENT_KEY:
		if (ed->ev.key.len == 1 &&
			(ed->ev.key.seq[0] == 'q' || ed->ev.key.seq[0] == 27)) {
			ed->running = false;
		}
		break;
	case TTYUI_EVENT_CURSOR:
		rv = handle_cursor_move(ed, ed->ev);
		break;
	case TTYUI_EVENT_RESIZE:
		clamp_view(ed);
		ed->needs_render = true;
		break;
	case TTYUI_EVENT_FOCUS:
	case TTYUI_EVENT_MOUSE:
	case TTYUI_EVENT_NONE:
	default:
		break;
	}

	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	struct Editor ed = {0};

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		rv = -1;
		goto out;
	}

	rv = ttyui_init(&ed.ui, STDIN_FILENO);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize ttyui\n");
		goto out;
	}

	rv = rope_init(&ed.rope);
	if (rv < 0) {
		fprintf(stderr, "Failed to init rope\n");
		goto out;
	}

	rv = rope_cursor_init(&ed.cursor, &ed.rope);
	if (rv < 0) {
		fprintf(stderr, "Failed to init cursor\n");
		goto out;
	}

	rv = rope_range_init(&ed.line_range, &ed.rope);
	if (rv < 0) {
		fprintf(stderr, "Failed to init range\n");
		goto out;
	}

	rv = load_file(&ed, argv[1]);
	if (rv < 0) {
		fprintf(stderr, "Failed to load file: %s\n", argv[1]);
		goto out;
	}

	ed.running = true;
	ed.needs_render = true;
	clamp_view(&ed);

	while (ed.running) {
		if (ed.needs_render) {
			rv = render(&ed);
			if (rv < 0) {
				break;
			}
		}

		rv = ttyui_event_next(&ed.ui, &ed.ev);
		if (rv < 0) {
			break;
		}
		rv = handle_event(&ed);
		if (rv < 0) {
			break;
		}
	}

out:
	ttyui_cleanup(&ed.ui);
	rope_range_cleanup(&ed.line_range);
	rope_cursor_cleanup(&ed.cursor);
	rope_cleanup(&ed.rope);

	return rv < 0 ? 1 : 0;
}
