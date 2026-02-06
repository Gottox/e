/*
 * LSP Server Example
 *
 * Demonstrates implementing a minimal LSP server.
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

/* Handle initialize request */
static int
on_initialize(
		struct LspInitializeParams *params, struct LspInitializeResult *result,
		struct LspResponseError *error, void *userdata) {
	(void)error;
	struct ServerState *state = userdata;

	/* Log client info if available */
	struct LspNameStringVersionStringOptLiteral client_info = {0};
	if (lsp_initialize_params__client_info(params, &client_info) == 0) {
		const char *name = json_object_get_string(
				json_object_object_get(client_info.json, "name"));
		const char *version = json_object_get_string(
				json_object_object_get(client_info.json, "version"));
		fprintf(stderr, "[SERVER] Client: %s %s\n",
				name ? name : "unknown",
				version ? version : "");
	}

	/* Build server capabilities */
	result->json = json_object_new_object();

	json_object *capabilities = json_object_new_object();

	/* Text document sync: full sync */
	json_object_object_add(capabilities, "textDocumentSync",
			json_object_new_int(1));

	/* Hover support */
	json_object_object_add(capabilities, "hoverProvider",
			json_object_new_boolean(1));

	json_object_object_add(result->json, "capabilities", capabilities);

	/* Server info */
	json_object *server_info = json_object_new_object();
	json_object_object_add(server_info, "name",
			json_object_new_string("liblsp-example-server"));
	json_object_object_add(server_info, "version",
			json_object_new_string("0.1.0"));
	json_object_object_add(result->json, "serverInfo", server_info);

	state->initialized = true;
	fprintf(stderr, "[SERVER] Initialized\n");

	return 0;
}

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

	fprintf(stderr, "[SERVER] Hover at %s:%d:%d\n",
			uri ? uri : "(null)", line, character);

	/* Return a simple hover response */
	result->json = json_object_new_object();

	json_object *contents = json_object_new_object();
	json_object_object_add(contents, "kind", json_object_new_string("markdown"));
	json_object_object_add(contents, "value",
			json_object_new_string("**Hello from liblsp!**\n\nThis is a hover response."));
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
	fprintf(stderr, "[SERVER] Opened: %s (%s)\n",
			uri ? uri : "(null)",
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

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	struct Lsp lsp = {0};
	lsp_init(&lsp);
	lsp_stdio(&lsp);

	struct ServerState state = {0};

	/* Set up request callbacks */
	struct LspServerRequestCallbacks req_cbs = {0};
	req_cbs.initialize = on_initialize;
	req_cbs.shutdown = on_shutdown;
	req_cbs.text_document__hover = on_hover;

	/* Set up notification callbacks */
	struct LspServerNotificationCallbacks notif_cbs = {0};
	notif_cbs.initialized = on_initialized;
	notif_cbs.exit = on_exit_notif;
	notif_cbs.text_document__did_open = on_did_open;
	notif_cbs.text_document__did_close = on_did_close;

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
