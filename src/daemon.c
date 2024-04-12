#include <e.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define SOCKET_NAME "echo_socket"

static int
e_daemon_rebuild_pollfds(struct EDaemon *daemon) {
	size_t nfds = daemon->client_count + 1;
	daemon->client_fds = (struct pollfd *)reallocarray(
			daemon->client_fds, nfds, sizeof(struct pollfd));
	if (!daemon->client_fds) {
		perror("Failed to allocate memory for pollfds");
		exit(EXIT_FAILURE);
	}

	int i = 0;
	for (struct EDaemonClient *current = daemon->clients; current != NULL;
		 current = current->next) {
		daemon->client_fds[i].fd = current->fd;
		daemon->client_fds[i].events = POLLIN;
		i++;
	}
	daemon->client_fds[i].fd = daemon->fd;
	daemon->client_fds[i].events = POLLIN;
	return nfds;
}

int
e_daemon_init(struct EDaemon *daemon, int listen_fd) {
	daemon->client_count = 0;
	daemon->clients = NULL;
	daemon->client_fds = NULL;
	daemon->fd = listen_fd;
	daemon->js_rt = JS_NewRuntime();
	daemon->js_ctx = JS_NewContext(daemon->js_rt);

	return 0;
}

struct EDaemonClient *
e_daemon_client_create(int fd) {
	struct EDaemonClient *new_client =
			(struct EDaemonClient *)malloc(sizeof(struct EDaemonClient));
	if (!new_client) {
		perror("Failed to allocate memory for client context");
		exit(EXIT_FAILURE);
	}
	new_client->fd = fd;
	new_client->next = NULL;
	return new_client;
}

void
e_daemon_add_client(struct EDaemon *daemon, int fd) {
	struct EDaemonClient *new_client = e_daemon_client_create(fd);
	new_client->next = daemon->clients;
	daemon->clients = new_client;
	daemon->client_count++;
}

void
e_daemon_remove_client(struct EDaemon *daemon, struct EDaemonClient *client) {
	if (daemon->clients == client) {
		daemon->clients = client->next;
	} else {
		struct EDaemonClient *temp = daemon->clients;
		while (temp->next != client) {
			temp = temp->next;
		}
		temp->next = client->next;
	}
	close(client->fd);
	free(client);
	daemon->client_count--;
}

int
e_daemon_cleanup(struct EDaemon *daemon) {
	while (daemon->clients != NULL) {
		e_daemon_remove_client(daemon, daemon->clients);
	}
	free(daemon->client_fds);
	close(daemon->fd);

	JS_FreeContext(daemon->js_ctx);
	JS_FreeRuntime(daemon->js_rt);
	return 0;
}

void
e_client_handle_chunk(struct EDaemon *daemon, struct EDaemonClient *client) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = read(client->fd, buffer, BUFFER_SIZE);
	if (bytes_read <= 0) {
		e_daemon_remove_client(daemon, client);
	} else {
		write(client->fd, buffer, bytes_read);
	}
}

char *
get_socket_path(void) {
	char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (xdg_runtime_dir != NULL) {
		int len = strlen(xdg_runtime_dir) + strlen("/" SOCKET_NAME) + 1;
		char *socket_path = malloc(len * sizeof(char));

		strcpy(socket_path, xdg_runtime_dir);
		strcat(socket_path, "/" SOCKET_NAME);
		return socket_path;
	} else {
		return strdup("/tmp/" SOCKET_NAME);
	}
}

int
e_daemon_process(struct EDaemon *daemon) {
	e_daemon_rebuild_pollfds(daemon);
	int nfds = daemon->client_count + 1;
	int rv = poll(daemon->client_fds, nfds, 10000);
	int client_clount = daemon->client_count;
	if (rv < 0) {
		perror("Poll error");
		exit(EXIT_FAILURE);
	}

	int i = 0;
	struct EDaemonClient *next = NULL;
	for (struct EDaemonClient *current = daemon->clients; current;
		 i++, current = next) {
		next = current->next;
		if (daemon->client_fds[i].revents & POLLIN) {
			e_client_handle_chunk(daemon, current);
		}
	}
	if (daemon->client_fds[i].revents & POLLIN) {
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(
				daemon->fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0) {
			perror("Accept failed");
		} else {
			e_daemon_add_client(daemon, client_fd);
		}
	} else if (client_clount == 0) {
		return 0;
	}
	return 1;
}

int
e_daemon(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	int rv;
	int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct EDaemon daemon = {0};
	struct sockaddr_un server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	char *socket_path = get_socket_path();
	strncpy(server_addr.sun_path, socket_path,
			sizeof(server_addr.sun_path) - 1);
	free(socket_path);

	unlink(server_addr.sun_path);

	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
		0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, 16) < 0) {
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	e_daemon_init(&daemon, listen_fd);

	do {
		rv = e_daemon_process(&daemon);
	} while (rv > 0);

	e_daemon_cleanup(&daemon);
	return 0;
}
