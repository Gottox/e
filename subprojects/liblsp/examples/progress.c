/*
 * LSP Progress Example
 *
 * Demonstrates sending and receiving $/progress notifications.
 *
 * This example shows a server that:
 * - Sends progress notifications during a textDocument/hover request
 * - Reports progress at 0%, 50%, 100% before returning the result
 *
 * And a client that:
 * - Receives and displays progress notifications from the server
 *
 * Usage:
 *   Server mode: ./progress --server
 *   Client mode: ./progress ./progress --server
 */

#include <errno.h>
#include <lsp.h>
#include <lsp_progress.h>
#include <lsp_util.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * Server Implementation
 * ============================================================================ */

struct ServerState {
	struct Lsp *lsp;
	int next_token;
};

/* Handle textDocument/hover with progress reporting */
static int
on_hover(
		struct LspHoverParams *params, struct LspHoverOrNull *result,
		struct LspResponseError *error, void *userdata) {
	(void)params;
	(void)error;
	struct ServerState *state = userdata;

	/* Create a progress token */
	struct LspProgressToken token = {0};
	char token_str[32];
	snprintf(token_str, sizeof(token_str), "hover-%d", state->next_token++);
	int rv = lsp_progress_token_from_string(&token, token_str);
	if (rv < 0) {
		fprintf(stderr, "[SERVER] Failed to create token\n");
	}

	/* Send progress: begin */
	fprintf(stderr, "[SERVER] Sending progress begin\n");
	rv = lsp_progress_begin_send(state->lsp, &token,
			"Processing Hover", "Starting analysis...", 0, false);
	if (rv < 0) {
		fprintf(stderr, "[SERVER] Failed to send begin: %s\n", strerror(-rv));
	}

	/* Simulate some work */
	usleep(100000); /* 100ms */

	/* Send progress: report at 50% */
	fprintf(stderr, "[SERVER] Sending progress report (50%%)\n");
	rv = lsp_progress_report_send(state->lsp, &token,
			"Analyzing symbols...", 50, false);
	if (rv < 0) {
		fprintf(stderr, "[SERVER] Failed to send report: %s\n", strerror(-rv));
	}

	/* Simulate more work */
	usleep(100000); /* 100ms */

	/* Send progress: report at 100% */
	fprintf(stderr, "[SERVER] Sending progress report (100%%)\n");
	rv = lsp_progress_report_send(state->lsp, &token,
			"Finalizing...", 100, false);
	if (rv < 0) {
		fprintf(stderr, "[SERVER] Failed to send report: %s\n", strerror(-rv));
	}

	usleep(50000); /* 50ms */

	/* Send progress: end */
	fprintf(stderr, "[SERVER] Sending progress end\n");
	rv = lsp_progress_end_send(state->lsp, &token, "Analysis complete");
	if (rv < 0) {
		fprintf(stderr, "[SERVER] Failed to send end: %s\n", strerror(-rv));
	}

	lsp_progress_token__cleanup(&token);

	/* Return hover result */
	result->json = json_object_new_object();
	json_object *contents = json_object_new_object();
	json_object_object_add(contents, "kind", json_object_new_string("markdown"));
	json_object_object_add(contents, "value",
			json_object_new_string("**Hover Result**\n\nProcessed with progress!"));
	json_object_object_add(result->json, "contents", contents);

	return 0;
}

static int
on_shutdown(struct LspResponseError *error, void *userdata) {
	(void)error;
	(void)userdata;
	fprintf(stderr, "[SERVER] Shutdown requested\n");
	return 0;
}

static int
on_initialized(struct LspInitializedParams *params, void *userdata) {
	(void)params;
	(void)userdata;
	fprintf(stderr, "[SERVER] Client initialized\n");
	return 0;
}

static int
on_exit_notif(void *userdata) {
	(void)userdata;
	fprintf(stderr, "[SERVER] Exit received\n");
	return 0;
}

static int
run_server(void) {
	struct Lsp lsp = {0};
	if (lsp_init(&lsp, "progress-example-server", "0.1.0") < 0) {
		fprintf(stderr, "[SERVER] Failed to initialize\n");
		return 1;
	}
	lsp_stdio(&lsp);

	struct ServerState state = {.lsp = &lsp, .next_token = 1};

	struct LspServerRequestCallbacks req_cbs = {
			.shutdown = on_shutdown,
			.text_document__hover = on_hover,
	};

	struct LspServerNotificationCallbacks notif_cbs = {
			.initialized = on_initialized,
			.exit = on_exit_notif,
	};

	fprintf(stderr, "[SERVER] Starting progress example server\n");

	struct pollfd pfd = {.fd = lsp_fd(&lsp), .events = POLLIN};

	while (lsp.running) {
		int rv = poll(&pfd, 1, -1);
		if (rv < 0) {
			if (errno == EINTR)
				continue;
			break;
		}

		rv = lsp_server_process(&lsp, &req_cbs, &notif_cbs, &state);
		if (rv == -EAGAIN)
			continue;
		if (rv == -ECONNRESET) {
			fprintf(stderr, "[SERVER] Client disconnected\n");
			break;
		}
	}

	lsp_cleanup(&lsp);
	return 0;
}

/* ============================================================================
 * Client Implementation
 * ============================================================================ */

/* Handle $/progress notifications */
static int
on_progress(struct LspProgressParams *params, void *userdata) {
	(void)userdata;

	/* Get the token */
	struct LspProgressToken token = {0};
	lsp_progress_params__token(params, &token);

	const char *token_str = NULL;
	int64_t token_int = 0;
	if (lsp_progress_token__is_string(&token)) {
		lsp_progress_token__as_string(&token, &token_str);
	} else if (lsp_progress_token__is_integer(&token)) {
		lsp_progress_token__as_integer(&token, &token_int);
	}

	/* Determine the progress kind and handle accordingly */
	switch (lsp_progress_value_kind(params)) {
	case LSP_PROGRESS_VALUE_BEGIN: {
		struct LspWorkDoneProgressBegin begin = {0};
		if (lsp_progress_value_as_begin(params, &begin) == LSP_OK) {
			const char *title = lsp_work_done_progress_begin__title(&begin);
			const char *message = lsp_work_done_progress_begin__message(&begin);
			uint64_t pct = lsp_work_done_progress_begin__percentage(&begin);

			if (token_str) {
				fprintf(stderr, "[PROGRESS:%s] BEGIN: %s", token_str, title);
			} else {
				fprintf(stderr, "[PROGRESS:%ld] BEGIN: %s", (long)token_int, title);
			}
			if (message)
				fprintf(stderr, " - %s", message);
			if (pct <= 100)
				fprintf(stderr, " (%lu%%)", (unsigned long)pct);
			fprintf(stderr, "\n");

			lsp_work_done_progress_begin__cleanup(&begin);
		}
		break;
	}

	case LSP_PROGRESS_VALUE_REPORT: {
		struct LspWorkDoneProgressReport report = {0};
		if (lsp_progress_value_as_report(params, &report) == LSP_OK) {
			const char *message = lsp_work_done_progress_report__message(&report);
			uint64_t pct = lsp_work_done_progress_report__percentage(&report);

			if (token_str) {
				fprintf(stderr, "[PROGRESS:%s] REPORT:", token_str);
			} else {
				fprintf(stderr, "[PROGRESS:%ld] REPORT:", (long)token_int);
			}
			if (message)
				fprintf(stderr, " %s", message);
			if (pct <= 100)
				fprintf(stderr, " (%lu%%)", (unsigned long)pct);
			fprintf(stderr, "\n");

			lsp_work_done_progress_report__cleanup(&report);
		}
		break;
	}

	case LSP_PROGRESS_VALUE_END: {
		struct LspWorkDoneProgressEnd end = {0};
		if (lsp_progress_value_as_end(params, &end) == LSP_OK) {
			const char *message = lsp_work_done_progress_end__message(&end);

			if (token_str) {
				fprintf(stderr, "[PROGRESS:%s] END", token_str);
			} else {
				fprintf(stderr, "[PROGRESS:%ld] END", (long)token_int);
			}
			if (message)
				fprintf(stderr, ": %s", message);
			fprintf(stderr, "\n");

			lsp_work_done_progress_end__cleanup(&end);
		}
		break;
	}

	default:
		fprintf(stderr, "[PROGRESS] Unknown progress kind\n");
		break;
	}

	return 0;
}

static int
run_client(const char *server_cmd[]) {
	struct Lsp lsp = {0};
	int rv = lsp_init(&lsp, NULL, NULL);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to initialize\n");
		return 1;
	}

	fprintf(stderr, "[CLIENT] Spawning server: %s\n", server_cmd[0]);
	rv = lsp_spawn(&lsp, server_cmd);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to spawn: %s\n", strerror(-rv));
		return 1;
	}
	fprintf(stderr, "[CLIENT] Server started (pid %d)\n", lsp.pid);

	/* Set up notification callbacks - including progress handler */
	struct LspClientNotificationCallbacks notif_cbs = {
			.progress = on_progress,
	};

	struct pollfd pfd = {.fd = lsp_fd(&lsp), .events = POLLIN};

	/* Send initialize request */
	struct LspInitializeParams init_params = {0};
	rv = lsp_initialize_params__init(&init_params);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to init params\n");
	}

	struct LspIntegerOrNull pid = {.json = json_object_new_int(getpid())};
	rv = lsp_initialize_params__set_process_id(&init_params, &pid);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to set process id\n");
	}

	struct LspClientCapabilities caps = {.json = json_object_new_object()};
	rv = lsp_initialize_params__set_capabilities(&init_params, &caps);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to set capabilities\n");
	}

	json_object *id = json_object_new_uint64(lsp_next_id(&lsp));
	json_object *init_req = lsp_initialize__request(&init_params, id);
	lsp_initialize_params__cleanup(&init_params);

	fprintf(stderr, "[CLIENT] Sending initialize...\n");
	rv = lsp_send(&lsp, init_req);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to send initialize: %s\n", strerror(-rv));
	}
	json_object_put(init_req);
	json_object_put(id);

	/* Wait for initialize response */
	while (lsp.running) {
		rv = poll(&pfd, 1, 5000);
		if (rv <= 0)
			break;
		rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
		if (rv != -EAGAIN)
			break;
	}

	/* Send initialized notification */
	struct LspInitializedParams initialized_params = {0};
	rv = lsp_initialized_params__init(&initialized_params);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to init initialized params\n");
	}
	json_object *initialized = lsp_initialized__notification(&initialized_params);
	lsp_initialized_params__cleanup(&initialized_params);
	rv = lsp_send(&lsp, initialized);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to send initialized: %s\n", strerror(-rv));
	}
	json_object_put(initialized);

	/* Give server time to process initialized */
	usleep(50000);

	fprintf(stderr, "[CLIENT] Sending hover request (should trigger progress)...\n");

	/* Send a hover request - this will trigger progress notifications */
	struct LspHoverParams hover_params = {0};
	rv = lsp_hover_params__init(&hover_params);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to init hover params\n");
	}

	struct LspTextDocumentIdentifier doc = {0};
	doc.json = json_object_new_object();
	json_object_object_add(doc.json, "uri",
			json_object_new_string("file:///test.c"));
	rv = lsp_hover_params__set_text_document(&hover_params, &doc);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to set text document\n");
	}

	struct LspPosition pos = {0};
	pos.json = json_object_new_object();
	json_object_object_add(pos.json, "line", json_object_new_int(0));
	json_object_object_add(pos.json, "character", json_object_new_int(0));
	rv = lsp_hover_params__set_position(&hover_params, &pos);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to set position\n");
	}

	id = json_object_new_uint64(lsp_next_id(&lsp));
	json_object *hover_req = lsp_text_document__hover__request(&hover_params, id);
	lsp_hover_params__cleanup(&hover_params);

	rv = lsp_send(&lsp, hover_req);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to send hover: %s\n", strerror(-rv));
	}
	json_object_put(hover_req);
	json_object_put(id);

	/*
	 * Process messages for ~500ms to receive:
	 * - Progress begin notification
	 * - Progress report notifications
	 * - Progress end notification
	 * - Hover response
	 */
	fprintf(stderr, "[CLIENT] Waiting for progress and response...\n");
	for (int i = 0; i < 10 && lsp.running; i++) {
		rv = poll(&pfd, 1, 50);
		if (rv > 0) {
			rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
			if (rv < 0 && rv != -EAGAIN && rv != -ECONNRESET) {
				fprintf(stderr, "[CLIENT] Process error: %s\n", strerror(-rv));
			}
		}
	}
	fprintf(stderr, "[CLIENT] Processing complete\n");

	/* Send shutdown */
	fprintf(stderr, "[CLIENT] Sending shutdown...\n");
	id = json_object_new_uint64(lsp_next_id(&lsp));
	json_object *shutdown_req = lsp_shutdown__request(id);
	rv = lsp_send(&lsp, shutdown_req);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to send shutdown: %s\n", strerror(-rv));
	}
	json_object_put(shutdown_req);
	json_object_put(id);

	/* Wait for response */
	for (int i = 0; i < 20 && lsp.running; i++) {
		rv = poll(&pfd, 1, 100);
		if (rv > 0) {
			rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
			if (rv == 0 || rv == -ECONNRESET)
				break;
		}
	}

	/* Send exit */
	json_object *exit_notif = lsp_exit__notification();
	rv = lsp_send(&lsp, exit_notif);
	if (rv < 0) {
		fprintf(stderr, "[CLIENT] Failed to send exit: %s\n", strerror(-rv));
	}
	json_object_put(exit_notif);

	fprintf(stderr, "[CLIENT] Done\n");
	lsp_cleanup(&lsp);
	return 0;
}

/* ============================================================================
 * Main
 * ============================================================================ */

int
main(int argc, char *argv[]) {
	if (argc >= 2 && strcmp(argv[1], "--server") == 0) {
		return run_server();
	}

	if (argc < 2) {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "  Server mode: %s --server\n", argv[0]);
		fprintf(stderr, "  Client mode: %s <server-command> [args...]\n", argv[0]);
		fprintf(stderr, "\nExample:\n");
		fprintf(stderr, "  %s %s --server\n", argv[0], argv[0]);
		return 1;
	}

	return run_client((const char **)&argv[1]);
}
