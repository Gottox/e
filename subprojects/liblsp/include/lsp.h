#ifndef LSP_H
#define LSP_H

#include <spawn.h>
#include <stdio.h>

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

#endif // LSP_H
