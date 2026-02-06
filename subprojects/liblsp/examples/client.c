/*
 * LSP Client Example
 *
 * Demonstrates spawning an LSP server and communicating with it.
 *
 * Usage: ./client <server-command> [args...]
 * Example: ./client clangd
 */

#include <errno.h>
#include <limits.h>
#include <lsp.h>
#include <lsp_util.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Notification handler for window/logMessage */
static int
on_log_message(struct LspLogMessageParams *params, void *userdata) {
	(void)userdata;
	const char *msg = lsp_log_message_params__message(params);
	int type = lsp_log_message_params__type(params);
	const char *level = "???";
	switch (type) {
	case 1: level = "ERROR"; break;
	case 2: level = "WARN"; break;
	case 3: level = "INFO"; break;
	case 4: level = "LOG"; break;
	}
	fprintf(stderr, "[%s] %s\n", level, msg ? msg : "(null)");
	return 0;
}

/* Notification handler for window/showMessage */
static int
on_show_message(struct LspShowMessageParams *params, void *userdata) {
	(void)userdata;
	const char *msg = lsp_show_message_params__message(params);
	fprintf(stderr, "[SERVER] %s\n", msg ? msg : "(null)");
	return 0;
}

/* Notification handler for textDocument/publishDiagnostics */
static int
on_publish_diagnostics(struct LspPublishDiagnosticsParams *params, void *userdata) {
	(void)userdata;
	const char *uri = lsp_publish_diagnostics_params__uri(params);
	struct LspDiagnosticArray diags = {0};
	lsp_publish_diagnostics_params__diagnostics(params, &diags);
	size_t count = lsp_diagnostic_array__len(&diags);
	fprintf(stderr, "[DIAG] %s: %zu diagnostics\n", uri ? uri : "(null)", count);
	return 0;
}

/* Build initialize request parameters */
static int
build_init_params(struct LspInitializeParams *params) {
	int rv = lsp_initialize_params__init(params);
	if (rv < 0) {
		return rv;
	}

	/* Set process ID */
	struct LspIntegerOrNull pid = {0};
	pid.json = json_object_new_int(getpid());
	rv = lsp_initialize_params__set_process_id(params, &pid);
	if (rv < 0) {
		return rv;
	}

	/* Set client info */
	struct LspNameStringVersionStringOptLiteral client_info = {0};
	client_info.json = json_object_new_object();
	json_object_object_add(client_info.json, "name",
			json_object_new_string("liblsp-example"));
	json_object_object_add(client_info.json, "version",
			json_object_new_string("0.1.0"));
	rv = lsp_initialize_params__set_client_info(params, &client_info);
	if (rv < 0) {
		return rv;
	}

	/* Set root URI (current directory) */
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd))) {
		char uri[PATH_MAX + 8];
		snprintf(uri, sizeof(uri), "file://%s", cwd);
		struct LspDocumentUriOrNull root = {0};
		root.json = json_object_new_string(uri);
		rv = lsp_initialize_params__set_root_uri(params, &root);
		if (rv < 0) {
			return rv;
		}
	}

	/* Set capabilities (minimal) */
	struct LspClientCapabilities caps = {0};
	caps.json = json_object_new_object();
	rv = lsp_initialize_params__set_capabilities(params, &caps);
	if (rv < 0) {
		return rv;
	}

	return 0;
}

int
main(int argc, char *argv[]) {
	int rv;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <server-command> [args...]\n", argv[0]);
		fprintf(stderr, "Example: %s clangd\n", argv[0]);
		return 1;
	}

	struct Lsp lsp = {0};
	rv = lsp_init(&lsp, NULL, NULL);
	if (rv < 0) {
		fprintf(stderr, "Failed to init lsp: %s\n", strerror(-rv));
		return 1;
	}

	/* Spawn the LSP server */
	fprintf(stderr, "Spawning %s...\n", argv[1]);
	rv = lsp_spawn(&lsp, (const char **)&argv[1]);
	if (rv < 0) {
		fprintf(stderr, "Failed to spawn server: %s\n", strerror(-rv));
		return 1;
	}
	fprintf(stderr, "Server started (pid %d)\n", lsp.pid);

	/* Set up notification callbacks */
	struct LspClientNotificationCallbacks notif_cbs = {0};
	notif_cbs.window__log_message = on_log_message;
	notif_cbs.window__show_message = on_show_message;
	notif_cbs.text_document__publish_diagnostics = on_publish_diagnostics;

	/* Send initialize request */
	struct LspInitializeParams init_params = {0};
	if (build_init_params(&init_params) != 0) {
		fprintf(stderr, "Failed to build init params\n");
		lsp_cleanup(&lsp);
		return 1;
	}

	json_object *id = json_object_new_int64(lsp_next_id(&lsp));
	json_object *init_req = lsp_initialize__request(&init_params, id);
	lsp_initialize_params__cleanup(&init_params);

	fprintf(stderr, "Sending initialize request...\n");
	rv = lsp_send(&lsp, init_req);
	json_object_put(init_req);
	json_object_put(id);

	if (rv < 0) {
		fprintf(stderr, "Failed to send initialize: %s\n", strerror(-rv));
		lsp_cleanup(&lsp);
		return 1;
	}

	/* Wait for initialize response */
	fprintf(stderr, "Waiting for response...\n");
	struct pollfd pfd = {.fd = lsp_fd(&lsp), .events = POLLIN};

	while (lsp.running) {
		rv = poll(&pfd, 1, 5000);
		if (rv < 0) {
			fprintf(stderr, "poll error: %s\n", strerror(errno));
			break;
		}
		if (rv == 0) {
			fprintf(stderr, "Timeout waiting for response\n");
			break;
		}

		rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
		if (rv == -EAGAIN) {
			continue;
		}
		if (rv < 0 && rv != -ECONNRESET) {
			fprintf(stderr, "Process error: %s\n", strerror(-rv));
		}
		/* Got response, break out */
		break;
	}

	fprintf(stderr, "Received response, sending initialized notification...\n");

	/* Send initialized notification */
	struct LspInitializedParams initialized_params = {0};
	rv = lsp_initialized_params__init(&initialized_params);
	if (rv < 0) {
		fprintf(stderr, "Failed to init initialized params\n");
	}
	json_object *initialized = lsp_initialized__notification(&initialized_params);
	lsp_initialized_params__cleanup(&initialized_params);
	rv = lsp_send(&lsp, initialized);
	if (rv < 0) {
		fprintf(stderr, "Failed to send initialized: %s\n", strerror(-rv));
	}
	json_object_put(initialized);

	/* Process a few more messages to see server activity */
	fprintf(stderr, "Processing messages for 2 seconds...\n");
	for (int i = 0; i < 20 && lsp.running; i++) {
		rv = poll(&pfd, 1, 100);
		if (rv > 0) {
			rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
			if (rv < 0 && rv != -EAGAIN && rv != -ECONNRESET) {
				fprintf(stderr, "Process error: %s\n", strerror(-rv));
			}
		}
	}

	/* Send shutdown request */
	fprintf(stderr, "Sending shutdown request...\n");
	id = json_object_new_int64(lsp_next_id(&lsp));
	json_object *shutdown_req = lsp_shutdown__request(id);
	rv = lsp_send(&lsp, shutdown_req);
	if (rv < 0) {
		fprintf(stderr, "Failed to send shutdown: %s\n", strerror(-rv));
	}
	json_object_put(shutdown_req);
	json_object_put(id);

	/* Wait for shutdown response */
	for (int i = 0; i < 50 && lsp.running; i++) {
		rv = poll(&pfd, 1, 100);
		if (rv > 0) {
			rv = lsp_client_process(&lsp, NULL, &notif_cbs, NULL);
			if (rv == 0 || rv == -ECONNRESET) {
				break;
			}
		}
	}

	/* Send exit notification */
	fprintf(stderr, "Sending exit notification...\n");
	json_object *exit_notif = lsp_exit__notification();
	rv = lsp_send(&lsp, exit_notif);
	if (rv < 0) {
		fprintf(stderr, "Failed to send exit: %s\n", strerror(-rv));
	}
	json_object_put(exit_notif);

	fprintf(stderr, "Cleaning up...\n");
	lsp_cleanup(&lsp);
	fprintf(stderr, "Done.\n");

	return 0;
}
