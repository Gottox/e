#include <lsp_util.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
test_c2s_method_lookup(void) {
	// Test known C2S methods
	ASSERT_EQ(lsp_c2s_method_lookup("initialize"), LSP_C2S_METHOD_INITIALIZE);
	ASSERT_EQ(lsp_c2s_method_lookup("shutdown"), LSP_C2S_METHOD_SHUTDOWN);
	ASSERT_EQ(lsp_c2s_method_lookup("textDocument/hover"), LSP_C2S_METHOD_TEXT_DOCUMENT__HOVER);
	ASSERT_EQ(lsp_c2s_method_lookup("textDocument/references"), LSP_C2S_METHOD_TEXT_DOCUMENT__REFERENCES);
	ASSERT_EQ(lsp_c2s_method_lookup("callHierarchy/incomingCalls"), LSP_C2S_METHOD_CALL_HIERARCHY__INCOMING_CALLS);

	// Test unknown method
	ASSERT_EQ(lsp_c2s_method_lookup("unknown/method"), LSP_C2S_METHOD_UNKNOWN);
	ASSERT_EQ(lsp_c2s_method_lookup(NULL), LSP_C2S_METHOD_UNKNOWN);
}

static void
test_s2c_method_lookup(void) {
	// Test known S2C methods
	ASSERT_EQ(lsp_s2c_method_lookup("window/showMessageRequest"), LSP_S2C_METHOD_WINDOW__SHOW_MESSAGE_REQUEST);
	ASSERT_EQ(lsp_s2c_method_lookup("client/registerCapability"), LSP_S2C_METHOD_CLIENT__REGISTER_CAPABILITY);
	ASSERT_EQ(lsp_s2c_method_lookup("workspace/applyEdit"), LSP_S2C_METHOD_WORKSPACE__APPLY_EDIT);

	// Test unknown method
	ASSERT_EQ(lsp_s2c_method_lookup("unknown/method"), LSP_S2C_METHOD_UNKNOWN);
	ASSERT_EQ(lsp_s2c_method_lookup(NULL), LSP_S2C_METHOD_UNKNOWN);

	// C2S method should not be found in S2C lookup
	ASSERT_EQ(lsp_s2c_method_lookup("initialize"), LSP_S2C_METHOD_UNKNOWN);
}

static void
test_c2s_method_names(void) {
	// Test C2S method name lookup
	ASSERT_STREQ(lsp_c2s_method_names[LSP_C2S_METHOD_INITIALIZE], "initialize");
	ASSERT_STREQ(lsp_c2s_method_names[LSP_C2S_METHOD_SHUTDOWN], "shutdown");
	ASSERT_STREQ(lsp_c2s_method_names[LSP_C2S_METHOD_TEXT_DOCUMENT__HOVER], "textDocument/hover");
	ASSERT_EQ(lsp_c2s_method_names[LSP_C2S_METHOD_UNKNOWN], NULL);
}

static void
test_s2c_method_names(void) {
	// Test S2C method name lookup
	ASSERT_STREQ(lsp_s2c_method_names[LSP_S2C_METHOD_WINDOW__SHOW_MESSAGE_REQUEST], "window/showMessageRequest");
	ASSERT_STREQ(lsp_s2c_method_names[LSP_S2C_METHOD_CLIENT__REGISTER_CAPABILITY], "client/registerCapability");
	ASSERT_EQ(lsp_s2c_method_names[LSP_S2C_METHOD_UNKNOWN], NULL);
}

static void
test_request_new(void) {
	json_object *id = json_object_new_int64(1);
	json_object *params = json_object_new_object();
	json_object_object_add(params, "foo", json_object_new_string("bar"));

	json_object *req = lsp_request_new("test/method", params, id);

	// Check jsonrpc version
	json_object *jsonrpc = json_object_object_get(req, "jsonrpc");
	ASSERT_NE(jsonrpc, NULL);
	ASSERT_STREQ(json_object_get_string(jsonrpc), "2.0");

	// Check method
	json_object *method_obj = json_object_object_get(req, "method");
	ASSERT_NE(method_obj, NULL);
	ASSERT_STREQ(json_object_get_string(method_obj), "test/method");

	// Check id
	json_object *id_obj = json_object_object_get(req, "id");
	ASSERT_NE(id_obj, NULL);
	ASSERT_EQ(json_object_get_int64(id_obj), 1);

	// Check params
	json_object *params_obj = json_object_object_get(req, "params");
	ASSERT_NE(params_obj, NULL);
	ASSERT_STREQ(json_object_get_string(json_object_object_get(params_obj, "foo")), "bar");

	json_object_put(id);
	json_object_put(req);
}

static void
test_request_new_no_params(void) {
	json_object *id = json_object_new_int64(2);

	json_object *req = lsp_request_new("shutdown", NULL, id);

	// Check no params field
	json_object *params_obj = json_object_object_get(req, "params");
	ASSERT_EQ(params_obj, NULL);

	json_object_put(id);
	json_object_put(req);
}

static void
test_shutdown_request(void) {
	json_object *id = json_object_new_int64(99);

	json_object *req = lsp_shutdown__request(id);

	// Check method
	json_object *method_obj = json_object_object_get(req, "method");
	ASSERT_NE(method_obj, NULL);
	ASSERT_STREQ(json_object_get_string(method_obj), "shutdown");

	// Check id
	json_object *id_obj = json_object_object_get(req, "id");
	ASSERT_NE(id_obj, NULL);
	ASSERT_EQ(json_object_get_int64(id_obj), 99);

	// No params
	json_object *params_obj = json_object_object_get(req, "params");
	ASSERT_EQ(params_obj, NULL);

	json_object_put(id);
	json_object_put(req);
}

static void
test_shutdown_read_request(void) {
	// Create a shutdown request
	json_object *req = json_object_new_object();
	json_object_object_add(req, "jsonrpc", json_object_new_string("2.0"));
	json_object_object_add(req, "method", json_object_new_string("shutdown"));
	json_object_object_add(req, "id", json_object_new_int64(42));

	// Read it back - for requests without params, returns the id directly
	json_object *id = lsp_shutdown__read_request(req);

	ASSERT_NE(id, NULL);
	ASSERT_TRUE(json_object_is_type(id, json_type_int));
	ASSERT_EQ(json_object_get_int64(id), 42);

	json_object_put(req);
}

DECLARE_TESTS
TEST(test_c2s_method_lookup)
TEST(test_s2c_method_lookup)
TEST(test_c2s_method_names)
TEST(test_s2c_method_names)
TEST(test_request_new)
TEST(test_request_new_no_params)
TEST(test_shutdown_request)
TEST(test_shutdown_read_request)
END_TESTS
