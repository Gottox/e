/*
 * LSP Server Example
 *
 * Demonstrates implementing a minimal LSP server with automatic capability
 * detection based on registered handlers.
 *
 * The server responds to initialize/shutdown requests and exits on the exit
 * notification. It logs all received requests and notifications to stderr.
 *
 * Usage: echo '<lsp-message>' | ./server
 * Or use with an editor that supports LSP.
 */

#include <errno.h>
#include <lsp.h>
#include <lsp_util.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>

/* Server state */
struct ServerState {
	bool initialized;
	bool shutdown_requested;
};

/* Handle shutdown request */
static int
on_shutdown(struct LspResponseError *error, void *userdata) {
	(void)error;
	struct ServerState *state = userdata;
	state->shutdown_requested = true;
	fprintf(stderr, "[SERVER] Shutdown requested\n");
	return 0;
}

/* Handle textDocument/hover request */
static int
on_hover(
		struct LspHoverParams *params, struct LspHoverOrNull *result,
		struct LspResponseError *error, void *userdata) {
	(void)error;
	(void)userdata;

	/* Get the document URI and position */
	struct LspTextDocumentIdentifier doc = {0};
	lsp_hover_params__text_document(params, &doc);
	const char *uri = lsp_text_document_identifier__uri(&doc);

	struct LspPosition pos = {0};
	lsp_hover_params__position(params, &pos);
	int line = lsp_position__line(&pos);
	int character = lsp_position__character(&pos);

	fprintf(stderr, "[SERVER] Hover at %s:%d:%d\n", uri, line, character);

	/* Return a simple hover response */
	result->json = json_object_new_object();

	json_object *contents = json_object_new_object();
	json_object_object_add(
			contents, "kind", json_object_new_string("markdown"));
	json_object_object_add(
			contents, "value",
			json_object_new_string(
					"**Hello from liblsp!**\n\nThis is a hover response."));
	json_object_object_add(result->json, "contents", contents);

	return 0;
}

/* Handle initialized notification */
static int
on_initialized(struct LspInitializedParams *params, void *userdata) {
	(void)params;
	(void)userdata;
	fprintf(stderr, "[SERVER] Client sent initialized notification\n");
	return 0;
}

/* Handle exit notification */
static int
on_exit_notif(void *userdata) {
	(void)userdata;
	fprintf(stderr, "[SERVER] Exit notification received\n");
	return 0;
}

/* Handle textDocument/didOpen notification */
static int
on_did_open(struct LspDidOpenTextDocumentParams *params, void *userdata) {
	(void)userdata;
	struct LspTextDocumentItem doc = {0};
	lsp_did_open_text_document_params__text_document(params, &doc);
	const char *uri = lsp_text_document_item__uri(&doc);
	const char *lang = lsp_text_document_item__language_id(&doc);
	fprintf(stderr, "[SERVER] Opened: %s (%s)\n", uri ? uri : "(null)",
			lang ? lang : "unknown");
	return 0;
}

/* Handle textDocument/didClose notification */
static int
on_did_close(struct LspDidCloseTextDocumentParams *params, void *userdata) {
	(void)userdata;
	struct LspTextDocumentIdentifier doc = {0};
	lsp_did_close_text_document_params__text_document(params, &doc);
	const char *uri = lsp_text_document_identifier__uri(&doc);
	fprintf(stderr, "[SERVER] Closed: %s\n", uri ? uri : "(null)");
	return 0;
}

static const struct LspServerRequestCallbacks req_cbs = {
		.shutdown = on_shutdown,
		.text_document__hover = on_hover,
};

static const struct LspServerNotificationCallbacks notif_cbs = {
		.initialized = on_initialized,
		.exit = on_exit_notif,
		.text_document__did_open = on_did_open,
		.text_document__did_close = on_did_close,
};

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	struct Lsp lsp = {0};
	if (lsp_init(&lsp, "liblsp-example-server", "0.1.0") != 0) {
		fprintf(stderr, "[SERVER] Failed to initialize LSP\n");
		return 1;
	}
	lsp_stdio(&lsp);

	struct ServerState state = {0};

	fprintf(stderr, "[SERVER] Starting LSP server on stdio...\n");

	struct pollfd pfd = {.fd = lsp_fd(&lsp), .events = POLLIN};

	while (lsp.running) {
		int rv = poll(&pfd, 1, -1);
		if (rv < 0) {
			if (errno == EINTR) {
				continue;
			}
			fprintf(stderr, "[SERVER] poll error: %s\n", strerror(errno));
			break;
		}

		rv = lsp_server_process(&lsp, &req_cbs, &notif_cbs, &state);
		if (rv == -EAGAIN) {
			continue;
		}
		if (rv == -ECONNRESET) {
			fprintf(stderr, "[SERVER] Client disconnected\n");
			break;
		}
		if (rv < 0) {
			fprintf(stderr, "[SERVER] Error processing message: %s\n",
					strerror(-rv));
		}
	}

	fprintf(stderr, "[SERVER] Exiting\n");
	lsp_cleanup(&lsp);

	return 0;
}
