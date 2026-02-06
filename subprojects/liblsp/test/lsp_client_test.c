#include <errno.h>
#include <json.h>
#include <lsp.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>
#include <unistd.h>

static void
test_spawn_cat(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	const char *argv[] = {"/bin/cat", NULL};
	int rv = lsp_spawn(&lsp, argv);
	ASSERT_EQ(rv, 0);

	ASSERT_NOT_NULL(lsp.sender);
	ASSERT_GE(lsp.receiver_fd, 0);
	ASSERT_GT(lsp.pid, 0);

	lsp_cleanup(&lsp);
}

static void
test_send_receive(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	// Use cat as an echo server
	const char *argv[] = {"/bin/cat", NULL};
	int rv = lsp_spawn(&lsp, argv);
	ASSERT_EQ(rv, 0);

	// Send a message
	json_object *msg = json_object_new_object();
	json_object_object_add(msg, "jsonrpc", json_object_new_string("2.0"));
	json_object_object_add(msg, "method", json_object_new_string("test"));
	json_object_object_add(msg, "id", json_object_new_int(1));

	rv = lsp_send(&lsp, msg);
	ASSERT_EQ(rv, 0);

	// Close sender to signal EOF and trigger cat to flush
	fclose(lsp.sender);
	lsp.sender = NULL;

	// Read the echoed message
	rv = lsp_recv_buffer_read(&lsp.recv_buf, lsp.receiver_fd);
	while (rv == -EAGAIN) {
		rv = lsp_recv_buffer_read(&lsp.recv_buf, lsp.receiver_fd);
	}
	ASSERT_EQ(rv, 0);

	json_object *received = lsp_recv_buffer_extract(&lsp.recv_buf);
	ASSERT_NOT_NULL(received);

	// Verify content
	json_object *method = json_object_object_get(received, "method");
	ASSERT_NOT_NULL(method);
	ASSERT_STREQ(json_object_get_string(method), "test");

	json_object_put(received);
	json_object_put(msg);
	lsp_cleanup(&lsp);
}

static int test_notif_handler_called = 0;

static int
test_log_message_handler(struct LspLogMessageParams *params, void *userdata) {
	(void)params;
	(void)userdata;
	test_notif_handler_called = 1;
	return 0;
}

static void
test_client_process_notification(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	// Create a pipe to feed data
	int pipefd[2];
	ASSERT_EQ(pipe(pipefd), 0);

	lsp.receiver_fd = pipefd[0];

	// Write a log message notification
	const char *log_msg =
			"Content-Length: 81\r\n\r\n"
			"{\"jsonrpc\":\"2.0\",\"method\":\"window/logMessage\","
			"\"params\":{\"type\":1,\"message\":\"hi\"}}";
	write(pipefd[1], log_msg, strlen(log_msg));
	close(pipefd[1]);

	struct LspClientNotificationCallbacks notif_cbs = {0};
	notif_cbs.window__log_message = test_log_message_handler;

	test_notif_handler_called = 0;

	int rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_notif_handler_called, 1);

	close(pipefd[0]);
	lsp_cleanup(&lsp);
}

static void
test_eof_sets_not_running(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	// Create a pipe and close write end immediately
	int pipefd[2];
	ASSERT_EQ(pipe(pipefd), 0);
	close(pipefd[1]); // Close write end

	lsp.receiver_fd = pipefd[0];

	int rv = lsp_client_process(&lsp, NULL, NULL, NULL);
	ASSERT_EQ(rv, -ECONNRESET);
	ASSERT_EQ(lsp.running, false);

	close(pipefd[0]);
	lsp_cleanup(&lsp);
}

static void
test_lsp_notify(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	// Use cat as echo server
	const char *argv[] = {"/bin/cat", NULL};
	int rv = lsp_spawn(&lsp, argv);
	ASSERT_EQ(rv, 0);

	// Create a notification using the generated function
	struct LspInitializedParams params = {0};
	ASSERT_EQ(lsp_initialized_params__init(&params), 0);

	// Use lsp_notify (via generated send function internally creates json)
	json_object *notif = lsp_initialized__notification(&params);
	rv = lsp_notify(&lsp, notif);
	ASSERT_EQ(rv, 0);

	lsp_initialized_params__cleanup(&params);

	// Close sender and read back
	fclose(lsp.sender);
	lsp.sender = NULL;

	rv = lsp_recv_buffer_read(&lsp.recv_buf, lsp.receiver_fd);
	while (rv == -EAGAIN) {
		rv = lsp_recv_buffer_read(&lsp.recv_buf, lsp.receiver_fd);
	}
	ASSERT_EQ(rv, 0);

	json_object *received = lsp_recv_buffer_extract(&lsp.recv_buf);
	ASSERT_NOT_NULL(received);

	// Verify it's the initialized notification
	json_object *method = json_object_object_get(received, "method");
	ASSERT_NOT_NULL(method);
	ASSERT_STREQ(json_object_get_string(method), "initialized");

	json_object_put(received);
	lsp_cleanup(&lsp);
}

static int test_response_callback_called = 0;
static int test_response_had_result = 0;

static void
test_shutdown_callback(struct LspResponseError *error, void *userdata) {
	(void)error;
	test_response_callback_called = 1;
	test_response_had_result = (userdata != NULL);
}

static void
test_lsp_request_response(void) {
	struct Lsp lsp = {0};
	ASSERT_EQ(lsp_init(&lsp, NULL, NULL), 0);

	// Create pipes - we'll manually feed the response
	int send_pipe[2];
	int recv_pipe[2];
	ASSERT_EQ(pipe(send_pipe), 0);
	ASSERT_EQ(pipe(recv_pipe), 0);

	lsp.sender = fdopen(send_pipe[1], "w");
	ASSERT_NOT_NULL(lsp.sender);
	lsp.receiver_fd = recv_pipe[0];

	// Send a shutdown request with callback
	test_response_callback_called = 0;
	test_response_had_result = 0;

	int rv = lsp_shutdown__send(&lsp, test_shutdown_callback, (void *)1);
	ASSERT_EQ(rv, 0);

	// Read the request to get the actual ID
	char buf[1024];
	ssize_t n = read(send_pipe[0], buf, sizeof(buf) - 1);
	ASSERT_GT(n, 0);
	buf[n] = '\0';

	// Find the body (after \r\n\r\n)
	char *body = strstr(buf, "\r\n\r\n");
	ASSERT_NOT_NULL(body);
	body += 4;

	// Parse to get the ID
	json_object *req = json_tokener_parse(body);
	ASSERT_NOT_NULL(req);
	json_object *id_obj = json_object_object_get(req, "id");
	ASSERT_NOT_NULL(id_obj);
	int64_t id = json_object_get_int64(id_obj);
	json_object_put(req);

	// Build the response
	char resp_body[128];
	snprintf(resp_body, sizeof(resp_body),
			"{\"jsonrpc\":\"2.0\",\"id\":%ld,\"result\":null}", id);
	char response[256];
	snprintf(response, sizeof(response),
			"Content-Length: %zu\r\n\r\n%s", strlen(resp_body), resp_body);

	ssize_t written = write(recv_pipe[1], response, strlen(response));
	ASSERT_EQ((size_t)written, strlen(response));
	close(recv_pipe[1]);

	// Process the response
	rv = lsp_client_process(&lsp, NULL, NULL, NULL);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_response_callback_called, 1);
	ASSERT_EQ(test_response_had_result, 1); // userdata was passed

	close(send_pipe[0]);
	close(recv_pipe[0]);
	lsp_cleanup(&lsp);
}

DECLARE_TESTS
TEST(test_spawn_cat)
TEST(test_send_receive)
TEST(test_client_process_notification)
TEST(test_eof_sets_not_running)
TEST(test_lsp_notify)
TEST(test_lsp_request_response)
END_TESTS
