#include <pty.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <testlib.h>
#include <ttyui.h>
#include <unistd.h>

struct EventCapture {
	struct TtyUiEvent events[8];
	size_t count;
};

static int
capture_handler(
		struct TtyUi *ui, struct TtyUiEvent *event,
		struct EventCapture *capture) {
	(void)ui;

	ASSERT_TRUE(
			capture->count <
			(sizeof(capture->events) / sizeof(capture->events[0])));
	capture->events[capture->count++] = *event;

	return 0;
}

static void
set_raw_mode(int fd) {
	struct termios tio;
	int rv = tcgetattr(fd, &tio);
	ASSERT_EQ(0, rv);

	cfmakeraw(&tio);

	rv = tcsetattr(fd, TCSANOW, &tio);
	ASSERT_EQ(0, rv);
}

static void
write_and_process(
		int master_fd, struct TtyUi *ui, struct EventCapture *capture,
		const char *seq, size_t len, size_t expected_events) {
	struct TtyUiEvent event = {0};
	ssize_t written = write(master_fd, seq, len);
	ASSERT_EQ((ssize_t)len, written);

	size_t target = capture->count + expected_events;
	while (capture->count < target) {
		int rv = ttyui_event_next(ui, &event);
		ASSERT_EQ(0, rv);
		capture_handler(ui, &event, capture);
	}

	ASSERT_EQ(target, capture->count);
}

static void
init_ttyui(
		struct TtyUi *ui, int *master_fd, int *slave_fd,
		struct EventCapture *capture) {
	int rv = openpty(master_fd, slave_fd, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	set_raw_mode(*slave_fd);

	ui->fd = *slave_fd;
	ui->sigwinch_pipe[0] = -1;
	ui->sigwinch_pipe[1] = -1;
	ui->input_buffer_len = 0;
}

static void
test_ttyui_csi_sequences() {
	int master_fd = -1;
	int slave_fd = -1;
	struct TtyUi ui = {0};
	struct EventCapture capture = {0};

	init_ttyui(&ui, &master_fd, &slave_fd, &capture);

	const char seq[] = "\x1b[A\x1b[<0;5;10M\x1b[I\x1b[O";
	write_and_process(master_fd, &ui, &capture, seq, sizeof(seq) - 1, 4);

	ASSERT_EQ((size_t)4, capture.count);

	const struct TtyUiEvent *cursor = &capture.events[0];
	ASSERT_EQ(TTYUI_EVENT_CURSOR, cursor->type);
	ASSERT_EQ('A', cursor->key.seq[0]);
	ASSERT_EQ((size_t)1, cursor->key.len);

	const struct TtyUiEvent *mouse = &capture.events[1];
	ASSERT_EQ(TTYUI_EVENT_MOUSE, mouse->type);
	ASSERT_EQ(0u, mouse->mouse.button);
	ASSERT_EQ(9u, mouse->mouse.x);
	ASSERT_EQ(4u, mouse->mouse.y);
	ASSERT_TRUE(mouse->mouse.pressed);

	const struct TtyUiEvent *focus_in = &capture.events[2];
	ASSERT_EQ(TTYUI_EVENT_FOCUS, focus_in->type);
	ASSERT_TRUE(focus_in->focus.focus);

	const struct TtyUiEvent *focus_out = &capture.events[3];
	ASSERT_EQ(TTYUI_EVENT_FOCUS, focus_out->type);
	ASSERT_FALSE(focus_out->focus.focus);

	close(master_fd);
	close(slave_fd);
}

static void
test_ttyui_utf8_key_sequence() {
	int master_fd = -1;
	int slave_fd = -1;
	struct TtyUi ui = {0};
	struct EventCapture capture = {0};

	init_ttyui(&ui, &master_fd, &slave_fd, &capture);

	const char euro_symbol[] = "\u20ac";
	write_and_process(
			master_fd, &ui, &capture, euro_symbol, strlen(euro_symbol), 1);

	ASSERT_EQ((size_t)1, capture.count);

	const struct TtyUiEvent *key = &capture.events[0];
	ASSERT_EQ(TTYUI_EVENT_KEY, key->type);
	ASSERT_EQ(strlen(euro_symbol), key->key.len);
	ASSERT_EQ(0, memcmp(euro_symbol, key->key.seq, key->key.len));

	close(master_fd);
	close(slave_fd);
}

DECLARE_TESTS
TEST(test_ttyui_csi_sequences)
TEST(test_ttyui_utf8_key_sequence)
END_TESTS
