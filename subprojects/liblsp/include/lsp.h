#ifndef LSP_H
#define LSP_H

#include <cextras/collection.h>
#include <lsp_recv_buffer.h>
#include <messages.h>
#include <randid.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef LSP_NO_UNUSED
#define LSP_NO_UNUSED __attribute__((warn_unused_result))
#endif

// Legacy connection type (kept for backward compatibility)
struct LSPConnection {
	FILE *sender;
	FILE *receiver;
	pid_t lsp_pid;
};

int lsp_connection_client_init(
		struct LSPConnection *connection, const char *lsp_command[]);

int lsp_connection_server_init(
		struct LSPConnection *connection, FILE *in, FILE *out);

int lsp_connection_send(
		struct LSPConnection *connection, const char *json_request,
		size_t request_length);

int lsp_connection_receive(
		struct LSPConnection *connection, char **json_response,
		size_t *response_length);

void lsp_connection_cleanup(struct LSPConnection *connection);

// Generic response callback wrapper type
typedef void (*LspResponseWrapper)(
		json_object *response, void *callback, void *userdata);

// Pending request info stored in hashmap
struct LspPendingRequest {
	LspResponseWrapper wrapper;
	void *callback;
	void *userdata;
};

// New high-level API
struct Lsp {
	FILE *sender;
	int receiver_fd;
	pid_t pid;
	bool running;
	struct RidGen id_gen;
	struct LspRecvBuffer recv_buf;
	struct CxHashMap pending_requests;
	const char *name;
	const char *version;
};

// Initialize state
LSP_NO_UNUSED int lsp_init(
		struct Lsp *lsp, const char *name, const char *version);

// Setup stdin/stdout for server mode
void lsp_stdio(struct Lsp *lsp);

// Spawn subprocess for client mode
LSP_NO_UNUSED int lsp_spawn(struct Lsp *lsp, const char *argv[]);

// Send a JSON message
LSP_NO_UNUSED int lsp_send(struct Lsp *lsp, json_object *msg);

// Send a notification and free the json
LSP_NO_UNUSED int lsp_notify(struct Lsp *lsp, json_object *notif);

// Send a request, store pending callback, and free the json
// wrapper_fn is called with (response, callback, userdata) when reply arrives
LSP_NO_UNUSED int lsp_request(
		struct Lsp *lsp, json_object *request, LspResponseWrapper wrapper_fn,
		void *callback, void *userdata);

// Generate next request ID
uint64_t lsp_next_id(struct Lsp *lsp);

// Get receiver fd for poll()
int lsp_fd(const struct Lsp *lsp);

// Cleanup resources
void lsp_cleanup(struct Lsp *lsp);

// Process one message as server (receives C2S messages)
LSP_NO_UNUSED int lsp_server_process(
		struct Lsp *lsp,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs, void *userdata);

// Process one message as client (receives S2C messages)
LSP_NO_UNUSED int lsp_client_process(
		struct Lsp *lsp,
		const struct LspClientRequestCallbacks *req_cbs,
		const struct LspClientNotificationCallbacks *notif_cbs, void *userdata);

#endif // LSP_H
