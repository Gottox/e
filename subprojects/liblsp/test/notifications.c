#include <lsp_util.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
test_c2s_notification_lookup(void) {
	// Test known C2S notifications
	ASSERT_EQ(lsp_c2s_notification_lookup("initialized"), LSP_C2S_NOTIFICATION_INITIALIZED);
	ASSERT_EQ(lsp_c2s_notification_lookup("exit"), LSP_C2S_NOTIFICATION_EXIT);
	ASSERT_EQ(lsp_c2s_notification_lookup("textDocument/didOpen"), LSP_C2S_NOTIFICATION_TEXT_DOCUMENT__DID_OPEN);
	ASSERT_EQ(lsp_c2s_notification_lookup("textDocument/didClose"), LSP_C2S_NOTIFICATION_TEXT_DOCUMENT__DID_CLOSE);

	// "both" direction notifications should be found in C2S
	ASSERT_EQ(lsp_c2s_notification_lookup("$/progress"), LSP_C2S_NOTIFICATION_PROGRESS);
	ASSERT_EQ(lsp_c2s_notification_lookup("$/cancelRequest"), LSP_C2S_NOTIFICATION_CANCEL_REQUEST);

	// Test unknown notification
	ASSERT_EQ(lsp_c2s_notification_lookup("unknown/notification"), LSP_C2S_NOTIFICATION_UNKNOWN);
	ASSERT_EQ(lsp_c2s_notification_lookup(NULL), LSP_C2S_NOTIFICATION_UNKNOWN);
}

static void
test_s2c_notification_lookup(void) {
	// Test known S2C notifications
	ASSERT_EQ(lsp_s2c_notification_lookup("textDocument/publishDiagnostics"), LSP_S2C_NOTIFICATION_TEXT_DOCUMENT__PUBLISH_DIAGNOSTICS);

	// "both" direction notifications should be found in S2C
	ASSERT_EQ(lsp_s2c_notification_lookup("$/progress"), LSP_S2C_NOTIFICATION_PROGRESS);
	ASSERT_EQ(lsp_s2c_notification_lookup("$/cancelRequest"), LSP_S2C_NOTIFICATION_CANCEL_REQUEST);

	// Test unknown notification
	ASSERT_EQ(lsp_s2c_notification_lookup("unknown/notification"), LSP_S2C_NOTIFICATION_UNKNOWN);
	ASSERT_EQ(lsp_s2c_notification_lookup(NULL), LSP_S2C_NOTIFICATION_UNKNOWN);

	// C2S-only notification should not be found in S2C lookup
	ASSERT_EQ(lsp_s2c_notification_lookup("initialized"), LSP_S2C_NOTIFICATION_UNKNOWN);
}

static void
test_c2s_notification_names(void) {
	// Test C2S notification name lookup
	ASSERT_STREQ(lsp_c2s_notification_names[LSP_C2S_NOTIFICATION_INITIALIZED], "initialized");
	ASSERT_STREQ(lsp_c2s_notification_names[LSP_C2S_NOTIFICATION_EXIT], "exit");
	ASSERT_STREQ(lsp_c2s_notification_names[LSP_C2S_NOTIFICATION_TEXT_DOCUMENT__DID_OPEN], "textDocument/didOpen");
	ASSERT_EQ(lsp_c2s_notification_names[LSP_C2S_NOTIFICATION_UNKNOWN], NULL);
}

static void
test_s2c_notification_names(void) {
	// Test S2C notification name lookup
	ASSERT_STREQ(lsp_s2c_notification_names[LSP_S2C_NOTIFICATION_TEXT_DOCUMENT__PUBLISH_DIAGNOSTICS], "textDocument/publishDiagnostics");
	ASSERT_EQ(lsp_s2c_notification_names[LSP_S2C_NOTIFICATION_UNKNOWN], NULL);
}

static void
test_notification_new(void) {
	json_object *params = json_object_new_object();
	json_object_object_add(params, "foo", json_object_new_string("bar"));

	json_object *notif = lsp_notification_new("test/notification", params);

	// Check jsonrpc version
	json_object *jsonrpc = json_object_object_get(notif, "jsonrpc");
	ASSERT_NE(jsonrpc, NULL);
	ASSERT_STREQ(json_object_get_string(jsonrpc), "2.0");

	// Check method
	json_object *method_obj = json_object_object_get(notif, "method");
	ASSERT_NE(method_obj, NULL);
	ASSERT_STREQ(json_object_get_string(method_obj), "test/notification");

	// Check no id field (notifications don't have id)
	json_object *id_obj = json_object_object_get(notif, "id");
	ASSERT_EQ(id_obj, NULL);

	// Check params
	json_object *params_obj = json_object_object_get(notif, "params");
	ASSERT_NE(params_obj, NULL);
	ASSERT_STREQ(json_object_get_string(json_object_object_get(params_obj, "foo")), "bar");

	json_object_put(notif);
}

static void
test_notification_new_no_params(void) {
	json_object *notif = lsp_notification_new("exit", NULL);

	// Check no params field
	json_object *params_obj = json_object_object_get(notif, "params");
	ASSERT_EQ(params_obj, NULL);

	// Check no id field
	json_object *id_obj = json_object_object_get(notif, "id");
	ASSERT_EQ(id_obj, NULL);

	json_object_put(notif);
}

static void
test_exit_notification(void) {
	json_object *notif = lsp_exit__notification();

	// Check method
	json_object *method_obj = json_object_object_get(notif, "method");
	ASSERT_NE(method_obj, NULL);
	ASSERT_STREQ(json_object_get_string(method_obj), "exit");

	// No id field (notifications don't have id)
	json_object *id_obj = json_object_object_get(notif, "id");
	ASSERT_EQ(id_obj, NULL);

	// No params
	json_object *params_obj = json_object_object_get(notif, "params");
	ASSERT_EQ(params_obj, NULL);

	json_object_put(notif);
}

static void
test_initialized_notification(void) {
	// Create InitializedParams
	struct LspInitializedParams params = {0};
	int rv = lsp_initialized_params__init(&params);
	ASSERT_EQ(rv, LSP_OK);

	json_object *notif = lsp_initialized__notification(&params);

	// Check method
	json_object *method_obj = json_object_object_get(notif, "method");
	ASSERT_NE(method_obj, NULL);
	ASSERT_STREQ(json_object_get_string(method_obj), "initialized");

	// No id field
	json_object *id_obj = json_object_object_get(notif, "id");
	ASSERT_EQ(id_obj, NULL);

	// Has params (even if empty)
	json_object *params_obj = json_object_object_get(notif, "params");
	ASSERT_NE(params_obj, NULL);

	json_object_put(notif);
	lsp_initialized_params__cleanup(&params);
}

static void
test_did_open_notification(void) {
	// Build a textDocument/didOpen notification manually
	json_object *notif = json_object_new_object();
	json_object_object_add(notif, "jsonrpc", json_object_new_string("2.0"));
	json_object_object_add(notif, "method", json_object_new_string("textDocument/didOpen"));

	json_object *params = json_object_new_object();
	json_object *text_doc = json_object_new_object();
	json_object_object_add(text_doc, "uri", json_object_new_string("file:///test.c"));
	json_object_object_add(text_doc, "languageId", json_object_new_string("c"));
	json_object_object_add(text_doc, "version", json_object_new_int(1));
	json_object_object_add(text_doc, "text", json_object_new_string("int main() {}"));
	json_object_object_add(params, "textDocument", text_doc);
	json_object_object_add(notif, "params", params);

	// Read it back
	struct LspDidOpenTextDocumentParams read_params = {0};
	int err = lsp_text_document__did_open__read_notification(notif, &read_params);
	ASSERT_EQ(err, LSP_OK);

	// Check text document - first get the nested struct
	struct LspTextDocumentItem text_document = {0};
	err = lsp_did_open_text_document_params__text_document(&read_params, &text_document);
	ASSERT_EQ(err, LSP_OK);

	ASSERT_STREQ(lsp_text_document_item__uri(&text_document), "file:///test.c");
	ASSERT_STREQ(lsp_text_document_item__language_id(&text_document), "c");
	ASSERT_EQ(lsp_text_document_item__version(&text_document), 1);
	ASSERT_STREQ(lsp_text_document_item__text(&text_document), "int main() {}");

	lsp_text_document_item__cleanup(&text_document);
	lsp_did_open_text_document_params__cleanup(&read_params);
	json_object_put(notif);
}

DECLARE_TESTS
TEST(test_c2s_notification_lookup)
TEST(test_s2c_notification_lookup)
TEST(test_c2s_notification_names)
TEST(test_s2c_notification_names)
TEST(test_notification_new)
TEST(test_notification_new_no_params)
TEST(test_exit_notification)
TEST(test_initialized_notification)
TEST(test_did_open_notification)
END_TESTS
