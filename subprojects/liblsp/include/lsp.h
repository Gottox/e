#ifndef LSPC_H
#define LSPC_H

#include <spawn.h>
#include <stdio.h>

struct LSPCConnection {
	FILE *sender;
	FILE *receiver;
	pid_t lsp_pid;
};

int
lsp_connection_init(struct LSPCConnection *client, const char *lsp_command[]);

int lsp_connection_send(
		struct LSPCConnection *client, const char *json_request,
		size_t request_length);

int lsp_connection_receive(
		struct LSPCConnection *client, char **json_response,
		size_t *response_length);

void lsp_connection_cleanup(struct LSPCConnection *client);

#endif // LSPC_H
