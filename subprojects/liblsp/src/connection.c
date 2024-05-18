#include <errno.h>
#include <lsp_private.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int
lsp_connection_init(struct LSPCConnection *client, const char *lsp_command[]) {
	int sender_pipe[2];
	int receiver_pipe[2];

	if (pipe(sender_pipe) != 0 || pipe(receiver_pipe) != 0) {
		return -errno;
	}

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	// Redirect stdin and stdout to the pipes
	posix_spawn_file_actions_adddup2(&actions, sender_pipe[0], STDIN_FILENO);
	posix_spawn_file_actions_adddup2(&actions, receiver_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&actions, sender_pipe[1]);
	posix_spawn_file_actions_addclose(&actions, receiver_pipe[0]);

	if (posix_spawn(
				&client->lsp_pid, lsp_command[0], &actions, NULL,
				(char *const *)lsp_command, environ) != 0) {
		return -errno;
	}

	close(sender_pipe[0]);
	close(receiver_pipe[1]);

	client->sender = fdopen(sender_pipe[1], "w");
	client->receiver = fdopen(receiver_pipe[0], "r");

	if (!client->sender || !client->receiver) {
		return -errno;
	}

	return 0;
}

int
lsp_connection_send(
		struct LSPCConnection *client, const char *req, size_t req_length) {
	if (fprintf(client->sender, "Content-Length: %zu\r\n\r\n", req_length) <
		0) {
		return -errno;
	}
	if (fwrite(req, 1, req_length, client->sender) != req_length) {
		return -errno;
	}
	fflush(client->sender);
	return 0;
}

int
lsp_connection_receive(
		struct LSPCConnection *client, char **res, size_t *res_length) {
	char buffer[1024];
	size_t content_length = 0;

	while (fgets(buffer, sizeof(buffer), client->receiver)) {
		// Ignore everything until we find the Content-Length header
		if (sscanf(buffer, "Content-Length: %zu", &content_length) != 1) {
			continue;
		}
		// Skip empty line afterwards
		if (fgets(buffer, sizeof(buffer), client->receiver) == NULL) {
			return -errno;
		}
		break;
	}

	// Allocate memory for the response
	*res = calloc(content_length + 1, sizeof(char));
	if (!*res) {
		return -errno;
	}

	if (fread(*res, 1, content_length, client->receiver) != content_length) {
		free(*res);
		return -errno;
	}
	(*res)[content_length] = '\0';
	*res_length = content_length;

	return 0;
}

void
lsp_connection_cleanup(struct LSPCConnection *client) {
	if (client->sender) {
		fclose(client->sender);
	}
	if (client->receiver) {
		fclose(client->receiver);
	}
	if (client->lsp_pid > 0) {
		kill(client->lsp_pid, SIGTERM);
		waitpid(client->lsp_pid, NULL, 0);
	}
}
