#include <cextras/unicode.h>
#include <ttyui.h>

#define FOREGROUND "38"
#define BACKGROUND "48"

static bool
has_color(uint32_t color) {
	return (color >> 24) & 0xFF;
}

static int
draw_color(const struct TtyUi *ui, const char *mode, uint32_t color) {
	uint8_t r = (color >> 0) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = (color >> 16) & 0xFF;

	fprintf(ui->fd_file, ";%s;2;%u;%u;%u", mode, r, g, b);

	return 0;
}

int
ttyui_draw_options(struct TtyUi *ui, const struct TtyUiDrawOptions *options) {
	fputs("\033[0", ui->fd_file);

	if (has_color(options->fg_color)) {
		draw_color(ui, FOREGROUND, options->fg_color);
	}

	if (has_color(options->bg_color)) {
		draw_color(ui, BACKGROUND, options->bg_color);
	}

	if (options->bold) {
		fputs(";1", ui->fd_file);
	}

	if (options->italic) {
		fputs(";3", ui->fd_file);
	}

	if (options->underline) {
		fputs(";4", ui->fd_file);
	}

	if (options->inverse) {
		fputs(";7", ui->fd_file);
	}

	if (options->strikethrough) {
		fputs(";9", ui->fd_file);
	}

	if (options->overline) {
		fputs(";53", ui->fd_file);
	}

	fputc('m', ui->fd_file);

	return 0;
}

int
ttyui_draw(
		struct TtyUi *ui, const char *str, size_t len,
		const struct TtyUiDrawOptions *options, struct TtyUiState *state) {
	(void)state;

	ttyui_draw_options(ui, options);
	fwrite(str, 1, len, ui->fd_file);
	fputs("\033[0m", ui->fd_file);
	fflush(ui->fd_file);

	return 0;
}

int
ttyui_draw_eol(
		struct TtyUi *ui, const struct TtyUiDrawOptions *options,
		struct TtyUiState *state) {
	(void)state;
	ttyui_draw_options(ui, options);
	fputs("\033[0K", ui->fd_file);
	fflush(ui->fd_file);

	return 0;
}
