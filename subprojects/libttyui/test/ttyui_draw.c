#include <pty.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <testlib.h>
#include <ttyui.h>
#include <unistd.h>

struct TtyUiHarness {
        struct TtyUi ui;
        int master_fd;
        int stdin_backup;
        char *colorterm;
        char *no_color;
};

static int
noop_handler(struct TtyUi *ui, struct TtyUiEvent *event, void *user_data) {
        (void)ui;
        (void)event;
        (void)user_data;

        return 0;
}

static size_t
read_available(int fd, char *buf, size_t buf_len) {
        size_t total = 0;

        while (total < buf_len) {
                fd_set read_set;
                FD_ZERO(&read_set);
                FD_SET(fd, &read_set);

                struct timeval tv = {.tv_sec = 0, .tv_usec = 100000};
                int ready = select(fd + 1, &read_set, NULL, NULL, &tv);
                if (ready <= 0 || !FD_ISSET(fd, &read_set)) {
                        break;
                }

                ssize_t rv = read(fd, buf + total, buf_len - total);
                if (rv <= 0) {
                        break;
                }

                total += (size_t)rv;
        }

        return total;
}

static void
setup_ttyui(struct TtyUiHarness *harness) {
        int slave_fd = -1;
        int rv = openpty(&harness->master_fd, &slave_fd, NULL, NULL, NULL);
        ASSERT_EQ(0, rv);

        harness->stdin_backup = dup(STDIN_FILENO);
        ASSERT_TRUE(harness->stdin_backup >= 0);

        struct winsize ws = {.ws_col = 80, .ws_row = 24};
        rv = ioctl(slave_fd, TIOCSWINSZ, &ws);
        ASSERT_EQ(0, rv);

        rv = dup2(slave_fd, STDIN_FILENO);
        ASSERT_EQ(0, rv);

        const char *colorterm = getenv("COLORTERM");
        harness->colorterm = colorterm != NULL ? strdup(colorterm) : NULL;

        const char *no_color = getenv("NO_COLOR");
        harness->no_color = no_color != NULL ? strdup(no_color) : NULL;

        setenv("COLORTERM", "truecolor", 1);
        unsetenv("NO_COLOR");

        rv = ttyui_init(&harness->ui, slave_fd, noop_handler, NULL);
        ASSERT_TRUE(rv >= 0);

        char drain[128];
        read_available(harness->master_fd, drain, sizeof(drain));
}

static void
teardown_ttyui(struct TtyUiHarness *harness) {
        ttyui_cleanup(&harness->ui);

        char drain[128];
        read_available(harness->master_fd, drain, sizeof(drain));

        close(harness->master_fd);
        int rv = dup2(harness->stdin_backup, STDIN_FILENO);
        ASSERT_EQ(0, rv);
        close(harness->stdin_backup);

        if (harness->colorterm != NULL) {
                setenv("COLORTERM", harness->colorterm, 1);
                free(harness->colorterm);
        } else {
                unsetenv("COLORTERM");
        }

        if (harness->no_color != NULL) {
                setenv("NO_COLOR", harness->no_color, 1);
                free(harness->no_color);
        } else {
                unsetenv("NO_COLOR");
        }
}

static void
test_ttyui_draw_truecolor_styles(void) {
        struct TtyUiHarness harness = {0};
        setup_ttyui(&harness);

        struct TtyUiDrawOptions options = {
                .fg = 0xFF010203,
                .bg = 0xFF040506,
                .flags = TTYUI_DRAW_BOLD | TTYUI_DRAW_UNDERLINE |
                         TTYUI_DRAW_ITALIC | TTYUI_DRAW_INVERSE |
                         TTYUI_DRAW_STRIKETHROUGH | TTYUI_DRAW_OVERLINE,
        };

        const char *text = "Hello";
        ttyui_draw(&harness.ui, text, strlen(text), &options, NULL);

        char output[256];
        size_t len = read_available(harness.master_fd, output, sizeof(output) - 1);
        if (len >= sizeof(output)) {
                len = sizeof(output) - 1;
        }
        output[len] = '\0';

        const char *expected = "\033[0;38;2;1;2;3;48;2;4;5;6;1;3;4;7;9;53mHello\033[0m";
        ASSERT_EQ(strlen(expected), len);
        ASSERT_EQ(0, memcmp(expected, output, len));

        teardown_ttyui(&harness);
}

static void
test_ttyui_draw_eol_background_and_flags(void) {
        struct TtyUiHarness harness = {0};
        setup_ttyui(&harness);

        struct TtyUiDrawOptions options = {
                .fg = 0,
                .bg = 0xFF0A0B0C,
                .flags = TTYUI_DRAW_UNDERLINE | TTYUI_DRAW_OVERLINE |
                         TTYUI_DRAW_INVERSE,
        };

        ttyui_draw_eol(&harness.ui, &options, NULL);

        char output[256];
        size_t len = read_available(harness.master_fd, output, sizeof(output) - 1);
        if (len >= sizeof(output)) {
                len = sizeof(output) - 1;
        }
        output[len] = '\0';

        const char *expected = "\033[0;48;2;10;11;12;4;7;53m\033[0K";
        ASSERT_EQ(strlen(expected), len);
        ASSERT_EQ(0, memcmp(expected, output, len));

        teardown_ttyui(&harness);
}

DECLARE_TESTS
TEST(test_ttyui_draw_truecolor_styles)
TEST(test_ttyui_draw_eol_background_and_flags)
END_TESTS
