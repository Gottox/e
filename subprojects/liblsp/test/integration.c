/*
 * Integration test: Spawn clangd, initialize session, rename a variable,
 * and shut down gracefully.
 */

#include <errno.h>
#include <fcntl.h>
#include <lsp.h>
#include <lsp_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <testlib.h>
#include <unistd.h>

/* Test state */
static int initialize_done = 0;
static int rename_done = 0;
static int shutdown_done = 0;
static int test_failed = 0;
static char *test_file_path = NULL;
static char test_file_uri[512] = {0};

/* Create a temporary C file for testing */
static int
create_test_file(void) {
	char template[] = "/tmp/lsp_integration_XXXXXX";
	int fd = mkstemp(template);
	if (fd < 0) {
		return -1;
	}

	const char *content = "int main(void) {\n"
						  "    int old_name = 42;\n"
						  "    return old_name;\n"
						  "}\n";

	write(fd, content, strlen(content));
	close(fd);

	/* Rename to .c extension */
	size_t len = strlen(template);
	test_file_path = malloc(len + 3);
	snprintf(test_file_path, len + 3, "%s.c", template);
	rename(template, test_file_path);

	/* Build file URI */
	snprintf(test_file_uri, sizeof(test_file_uri), "file://%s", test_file_path);

	return 0;
}

static void
cleanup_test_file(void) {
	if (test_file_path) {
		unlink(test_file_path);
		free(test_file_path);
		test_file_path = NULL;
	}
}

/* Callback: Initialize response */
static void
on_initialize(
		struct LspInitializeResult *result, struct LspResponseError *error,
		void *userdata) {
	(void)userdata;

	if (error && error->json) {
		fprintf(stderr, "Initialize error: %s\n",
				lsp_response_error__message(error));
		test_failed = 1;
		return;
	}

	if (result) {
		/* Check server capabilities */
		struct LspServerCapabilities caps = {0};
		if (lsp_initialize_result__capabilities(result, &caps) == 0) {
			/* Server initialized successfully */
			initialize_done = 1;
		}
		lsp_server_capabilities__cleanup(&caps);
	}
}

/* Callback: Rename response */
static void
on_rename(
		struct LspWorkspaceEditOrNull *result, struct LspResponseError *error,
		void *userdata) {
	(void)userdata;

	if (error && error->json) {
		fprintf(stderr, "Rename error: %s\n",
				lsp_response_error__message(error));
		test_failed = 1;
		return;
	}

	/* Rename completed (result may be null if no changes needed) */
	rename_done = 1;
	(void)result;
}

/* Callback: Shutdown response */
static void
on_shutdown(struct LspResponseError *error, void *userdata) {
	(void)userdata;

	if (error && error->json) {
		fprintf(stderr, "Shutdown error: %s\n",
				lsp_response_error__message(error));
		test_failed = 1;
		return;
	}

	shutdown_done = 1;
}

static int
wait_for_flag(struct Lsp *lsp, int *flag) {
	int rv = 0;
	while (!*flag && !test_failed) {
		rv = lsp_client_process(lsp, NULL, NULL, NULL);
		if (rv < 0 && rv != -EAGAIN) {
			return rv;
		}
	}

	return 0;
}

static void
test_clangd_integration(void) {
	struct Lsp lsp = {0};
	int rv;

	/* Create test file */
	rv = create_test_file();
	ASSERT_EQ(rv, 0);

	/* Initialize LSP */
	rv = lsp_init(&lsp, NULL, NULL);
	ASSERT_EQ(rv, 0);

	/* Spawn clangd */
	const char *clangd_path = getenv("CLANGD");
	if (!clangd_path) {
		FAIL("%s", "CLANGD environment variable not set");
	}
	const char *argv[] = {clangd_path, "--log=error", NULL};
	rv = lsp_spawn(&lsp, argv);
	if (rv != 0) {
		cleanup_test_file();
		lsp_cleanup(&lsp);
		FAIL("Failed to spawn clangd: %s", strerror(-rv));
	}

	/* === Step 1: Send initialize request === */
	struct LspInitializeParams init_params = {0};
	rv = lsp_initialize_params__init(&init_params);
	ASSERT_EQ(rv, 0);

	struct LspIntegerOrNull pid = {0};
	pid.json = json_object_new_int(getpid());
	rv = lsp_initialize_params__set_process_id(&init_params, &pid);
	json_object_put(pid.json);
	ASSERT_EQ(rv, 0);

	struct LspClientCapabilities caps = {0};
	caps.json = json_object_new_object();
	rv = lsp_initialize_params__set_capabilities(&init_params, &caps);
	json_object_put(caps.json);
	ASSERT_EQ(rv, 0);

	rv = lsp_initialize__send(&init_params, &lsp, on_initialize, NULL);
	ASSERT_EQ(rv, 0);

	lsp_initialize_params__cleanup(&init_params);

	/* Wait for initialize response */
	rv = wait_for_flag(&lsp, &initialize_done);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_failed, 0);

	/* === Step 2: Send initialized notification === */
	struct LspInitializedParams initialized_params = {0};
	rv = lsp_initialized_params__init(&initialized_params);
	ASSERT_EQ(rv, 0);
	rv = lsp_initialized__send(&initialized_params, &lsp);
	ASSERT_EQ(rv, 0);
	lsp_initialized_params__cleanup(&initialized_params);

	/* === Step 3: Open the test file === */
	struct LspDidOpenTextDocumentParams open_params = {0};
	rv = lsp_did_open_text_document_params__init(&open_params);
	ASSERT_EQ(rv, 0);

	struct LspTextDocumentItem text_doc = {0};
	rv = lsp_text_document_item__init(&text_doc);
	ASSERT_EQ(rv, 0);
	rv = lsp_text_document_item__set_uri(&text_doc, test_file_uri);
	ASSERT_EQ(rv, 0);
	rv = lsp_text_document_item__set_language_id(&text_doc, "c");
	ASSERT_EQ(rv, 0);
	rv = lsp_text_document_item__set_version(&text_doc, 1);
	ASSERT_EQ(rv, 0);

	/* Read file content */
	FILE *f = fopen(test_file_path, "r");
	ASSERT_NOT_NULL(f);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *content = malloc(size + 1);
	fread(content, 1, size, f);
	content[size] = '\0';
	fclose(f);

	rv = lsp_text_document_item__set_text(&text_doc, content);
	ASSERT_EQ(rv, 0);
	free(content);

	rv = lsp_did_open_text_document_params__set_text_document(&open_params, &text_doc);
	ASSERT_EQ(rv, 0);
	lsp_text_document_item__cleanup(&text_doc);

	rv = lsp_text_document__did_open__send(&open_params, &lsp);
	ASSERT_EQ(rv, 0);

	lsp_did_open_text_document_params__cleanup(&open_params);

	/* === Step 4: Send rename request (rename old_name to new_name) === */
	struct LspRenameParams rename_params = {0};
	rv = lsp_rename_params__init(&rename_params);
	ASSERT_EQ(rv, 0);

	struct LspTextDocumentIdentifier doc_id = {0};
	rv = lsp_text_document_identifier__init(&doc_id);
	ASSERT_EQ(rv, 0);
	rv = lsp_text_document_identifier__set_uri(&doc_id, test_file_uri);
	ASSERT_EQ(rv, 0);
	rv = lsp_rename_params__set_text_document(&rename_params, &doc_id);
	ASSERT_EQ(rv, 0);
	lsp_text_document_identifier__cleanup(&doc_id);

	/* Position of "old_name" on line 2, column 8 (0-indexed: line 1, char 8) */
	struct LspPosition pos = {0};
	rv = lsp_position__init(&pos);
	ASSERT_EQ(rv, 0);
	rv = lsp_position__set_line(&pos, 1);
	ASSERT_EQ(rv, 0);
	rv = lsp_position__set_character(&pos, 8);
	ASSERT_EQ(rv, 0);
	rv = lsp_rename_params__set_position(&rename_params, &pos);
	ASSERT_EQ(rv, 0);
	lsp_position__cleanup(&pos);

	rv = lsp_rename_params__set_new_name(&rename_params, "new_name");
	ASSERT_EQ(rv, 0);

	rv = lsp_text_document__rename__send(&rename_params, &lsp, on_rename, NULL);
	ASSERT_EQ(rv, 0);

	lsp_rename_params__cleanup(&rename_params);

	/* Wait for rename response */
	rv = wait_for_flag(&lsp, &rename_done);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_failed, 0);

	/* === Step 5: Send shutdown request === */
	rv = lsp_shutdown__send(&lsp, on_shutdown, NULL);
	ASSERT_EQ(rv, 0);

	/* Wait for shutdown response */
	rv = wait_for_flag(&lsp, &shutdown_done);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_failed, 0);

	/* === Step 6: Send exit notification === */
	rv = lsp_exit__send(&lsp);
	ASSERT_EQ(rv, 0);

	/* Cleanup */
	cleanup_test_file();
	lsp_cleanup(&lsp);
}

DECLARE_TESTS
TEST(test_clangd_integration)
END_TESTS
