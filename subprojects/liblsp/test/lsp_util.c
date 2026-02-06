#include <lsp_util.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
test_classify_request(void) {
	json_object *id = json_object_new_int(1);
	json_object *req = lsp_request_new("initialize", NULL, id);

	ASSERT_EQ(lsp_message_classify(req), LSP_MESSAGE_REQUEST);

	json_object_put(req);
	json_object_put(id);
}

static void
test_classify_notification(void) {
	json_object *notif = lsp_notification_new("exit", NULL);

	ASSERT_EQ(lsp_message_classify(notif), LSP_MESSAGE_NOTIFICATION);

	json_object_put(notif);
}

static void
test_classify_response(void) {
	json_object *id = json_object_new_int(1);
	json_object *result = json_object_new_object();
	json_object *resp = lsp_response_new(id, result);

	ASSERT_EQ(lsp_message_classify(resp), LSP_MESSAGE_RESPONSE);

	json_object_put(resp);
	json_object_put(id);
}

static void
test_classify_error_response(void) {
	json_object *id = json_object_new_int(1);
	json_object *resp = lsp_response_error_new(id, -32600, "Invalid Request");

	ASSERT_EQ(lsp_message_classify(resp), LSP_MESSAGE_RESPONSE);

	json_object_put(resp);
	json_object_put(id);
}

static void
test_classify_invalid(void) {
	ASSERT_EQ(lsp_message_classify(NULL), LSP_MESSAGE_INVALID);

	json_object *empty = json_object_new_object();
	ASSERT_EQ(lsp_message_classify(empty), LSP_MESSAGE_INVALID);
	json_object_put(empty);
}

static void
test_response_new(void) {
	json_object *id = json_object_new_int(42);
	json_object *result = json_object_new_string("success");
	json_object *resp = lsp_response_new(id, result);

	ASSERT_STREQ(
		json_object_get_string(json_object_object_get(resp, "jsonrpc")), "2.0");
	ASSERT_EQ(
		json_object_get_int(json_object_object_get(resp, "id")), 42);
	ASSERT_STREQ(
		json_object_get_string(json_object_object_get(resp, "result")), "success");

	json_object_put(resp);
	json_object_put(id);
}

static void
test_response_error_new(void) {
	json_object *id = json_object_new_int(42);
	json_object *resp = lsp_response_error_new(id, -32601, "Method not found");

	ASSERT_STREQ(
		json_object_get_string(json_object_object_get(resp, "jsonrpc")), "2.0");

	json_object *err = json_object_object_get(resp, "error");
	ASSERT_NE(err, NULL);
	ASSERT_EQ(json_object_get_int(json_object_object_get(err, "code")), -32601);
	ASSERT_STREQ(
		json_object_get_string(json_object_object_get(err, "message")), "Method not found");

	json_object_put(resp);
	json_object_put(id);
}

DECLARE_TESTS
TEST(test_classify_request)
TEST(test_classify_notification)
TEST(test_classify_response)
TEST(test_classify_error_response)
TEST(test_classify_invalid)
TEST(test_response_new)
TEST(test_response_error_new)
END_TESTS
