#include <lsp_progress.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
test_progress_value_kind_begin(void) {
	/* Build a begin progress notification */
	json_object *params_json = json_object_new_object();
	json_object *token = json_object_new_string("test-token");
	json_object_object_add(params_json, "token", token);

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("begin"));
	json_object_object_add(value, "title", json_object_new_string("Test"));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};

	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_BEGIN);

	json_object_put(params_json);
}

static void
test_progress_value_kind_report(void) {
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_int(42));

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("report"));
	json_object_object_add(value, "percentage", json_object_new_int(50));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};

	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_REPORT);

	json_object_put(params_json);
}

static void
test_progress_value_kind_end(void) {
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_string("tok"));

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("end"));
	json_object_object_add(value, "message", json_object_new_string("Done"));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};

	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_END);

	json_object_put(params_json);
}

static void
test_progress_value_kind_unknown(void) {
	/* Missing value */
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_string("tok"));

	struct LspProgressParams params = {.json = params_json};
	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_UNKNOWN);

	/* Missing kind */
	json_object *value = json_object_new_object();
	json_object_object_add(params_json, "value", value);
	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_UNKNOWN);

	/* Unknown kind */
	json_object_object_add(value, "kind", json_object_new_string("other"));
	ASSERT_EQ(lsp_progress_value_kind(&params), LSP_PROGRESS_VALUE_UNKNOWN);

	/* NULL params */
	ASSERT_EQ(lsp_progress_value_kind(NULL), LSP_PROGRESS_VALUE_UNKNOWN);

	json_object_put(params_json);
}

static void
test_progress_value_as_begin(void) {
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_string("tok"));

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("begin"));
	json_object_object_add(value, "title", json_object_new_string("Indexing"));
	json_object_object_add(value, "message", json_object_new_string("Starting"));
	json_object_object_add(value, "percentage", json_object_new_int(0));
	json_object_object_add(value, "cancellable", json_object_new_boolean(1));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};
	struct LspWorkDoneProgressBegin begin = {0};

	int rv = lsp_progress_value_as_begin(&params, &begin);
	ASSERT_EQ(rv, LSP_OK);

	ASSERT_STREQ(lsp_work_done_progress_begin__title(&begin), "Indexing");
	ASSERT_STREQ(lsp_work_done_progress_begin__message(&begin), "Starting");
	ASSERT_EQ(lsp_work_done_progress_begin__percentage(&begin), 0);
	ASSERT_EQ(lsp_work_done_progress_begin__cancellable(&begin), true);

	lsp_work_done_progress_begin__cleanup(&begin);
	json_object_put(params_json);
}

static void
test_progress_value_as_report(void) {
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_int(123));

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("report"));
	json_object_object_add(value, "message", json_object_new_string("Progress"));
	json_object_object_add(value, "percentage", json_object_new_int(75));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};
	struct LspWorkDoneProgressReport report = {0};

	int rv = lsp_progress_value_as_report(&params, &report);
	ASSERT_EQ(rv, LSP_OK);

	ASSERT_STREQ(lsp_work_done_progress_report__message(&report), "Progress");
	ASSERT_EQ(lsp_work_done_progress_report__percentage(&report), 75);

	lsp_work_done_progress_report__cleanup(&report);
	json_object_put(params_json);
}

static void
test_progress_value_as_end(void) {
	json_object *params_json = json_object_new_object();
	json_object_object_add(params_json, "token", json_object_new_string("tok"));

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("end"));
	json_object_object_add(value, "message", json_object_new_string("Complete"));
	json_object_object_add(params_json, "value", value);

	struct LspProgressParams params = {.json = params_json};
	struct LspWorkDoneProgressEnd end = {0};

	int rv = lsp_progress_value_as_end(&params, &end);
	ASSERT_EQ(rv, LSP_OK);

	ASSERT_STREQ(lsp_work_done_progress_end__message(&end), "Complete");

	lsp_work_done_progress_end__cleanup(&end);
	json_object_put(params_json);
}

static void
test_progress_token_from_string(void) {
	struct LspProgressToken token = {0};

	int rv = lsp_progress_token_from_string(&token, "my-token");
	ASSERT_EQ(rv, LSP_OK);
	ASSERT_NE(token.json, NULL);
	ASSERT_EQ(lsp_progress_token__is_string(&token), true);

	const char *str = NULL;
	rv = lsp_progress_token__as_string(&token, &str);
	ASSERT_EQ(rv, LSP_OK);
	ASSERT_STREQ(str, "my-token");

	lsp_progress_token__cleanup(&token);
}

static void
test_progress_token_from_integer(void) {
	struct LspProgressToken token = {0};

	int rv = lsp_progress_token_from_integer(&token, 42);
	ASSERT_EQ(rv, LSP_OK);
	ASSERT_NE(token.json, NULL);
	ASSERT_EQ(lsp_progress_token__is_integer(&token), true);

	int64_t value = 0;
	rv = lsp_progress_token__as_integer(&token, &value);
	ASSERT_EQ(rv, LSP_OK);
	ASSERT_EQ(value, 42);

	lsp_progress_token__cleanup(&token);
}

DECLARE_TESTS
TEST(test_progress_value_kind_begin)
TEST(test_progress_value_kind_report)
TEST(test_progress_value_kind_end)
TEST(test_progress_value_kind_unknown)
TEST(test_progress_value_as_begin)
TEST(test_progress_value_as_report)
TEST(test_progress_value_as_end)
TEST(test_progress_token_from_string)
TEST(test_progress_token_from_integer)
END_TESTS
