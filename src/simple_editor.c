#include <termbox2.h>

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	int rv = 0;
	struct tb_event e;
	tb_init();
	tb_set_input_mode(TB_INPUT_MOUSE | TB_INPUT_ESC);
	tb_sendf("\x1b[?1003;1006h", 1003, 1006);
	tb_present();

	tb_sendf("\x1b[?%d;%dl", 1003, 1006);

	while (rv == 0) {
		tb_poll_event(&e);
		if (e.type == TB_EVENT_KEY && e.key == TB_KEY_ESC) {
			rv = 1;
		}
	}

	tb_shutdown();

	return 0;
}
