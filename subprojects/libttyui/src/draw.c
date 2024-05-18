#include <cextras/unicode.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <ttyui.h>

#define FOREGROUND "38"
#define BACKGROUND "48"
#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

struct {
	uint8_t xterm_number;
	uint32_t color;
} color_table[] = {
		{0, 0x000000}, /* Black (SYSTEM) */
		{1, 0x800000}, /* Maroon (SYSTEM) */
		{2, 0x008000}, /* Green (SYSTEM) */
		{3, 0x808000}, /* Olive (SYSTEM) */
		{4, 0x000080}, /* Navy (SYSTEM) */
		{5, 0x800080}, /* Purple (SYSTEM) */
		{6, 0x008080}, /* Teal (SYSTEM) */
		{7, 0xc0c0c0}, /* Silver (SYSTEM) */
		{8, 0x808080}, /* Grey (SYSTEM) */
		{9, 0xff0000}, /* Red (SYSTEM) */
		{10, 0x00ff00}, /* Lime (SYSTEM) */
		{11, 0xffff00}, /* Yellow (SYSTEM) */
		{12, 0x0000ff}, /* Blue (SYSTEM) */
		{13, 0xff00ff}, /* Fuchsia (SYSTEM) */
		{14, 0x00ffff}, /* Aqua (SYSTEM) */
		{15, 0xffffff}, /* White (SYSTEM) */
		{16, 0x000000}, /* Grey0 */
		{17, 0x00005f}, /* NavyBlue */
		{18, 0x000087}, /* DarkBlue */
		{19, 0x0000af}, /* Blue3 */
		{20, 0x0000d7}, /* Blue3 */
		{21, 0x0000ff}, /* Blue1 */
		{22, 0x005f00}, /* DarkGreen */
		{23, 0x005f5f}, /* DeepSkyBlue4 */
		{24, 0x005f87}, /* DeepSkyBlue4 */
		{25, 0x005faf}, /* DeepSkyBlue4 */
		{26, 0x005fd7}, /* DodgerBlue3 */
		{27, 0x005fff}, /* DodgerBlue2 */
		{28, 0x008700}, /* Green4 */
		{29, 0x00875f}, /* SpringGreen4 */
		{30, 0x008787}, /* Turquoise4 */
		{31, 0x0087af}, /* DeepSkyBlue3 */
		{32, 0x0087d7}, /* DeepSkyBlue3 */
		{33, 0x0087ff}, /* DodgerBlue1 */
		{34, 0x00af00}, /* Green3 */
		{35, 0x00af5f}, /* SpringGreen3 */
		{36, 0x00af87}, /* DarkCyan */
		{37, 0x00afaf}, /* LightSeaGreen */
		{38, 0x00afd7}, /* DeepSkyBlue2 */
		{39, 0x00afff}, /* DeepSkyBlue1 */
		{40, 0x00d700}, /* Green3 */
		{41, 0x00d75f}, /* SpringGreen3 */
		{42, 0x00d787}, /* SpringGreen2 */
		{43, 0x00d7af}, /* Cyan3 */
		{44, 0x00d7d7}, /* DarkTurquoise */
		{45, 0x00d7ff}, /* Turquoise2 */
		{46, 0x00ff00}, /* Green1 */
		{47, 0x00ff5f}, /* SpringGreen2 */
		{48, 0x00ff87}, /* SpringGreen1 */
		{49, 0x00ffaf}, /* MediumSpringGreen */
		{50, 0x00ffd7}, /* Cyan2 */
		{51, 0x00ffff}, /* Cyan1 */
		{52, 0x5f0000}, /* DarkRed */
		{53, 0x5f005f}, /* DeepPink4 */
		{54, 0x5f0087}, /* Purple4 */
		{55, 0x5f00af}, /* Purple4 */
		{56, 0x5f00d7}, /* Purple3 */
		{57, 0x5f00ff}, /* BlueViolet */
		{58, 0x5f5f00}, /* Orange4 */
		{59, 0x5f5f5f}, /* Grey37 */
		{60, 0x5f5f87}, /* MediumPurple4 */
		{61, 0x5f5faf}, /* SlateBlue3 */
		{62, 0x5f5fd7}, /* SlateBlue3 */
		{63, 0x5f5fff}, /* RoyalBlue1 */
		{64, 0x5f8700}, /* Chartreuse4 */
		{65, 0x5f875f}, /* DarkSeaGreen4 */
		{66, 0x5f8787}, /* PaleTurquoise4 */
		{67, 0x5f87af}, /* SteelBlue */
		{68, 0x5f87d7}, /* SteelBlue3 */
		{69, 0x5f87ff}, /* CornflowerBlue */
		{70, 0x5faf00}, /* Chartreuse3 */
		{71, 0x5faf5f}, /* DarkSeaGreen4 */
		{72, 0x5faf87}, /* CadetBlue */
		{73, 0x5fafaf}, /* CadetBlue */
		{74, 0x5fafd7}, /* SkyBlue3 */
		{75, 0x5fafff}, /* SteelBlue1 */
		{76, 0x5fd700}, /* Chartreuse3 */
		{77, 0x5fd75f}, /* PaleGreen3 */
		{78, 0x5fd787}, /* SeaGreen3 */
		{79, 0x5fd7af}, /* Aquamarine3 */
		{80, 0x5fd7d7}, /* MediumTurquoise */
		{81, 0x5fd7ff}, /* SteelBlue1 */
		{82, 0x5fff00}, /* Chartreuse2 */
		{83, 0x5fff5f}, /* SeaGreen2 */
		{84, 0x5fff87}, /* SeaGreen1 */
		{85, 0x5fffaf}, /* SeaGreen1 */
		{86, 0x5fffd7}, /* Aquamarine1 */
		{87, 0x5fffff}, /* DarkSlateGray2 */
		{88, 0x870000}, /* DarkRed */
		{89, 0x87005f}, /* DeepPink4 */
		{90, 0x870087}, /* DarkMagenta */
		{91, 0x8700af}, /* DarkMagenta */
		{92, 0x8700d7}, /* DarkViolet */
		{93, 0x8700ff}, /* Purple */
		{94, 0x875f00}, /* Orange4 */
		{95, 0x875f5f}, /* LightPink4 */
		{96, 0x875f87}, /* Plum4 */
		{97, 0x875faf}, /* MediumPurple3 */
		{98, 0x875fd7}, /* MediumPurple3 */
		{99, 0x875fff}, /* SlateBlue1 */
		{100, 0x878700}, /* Yellow4 */
		{101, 0x87875f}, /* Wheat4 */
		{102, 0x878787}, /* Grey53 */
		{103, 0x8787af}, /* LightSlateGrey */
		{104, 0x8787d7}, /* MediumPurple */
		{105, 0x8787ff}, /* LightSlateBlue */
		{106, 0x87af00}, /* Yellow4 */
		{107, 0x87af5f}, /* DarkOliveGreen3 */
		{108, 0x87af87}, /* DarkSeaGreen */
		{109, 0x87afaf}, /* LightSkyBlue3 */
		{110, 0x87afd7}, /* LightSkyBlue3 */
		{111, 0x87afff}, /* SkyBlue2 */
		{112, 0x87d700}, /* Chartreuse2 */
		{113, 0x87d75f}, /* DarkOliveGreen3 */
		{114, 0x87d787}, /* PaleGreen3 */
		{115, 0x87d7af}, /* DarkSeaGreen3 */
		{116, 0x87d7d7}, /* DarkSlateGray3 */
		{117, 0x87d7ff}, /* SkyBlue1 */
		{118, 0x87ff00}, /* Chartreuse1 */
		{119, 0x87ff5f}, /* LightGreen */
		{120, 0x87ff87}, /* LightGreen */
		{121, 0x87ffaf}, /* PaleGreen1 */
		{122, 0x87ffd7}, /* Aquamarine1 */
		{123, 0x87ffff}, /* DarkSlateGray1 */
		{124, 0xaf0000}, /* Red3 */
		{125, 0xaf005f}, /* DeepPink4 */
		{126, 0xaf0087}, /* MediumVioletRed */
		{127, 0xaf00af}, /* Magenta3 */
		{128, 0xaf00d7}, /* DarkViolet */
		{129, 0xaf00ff}, /* Purple */
		{130, 0xaf5f00}, /* DarkOrange3 */
		{131, 0xaf5f5f}, /* IndianRed */
		{132, 0xaf5f87}, /* HotPink3 */
		{133, 0xaf5faf}, /* MediumOrchid3 */
		{134, 0xaf5fd7}, /* MediumOrchid */
		{135, 0xaf5fff}, /* MediumPurple2 */
		{136, 0xaf8700}, /* DarkGoldenrod */
		{137, 0xaf875f}, /* LightSalmon3 */
		{138, 0xaf8787}, /* RosyBrown */
		{139, 0xaf87af}, /* Grey63 */
		{140, 0xaf87d7}, /* MediumPurple2 */
		{141, 0xaf87ff}, /* MediumPurple1 */
		{142, 0xafaf00}, /* Gold3 */
		{143, 0xafaf5f}, /* DarkKhaki */
		{144, 0xafaf87}, /* NavajoWhite3 */
		{145, 0xafafaf}, /* Grey69 */
		{146, 0xafafd7}, /* LightSteelBlue3 */
		{147, 0xafafff}, /* LightSteelBlue */
		{148, 0xafd700}, /* Yellow3 */
		{149, 0xafd75f}, /* DarkOliveGreen3 */
		{150, 0xafd787}, /* DarkSeaGreen3 */
		{151, 0xafd7af}, /* DarkSeaGreen2 */
		{152, 0xafd7d7}, /* LightCyan3 */
		{153, 0xafd7ff}, /* LightSkyBlue1 */
		{154, 0xafff00}, /* GreenYellow */
		{155, 0xafff5f}, /* DarkOliveGreen2 */
		{156, 0xafff87}, /* PaleGreen1 */
		{157, 0xafffaf}, /* DarkSeaGreen2 */
		{158, 0xafffd7}, /* DarkSeaGreen1 */
		{159, 0xafffff}, /* PaleTurquoise1 */
		{160, 0xd70000}, /* Red3 */
		{161, 0xd7005f}, /* DeepPink3 */
		{162, 0xd70087}, /* DeepPink3 */
		{163, 0xd700af}, /* Magenta3 */
		{164, 0xd700d7}, /* Magenta3 */
		{165, 0xd700ff}, /* Magenta2 */
		{166, 0xd75f00}, /* DarkOrange3 */
		{167, 0xd75f5f}, /* IndianRed */
		{168, 0xd75f87}, /* HotPink3 */
		{169, 0xd75faf}, /* HotPink2 */
		{170, 0xd75fd7}, /* Orchid */
		{171, 0xd75fff}, /* MediumOrchid1 */
		{172, 0xd78700}, /* Orange3 */
		{173, 0xd7875f}, /* LightSalmon3 */
		{174, 0xd78787}, /* LightPink3 */
		{175, 0xd787af}, /* Pink3 */
		{176, 0xd787d7}, /* Plum3 */
		{177, 0xd787ff}, /* Violet */
		{178, 0xd7af00}, /* Gold3 */
		{179, 0xd7af5f}, /* LightGoldenrod3 */
		{180, 0xd7af87}, /* Tan */
		{181, 0xd7afaf}, /* MistyRose3 */
		{182, 0xd7afd7}, /* Thistle3 */
		{183, 0xd7afff}, /* Plum2 */
		{184, 0xd7d700}, /* Yellow3 */
		{185, 0xd7d75f}, /* Khaki3 */
		{186, 0xd7d787}, /* LightGoldenrod2 */
		{187, 0xd7d7af}, /* LightYellow3 */
		{188, 0xd7d7d7}, /* Grey84 */
		{189, 0xd7d7ff}, /* LightSteelBlue1 */
		{190, 0xd7ff00}, /* Yellow2 */
		{191, 0xd7ff5f}, /* DarkOliveGreen1 */
		{192, 0xd7ff87}, /* DarkOliveGreen1 */
		{193, 0xd7ffaf}, /* DarkSeaGreen1 */
		{194, 0xd7ffd7}, /* Honeydew2 */
		{195, 0xd7ffff}, /* LightCyan1 */
		{196, 0xff0000}, /* Red1 */
		{197, 0xff005f}, /* DeepPink2 */
		{198, 0xff0087}, /* DeepPink1 */
		{199, 0xff00af}, /* DeepPink1 */
		{200, 0xff00d7}, /* Magenta2 */
		{201, 0xff00ff}, /* Magenta1 */
		{202, 0xff5f00}, /* OrangeRed1 */
		{203, 0xff5f5f}, /* IndianRed1 */
		{204, 0xff5f87}, /* IndianRed1 */
		{205, 0xff5faf}, /* HotPink */
		{206, 0xff5fd7}, /* HotPink */
		{207, 0xff5fff}, /* MediumOrchid1 */
		{208, 0xff8700}, /* DarkOrange */
		{209, 0xff875f}, /* Salmon1 */
		{210, 0xff8787}, /* LightCoral */
		{211, 0xff87af}, /* PaleVioletRed1 */
		{212, 0xff87d7}, /* Orchid2 */
		{213, 0xff87ff}, /* Orchid1 */
		{214, 0xffaf00}, /* Orange1 */
		{215, 0xffaf5f}, /* SandyBrown */
		{216, 0xffaf87}, /* LightSalmon1 */
		{217, 0xffafaf}, /* LightPink1 */
		{218, 0xffafd7}, /* Pink1 */
		{219, 0xffafff}, /* Plum1 */
		{220, 0xffd700}, /* Gold1 */
		{221, 0xffd75f}, /* LightGoldenrod2 */
		{222, 0xffd787}, /* LightGoldenrod2 */
		{223, 0xffd7af}, /* NavajoWhite1 */
		{224, 0xffd7d7}, /* MistyRose1 */
		{225, 0xffd7ff}, /* Thistle1 */
		{226, 0xffff00}, /* Yellow1 */
		{227, 0xffff5f}, /* LightGoldenrod1 */
		{228, 0xffff87}, /* Khaki1 */
		{229, 0xffffaf}, /* Wheat1 */
		{230, 0xffffd7}, /* Cornsilk1 */
		{231, 0xffffff}, /* Grey100 */
		{232, 0x080808}, /* Grey3 */
		{233, 0x121212}, /* Grey7 */
		{234, 0x1c1c1c}, /* Grey11 */
		{235, 0x262626}, /* Grey15 */
		{236, 0x303030}, /* Grey19 */
		{237, 0x3a3a3a}, /* Grey23 */
		{238, 0x444444}, /* Grey27 */
		{239, 0x4e4e4e}, /* Grey30 */
		{240, 0x585858}, /* Grey35 */
		{241, 0x626262}, /* Grey39 */
		{242, 0x6c6c6c}, /* Grey42 */
		{243, 0x767676}, /* Grey46 */
		{244, 0x808080}, /* Grey50 */
		{245, 0x8a8a8a}, /* Grey54 */
		{246, 0x949494}, /* Grey58 */
		{247, 0x9e9e9e}, /* Grey62 */
		{248, 0xa8a8a8}, /* Grey66 */
		{249, 0xb2b2b2}, /* Grey70 */
		{250, 0xbcbcbc}, /* Grey74 */
		{251, 0xc6c6c6}, /* Grey78 */
		{252, 0xd0d0d0}, /* Grey82 */
		{253, 0xdadada}, /* Grey85 */
		{254, 0xe4e4e4}, /* Grey89 */
		{255, 0xeeeeee}, /* Grey93 */
};

static uint16_t
manhatten_distance(uint32_t color1, uint32_t color2) {
	uint8_t r1 = (color1 >> 16) & 0xFF;
	uint8_t g1 = (color1 >> 8) & 0xFF;
	uint8_t b1 = color1 & 0xFF;

	uint8_t r2 = (color2 >> 16) & 0xFF;
	uint8_t g2 = (color2 >> 8) & 0xFF;
	uint8_t b2 = color2 & 0xFF;

	return abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
}

static int
draw_color_256(const struct TtyUi *ui, const char *mode, uint32_t color) {
	unsigned int min_distance = UINT_MAX;
	uint8_t xterm_number = 0;
	for (size_t i = 0; i < LENGTH(color_table); i++) {
		uint16_t distance = manhatten_distance(color, color_table[i].color);
		if (distance < min_distance) {
			min_distance = distance;
			xterm_number = color_table[i].xterm_number;
		}
	}

	fprintf(ui->fd_file, ";%s;5;%u", mode, xterm_number);

	return 0;
}

static int
draw_color_true(const struct TtyUi *ui, const char *mode, uint32_t color) {
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	int written = fprintf(ui->fd_file, ";%s;2;%u;%u;%u", mode, r, g, b);

	return written < 0 ? written : 0;
}

static int
draw_color(const struct TtyUi *ui, const char *mode, uint32_t color) {
	switch (ui->color_mode) {
	case TTYUI_COLOR_MODE_TRUE:
		return draw_color_true(ui, mode, color);
	case TTYUI_COLOR_MODE_256:
		return draw_color_256(ui, mode, color);
	case TTYUI_COLOR_MODE_OFF:
		break;
	}
	return 0;
}

static bool
has_color(uint32_t color) {
	return (color >> 24) & 0xFF;
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
