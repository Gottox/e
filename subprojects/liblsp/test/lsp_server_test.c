#include <errno.h>
#include <lsp.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>
#include <unistd.h>

static void
test_init_cleanup(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);

	ASSERT_EQ(lsp.sender, NULL);
	ASSERT_EQ(lsp.receiver_fd, -1);
	ASSERT_EQ(lsp.running, true);

	lsp_cleanup(&lsp);
}

static void
test_stdio(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);
	lsp_stdio(&lsp);

	ASSERT_EQ(lsp.sender, stdout);
	ASSERT_EQ(lsp.receiver_fd, STDIN_FILENO);

	// Don't cleanup - we don't want to close stdin/stdout
}

static void
test_next_id(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);

	uint64_t id1 = lsp_next_id(&lsp);
	uint64_t id2 = lsp_next_id(&lsp);
	uint64_t id3 = lsp_next_id(&lsp);

	// IDs should be unique
	ASSERT_NE(id1, id2);
	ASSERT_NE(id2, id3);
	ASSERT_NE(id1, id3);

	lsp_cleanup(&lsp);
}

static int test_exit_handler_called = 0;

static int
test_exit_handler(void *userdata) {
	(void)userdata;
	test_exit_handler_called = 1;
	return 0;
}

static void
test_server_process_exit(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);

	// Create a pipe to feed data
	int pipefd[2];
	ASSERT_EQ(pipe(pipefd), 0);

	lsp.receiver_fd = pipefd[0];

	// Write an exit notification
	const char *exit_msg =
			"Content-Length: 33\r\n\r\n"
			"{\"jsonrpc\":\"2.0\",\"method\":\"exit\"}";
	write(pipefd[1], exit_msg, strlen(exit_msg));
	close(pipefd[1]);

	struct LspServerNotificationCallbacks notif_cbs = {0};
	notif_cbs.exit = test_exit_handler;

	test_exit_handler_called = 0;

	int rv = lsp_server_process(&lsp, NULL, &notif_cbs, NULL);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(lsp.running, false);
	ASSERT_EQ(test_exit_handler_called, 1);

	close(pipefd[0]);
	lsp_cleanup(&lsp);
}

static int test_init_handler_called = 0;
static int
test_init_handler(
		struct LspInitializeParams *params, struct LspInitializeResult *result,
		struct LspResponseError *error, void *userdata) {
	(void)error;
	(void)userdata;
	test_init_handler_called = 1;
	// Check we got params
	ASSERT_NOT_NULL(params);
	// Set a minimal result
	result->json = json_object_new_object();
	json_object_object_add(
			result->json, "capabilities", json_object_new_object());
	return 0;
}

static void
test_server_process_request(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);

	// Create pipes for input/output
	int in_pipe[2];
	int out_pipe[2];
	ASSERT_EQ(pipe(in_pipe), 0);
	ASSERT_EQ(pipe(out_pipe), 0);

	lsp.receiver_fd = in_pipe[0];
	lsp.sender = fdopen(out_pipe[1], "w");

	// Write an initialize request
	const char *init_msg =
			"Content-Length: 75\r\n\r\n"
			"{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\","
			"\"params\":{\"capabilities\":{}}}";
	write(in_pipe[1], init_msg, strlen(init_msg));
	close(in_pipe[1]);

	struct LspServerRequestCallbacks req_cbs = {0};
	req_cbs.initialize = test_init_handler;

	test_init_handler_called = 0;

	int rv = lsp_server_process(&lsp, &req_cbs, NULL, NULL);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_init_handler_called, 1);

	// Read response
	char buf[1024] = {0};
	ssize_t n = read(out_pipe[0], buf, sizeof(buf) - 1);
	ASSERT_GT(n, 0);

	// Should contain Content-Length header and result
	ASSERT_NOT_NULL(strstr(buf, "Content-Length:"));
	ASSERT_NOT_NULL(strstr(buf, "\"result\""));

	close(in_pipe[0]);
	close(out_pipe[0]);
	fclose(lsp.sender);
	lsp.sender = NULL;
	lsp_cleanup(&lsp);
}

static void
test_server_partial_message(void) {
	struct Lsp lsp = {0};
	lsp_init(&lsp);

	// Create a pipe
	int pipefd[2];
	ASSERT_EQ(pipe(pipefd), 0);

	lsp.receiver_fd = pipefd[0];

	// Write partial message (just headers, no body yet)
	const char *partial = "Content-Length: 33\r\n\r\n{\"jsonrpc\":";
	write(pipefd[1], partial, strlen(partial));

	// First read should return EAGAIN (partial)
	int rv = lsp_server_process(&lsp, NULL, NULL, NULL);
	ASSERT_EQ(rv, -EAGAIN);

	// Complete the message
	const char *rest = "\"2.0\",\"method\":\"exit\"}";
	write(pipefd[1], rest, strlen(rest));

	struct LspServerNotificationCallbacks notif_cbs = {0};
	rv = lsp_server_process(&lsp, NULL, &notif_cbs, NULL);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(lsp.running, false);

	close(pipefd[0]);
	close(pipefd[1]);
	lsp_cleanup(&lsp);
}

DECLARE_TESTS
TEST(test_init_cleanup)
TEST(test_stdio)
TEST(test_next_id)
TEST(test_server_process_exit)
TEST(test_server_process_request)
TEST(test_server_partial_message)
END_TESTS
