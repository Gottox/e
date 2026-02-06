#include <lsp_initialization.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

/*
 * The initialization module only checks if callback pointers are non-NULL.
 * We use a dummy placeholder value cast to the correct type to satisfy
 * the compiler while providing a non-NULL value for testing.
 */
#define DUMMY_REQ_HANDLER(type) ((type)(void *)1)
#define DUMMY_NOTIF_HANDLER(type) ((type)(void *)1)

static void
test_build_capabilities_null(void) {
	json_object *caps = lsp_build_server_capabilities(NULL, NULL);
	ASSERT_NE(caps, NULL);
	/* Should be empty object */
	ASSERT_EQ(json_object_object_length(caps), 0);
	json_object_put(caps);
}

static void
test_build_capabilities_hover(void) {
	struct LspServerRequestCallbacks req_cbs = {0};
	req_cbs.text_document__hover = DUMMY_REQ_HANDLER(LspTextDocumentHoverHandler);

	json_object *caps = lsp_build_server_capabilities(&req_cbs, NULL);
	ASSERT_NE(caps, NULL);

	json_object *hover = json_object_object_get(caps, "hoverProvider");
	ASSERT_NE(hover, NULL);
	ASSERT_EQ(json_object_get_boolean(hover), 1);

	json_object_put(caps);
}

static void
test_build_capabilities_multiple(void) {
	struct LspServerRequestCallbacks req_cbs = {0};
	req_cbs.text_document__hover = DUMMY_REQ_HANDLER(LspTextDocumentHoverHandler);
	req_cbs.text_document__completion = DUMMY_REQ_HANDLER(LspTextDocumentCompletionHandler);
	req_cbs.text_document__definition = DUMMY_REQ_HANDLER(LspTextDocumentDefinitionHandler);

	json_object *caps = lsp_build_server_capabilities(&req_cbs, NULL);
	ASSERT_NE(caps, NULL);

	ASSERT_NE(json_object_object_get(caps, "hoverProvider"), NULL);
	ASSERT_NE(json_object_object_get(caps, "completionProvider"), NULL);
	ASSERT_NE(json_object_object_get(caps, "definitionProvider"), NULL);

	json_object_put(caps);
}

static void
test_build_capabilities_text_sync(void) {
	struct LspServerNotificationCallbacks notif_cbs = {0};
	notif_cbs.text_document__did_open = DUMMY_NOTIF_HANDLER(LspTextDocumentDidOpenHandler);
	notif_cbs.text_document__did_close = DUMMY_NOTIF_HANDLER(LspTextDocumentDidCloseHandler);

	json_object *caps = lsp_build_server_capabilities(NULL, &notif_cbs);
	ASSERT_NE(caps, NULL);

	json_object *sync = json_object_object_get(caps, "textDocumentSync");
	ASSERT_NE(sync, NULL);

	json_object *open_close = json_object_object_get(sync, "openClose");
	ASSERT_NE(open_close, NULL);
	ASSERT_EQ(json_object_get_boolean(open_close), 1);

	json_object_put(caps);
}

static void
test_build_initialize_result_minimal(void) {
	json_object *result = lsp_build_initialize_result(NULL, NULL, NULL, NULL);
	ASSERT_NE(result, NULL);

	json_object *caps = json_object_object_get(result, "capabilities");
	ASSERT_NE(caps, NULL);
	ASSERT_EQ(json_object_object_length(caps), 0);

	/* No serverInfo when name/version not provided */
	ASSERT_EQ(json_object_object_get(result, "serverInfo"), NULL);

	json_object_put(result);
}

static void
test_build_initialize_result_with_server_info(void) {
	json_object *result =
			lsp_build_initialize_result("TestServer", "1.0.0", NULL, NULL);
	ASSERT_NE(result, NULL);

	json_object *server_info = json_object_object_get(result, "serverInfo");
	ASSERT_NE(server_info, NULL);

	ASSERT_STREQ(json_object_get_string(json_object_object_get(server_info, "name")),
			"TestServer");
	ASSERT_STREQ(json_object_get_string(json_object_object_get(server_info, "version")),
			"1.0.0");

	json_object_put(result);
}

static void
test_build_initialize_result_with_capabilities(void) {
	struct LspServerRequestCallbacks req_cbs = {0};
	req_cbs.text_document__hover = DUMMY_REQ_HANDLER(LspTextDocumentHoverHandler);

	struct LspServerNotificationCallbacks notif_cbs = {0};
	notif_cbs.text_document__did_open = DUMMY_NOTIF_HANDLER(LspTextDocumentDidOpenHandler);

	json_object *result =
			lsp_build_initialize_result("MyLSP", "2.0", &req_cbs, &notif_cbs);
	ASSERT_NE(result, NULL);

	/* Check capabilities */
	json_object *caps = json_object_object_get(result, "capabilities");
	ASSERT_NE(caps, NULL);
	ASSERT_NE(json_object_object_get(caps, "hoverProvider"), NULL);
	ASSERT_NE(json_object_object_get(caps, "textDocumentSync"), NULL);

	/* Check serverInfo */
	json_object *server_info = json_object_object_get(result, "serverInfo");
	ASSERT_NE(server_info, NULL);
	ASSERT_STREQ(json_object_get_string(json_object_object_get(server_info, "name")),
			"MyLSP");

	json_object_put(result);
}

DECLARE_TESTS
TEST(test_build_capabilities_null)
TEST(test_build_capabilities_hover)
TEST(test_build_capabilities_multiple)
TEST(test_build_capabilities_text_sync)
TEST(test_build_initialize_result_minimal)
TEST(test_build_initialize_result_with_server_info)
TEST(test_build_initialize_result_with_capabilities)
END_TESTS
