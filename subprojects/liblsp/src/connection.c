#include <errno.h>
#include <lsp_private.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int
lsp_connection_client_init(
		struct LSPConnection *connection, const char *lsp_command[]) {
	int rv = 0;
	int sender_pipe[2] = {0};
	int receiver_pipe[2] = {0};

	if (pipe(sender_pipe) != 0 || pipe(receiver_pipe) != 0) {
		rv = -errno;
		goto out;
	}

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	// Redirect stdin and stdout to the pipes
	posix_spawn_file_actions_adddup2(&actions, sender_pipe[0], STDIN_FILENO);
	posix_spawn_file_actions_adddup2(&actions, receiver_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&actions, sender_pipe[1]);
	posix_spawn_file_actions_addclose(&actions, receiver_pipe[0]);

	if (posix_spawn(
				&connection->lsp_pid, lsp_command[0], &actions, NULL,
				(char *const *)lsp_command, environ) != 0) {
		rv = -errno;
		goto out;
	}

	close(sender_pipe[0]);
	close(receiver_pipe[1]);

	connection->sender = fdopen(sender_pipe[1], "w");
	connection->receiver = fdopen(receiver_pipe[0], "r");

	if (!connection->sender || !connection->receiver) {
		rv = -1;
		goto out;
	}

out:
	posix_spawn_file_actions_destroy(&actions);
	return rv;
}

int
lsp_connection_server_init(
		struct LSPConnection *connection, FILE *in, FILE *out) {
	if (out == NULL) {
		out = stdout;
	}
	if (in == NULL) {
		in = stdin;
	}
	connection->sender = out;
	connection->receiver = in;
	connection->lsp_pid = -1;
	return 0;
}

int
lsp_connection_send(
		struct LSPConnection *connection, const char *req, size_t req_length) {
	if (fprintf(connection->sender, "Content-Length: %zu\r\n\r\n", req_length) <
		0) {
		return -errno;
	}
	if (fwrite(req, 1, req_length, connection->sender) != req_length) {
		return -errno;
	}
	fflush(connection->sender);
	return 0;
}

int
lsp_connection_receive(
		struct LSPConnection *connection, char **res, size_t *res_length) {
	char buffer[32];
	size_t content_length = 0;
	bool content_length_found = false;
	bool is_new_line = true;

	while (fgets(buffer, sizeof(buffer), connection->receiver)) {
		// Skip headers that are bigger than the buffer. The headers that we are
		// interested in should fit in the buffer.
		int buffer_len = strlen(buffer);
		if (buffer_len == 0 || buffer[buffer_len - 1] != '\n') {
			is_new_line = false;
			continue;
		} else if (!is_new_line) {
			is_new_line = true;
			continue;
		}

		if (sscanf(buffer, "Content-Length: %zu\r\n", &content_length) == 1) {
			content_length_found = true;
		} else if (strcmp(buffer, "\r\n") == 0) {
			break; // End of headers
		}
	}

	if (!content_length_found) {
		return -EINVAL;
	}

	// Allocate memory for the response
	*res = calloc(content_length + 1, sizeof(char));
	if (!*res) {
		return -errno;
	}

	if (fread(*res, 1, content_length, connection->receiver) !=
		content_length) {
		free(*res);
		return -errno;
	}
	(*res)[content_length] = '\0';
	*res_length = content_length;

	return 0;
}

void
lsp_connection_cleanup(struct LSPConnection *connection) {
	if (connection->sender) {
		fclose(connection->sender);
	}
	if (connection->receiver) {
		fclose(connection->receiver);
	}
	if (connection->lsp_pid > 0) {
		kill(connection->lsp_pid, SIGTERM);
		waitpid(connection->lsp_pid, NULL, 0);
	}
}
