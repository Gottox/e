#define _XOPEN_SOURCE 600

#include <fcntl.h>
#include <pty.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <testlib.h>
#include <ttyui.h>
#include <unistd.h>

#define EXPECTED_INIT_SEQ "\x1b[?1049;1004;1003;1006h\x1b[?7l"
#define EXPECTED_EXIT_SEQ "\x1b[?1049;1004;1003;1006l\x1b[?7h"

static void
signal_probe(int signum) {
	(void)signum;
}

static void
expect_sequence(int fd, const char *expected) {
	char buffer[64] = {0};
	ssize_t nread = read(fd, buffer, sizeof(buffer));
	ASSERT_EQ((ssize_t)strlen(expected), nread);
	ASSERT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

static void
check_ttyui_init_and_cleanup() {
	int master_fd = -1;
	int slave_fd = -1;
	int rv = openpty(&master_fd, &slave_fd, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	int flags = fcntl(master_fd, F_GETFL);
	ASSERT_TRUE(flags >= 0);
	rv = fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);
	ASSERT_EQ(0, rv);

	int ui_fd = dup(slave_fd);
	ASSERT_TRUE(ui_fd >= 0);

	struct termios termios_before = {0};
	ASSERT_EQ(0, tcgetattr(slave_fd, &termios_before));

	void (*prior_sigwinch)(int) = signal(SIGWINCH, signal_probe);
	ASSERT_NE(SIG_ERR, prior_sigwinch);

	struct TtyUi ui = {0};
	rv = ttyui_init(&ui, ui_fd);
	ASSERT_TRUE(rv >= 0);

	expect_sequence(master_fd, EXPECTED_INIT_SEQ);

	rv = ttyui_cleanup(&ui);
	ASSERT_EQ(0, rv);

	expect_sequence(master_fd, EXPECTED_EXIT_SEQ);

	struct termios termios_after = {0};
	ASSERT_EQ(0, tcgetattr(slave_fd, &termios_after));
	ASSERT_EQ(
			0, memcmp(&termios_before, &termios_after, sizeof(termios_before)));

	struct sigaction sigact = {0};
	ASSERT_EQ(0, sigaction(SIGWINCH, NULL, &sigact));
	ASSERT_EQ(signal_probe, sigact.sa_handler);

	signal(SIGWINCH, prior_sigwinch);
	close(slave_fd);
	close(master_fd);
}

DECLARE_TESTS
NO_TEST(check_ttyui_init_and_cleanup)
END_TESTS
