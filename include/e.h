#include <quickjs/quickjs.h>
#include <rope.h>

struct pollfd;

struct EDaemonClient {
	int fd;
	struct EDaemonClient *next;
};

struct EDaemon {
	int fd;
	int client_count;
	struct EDaemonClient *clients;
	struct pollfd *client_fds;
	JSRuntime *js_rt;
	JSContext *js_ctx;
};

int e_client(int argc, char *argv[]);
int e_daemon(int argc, char *argv[]);
