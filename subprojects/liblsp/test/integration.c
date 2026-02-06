/*
 * Integration test: Spawn clangd, initialize session, rename a variable,
 * and shut down gracefully.
 */

#include <errno.h>
#include <fcntl.h>
#include <lsp.h>
#include <lsp_util.h>
#include <poll.h>
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

	const char *content =
			"int main(void) {\n"
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
		fprintf(stderr, "Rename error: %s\n", lsp_response_error__message(error));
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

/* Helper: Wait for response with timeout */
static int
process_with_timeout(struct Lsp *lsp, int timeout_ms) {
	struct pollfd pfd = {
			.fd = lsp_fd(lsp),
			.events = POLLIN,
	};

	int rv = poll(&pfd, 1, timeout_ms);
	if (rv <= 0) {
		return rv == 0 ? -ETIMEDOUT : -errno;
	}

	return lsp_client_process(lsp, NULL, NULL, NULL);
}

/* Helper: Process until a flag is set or timeout */
static int
wait_for_flag(struct Lsp *lsp, int *flag, int timeout_ms) {
	int elapsed = 0;
	const int step = 100;

	while (!*flag && elapsed < timeout_ms && !test_failed) {
		int rv = process_with_timeout(lsp, step);
		if (rv < 0 && rv != -ETIMEDOUT && rv != -EAGAIN) {
			return rv;
		}
		elapsed += step;
	}

	return *flag ? 0 : -ETIMEDOUT;
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
	const char *argv[] = {"clangd", "--log=error", NULL};
	rv = lsp_spawn(&lsp, argv);
	if (rv != 0) {
		fprintf(stderr, "Failed to spawn clangd (is it installed?)\n");
		cleanup_test_file();
		lsp_cleanup(&lsp);
		return; /* Skip test if clangd not available */
	}

	/* === Step 1: Send initialize request === */
	{
		struct LspInitializeParams params = {0};
		rv = lsp_initialize_params__init(&params);
		ASSERT_EQ(rv, 0);

		struct LspIntegerOrNull pid = {0};
		pid.json = json_object_new_int(getpid());
		lsp_initialize_params__set_process_id(&params, &pid);

		struct LspClientCapabilities caps = {0};
		caps.json = json_object_new_object();
		lsp_initialize_params__set_capabilities(&params, &caps);

		rv = lsp_initialize__send(&params, &lsp, on_initialize, NULL);
		ASSERT_EQ(rv, 0);

		lsp_initialize_params__cleanup(&params);
	}

	/* Wait for initialize response */
	rv = wait_for_flag(&lsp, &initialize_done, 10000);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_failed, 0);

	/* === Step 2: Send initialized notification === */
	{
		struct LspInitializedParams params = {0};
		lsp_initialized_params__init(&params);
		rv = lsp_initialized__send(&params, &lsp);
		ASSERT_EQ(rv, 0);
		lsp_initialized_params__cleanup(&params);
	}

	/* === Step 3: Open the test file === */
	{
		struct LspDidOpenTextDocumentParams params = {0};
		rv = lsp_did_open_text_document_params__init(&params);
		ASSERT_EQ(rv, 0);

		struct LspTextDocumentItem doc = {0};
		lsp_text_document_item__init(&doc);
		lsp_text_document_item__set_uri(&doc, test_file_uri);
		lsp_text_document_item__set_language_id(&doc, "c");
		lsp_text_document_item__set_version(&doc, 1);

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

		lsp_text_document_item__set_text(&doc, content);
		free(content);

		lsp_did_open_text_document_params__set_text_document(&params, &doc);
		lsp_text_document_item__cleanup(&doc);

		rv = lsp_text_document__did_open__send(&params, &lsp);
		ASSERT_EQ(rv, 0);

		lsp_did_open_text_document_params__cleanup(&params);
	}

	/* Give clangd a moment to index */
	usleep(500000);

	/* === Step 4: Send rename request (rename old_name to new_name) === */
	{
		struct LspRenameParams params = {0};
		rv = lsp_rename_params__init(&params);
		ASSERT_EQ(rv, 0);

		struct LspTextDocumentIdentifier doc = {0};
		lsp_text_document_identifier__init(&doc);
		lsp_text_document_identifier__set_uri(&doc, test_file_uri);
		lsp_rename_params__set_text_document(&params, &doc);
		lsp_text_document_identifier__cleanup(&doc);

		/* Position of "old_name" on line 2, column 8 (0-indexed: line 1, char 8) */
		struct LspPosition pos = {0};
		lsp_position__init(&pos);
		lsp_position__set_line(&pos, 1);
		lsp_position__set_character(&pos, 8);
		lsp_rename_params__set_position(&params, &pos);
		lsp_position__cleanup(&pos);

		lsp_rename_params__set_new_name(&params, "new_name");

		rv = lsp_text_document__rename__send(&params, &lsp, on_rename, NULL);
		ASSERT_EQ(rv, 0);

		lsp_rename_params__cleanup(&params);
	}

	/* Wait for rename response */
	rv = wait_for_flag(&lsp, &rename_done, 10000);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(test_failed, 0);

	/* === Step 5: Send shutdown request === */
	rv = lsp_shutdown__send(&lsp, on_shutdown, NULL);
	ASSERT_EQ(rv, 0);

	/* Wait for shutdown response */
	rv = wait_for_flag(&lsp, &shutdown_done, 5000);
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
