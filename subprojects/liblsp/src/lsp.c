#include <errno.h>
#include <lsp.h>
#include <lsp_initialization.h>
#include <lsp_util.h>
#include <signal.h>
#include <spawn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int
lsp_init(struct Lsp *lsp, const char *name, const char *version) {
	lsp->sender = NULL;
	lsp->receiver_fd = -1;
	lsp->pid = -1;
	lsp->running = true;
	lsp->name = name;
	lsp->version = version;
	rid_gen_init(&lsp->id_gen);
	lsp_recv_buffer_init(&lsp->recv_buf);
	return cx_hash_map_init(
			&lsp->pending_requests, 16, sizeof(struct LspPendingRequest));
}

void
lsp_stdio(struct Lsp *lsp) {
	lsp->sender = stdout;
	lsp->receiver_fd = STDIN_FILENO;
}

int
lsp_spawn(struct Lsp *lsp, const char *argv[]) {
	int rv = 0;
	int sender_pipe[2] = {0};
	int receiver_pipe[2] = {0};

	if (pipe(sender_pipe) != 0 || pipe(receiver_pipe) != 0) {
		rv = -errno;
		goto out_early;
	}

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	posix_spawn_file_actions_adddup2(&actions, sender_pipe[0], STDIN_FILENO);
	posix_spawn_file_actions_adddup2(&actions, receiver_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&actions, sender_pipe[1]);
	posix_spawn_file_actions_addclose(&actions, receiver_pipe[0]);

	if (posix_spawnp(
				&lsp->pid, argv[0], &actions, NULL, (char *const *)argv,
				environ) != 0) {
		rv = -errno;
		goto out;
	}

	close(sender_pipe[0]);
	close(receiver_pipe[1]);

	lsp->sender = fdopen(sender_pipe[1], "w");
	lsp->receiver_fd = receiver_pipe[0];

	if (!lsp->sender) {
		rv = -errno;
		goto out;
	}

out:
	posix_spawn_file_actions_destroy(&actions);
out_early:
	return rv;
}

int
lsp_send(struct Lsp *lsp, json_object *msg) {
	const char *str = json_object_to_json_string(msg);
	if (str == NULL) {
		return -ENOMEM;
	}
	size_t len = strlen(str);
	if (fprintf(lsp->sender, "Content-Length: %zu\r\n\r\n", len) < 0) {
		return -errno;
	}
	if (fwrite(str, 1, len, lsp->sender) != len) {
		return -errno;
	}
	fflush(lsp->sender);
	return 0;
}

uint64_t
lsp_next_id(struct Lsp *lsp) {
	return rid_gen_next(&lsp->id_gen);
}

int
lsp_notify(struct Lsp *lsp, json_object *notif) {
	int rv = lsp_send(lsp, notif);
	json_object_put(notif);
	return rv;
}

int
lsp_request(
		struct Lsp *lsp, json_object *request, LspResponseWrapper wrapper_fn,
		void *callback, void *userdata) {
	// Get the request ID
	json_object *id_obj = json_object_object_get(request, "id");
	if (id_obj == NULL) {
		json_object_put(request);
		return -EINVAL;
	}
	uint64_t id = (uint64_t)json_object_get_int64(id_obj);

	// Create pending request info
	struct LspPendingRequest pending = {
		.wrapper = wrapper_fn,
		.callback = callback,
		.userdata = userdata,
	};

	// Store in hashmap (copies the struct)
	void *stored = cx_hash_map_put(&lsp->pending_requests, id, &pending);
	if (stored == NULL) {
		json_object_put(request);
		return -ENOMEM;
	}

	// Send the request
	int rv = lsp_send(lsp, request);
	json_object_put(request);

	if (rv != 0) {
		cx_hash_map_delete(&lsp->pending_requests, id);
	}

	return rv;
}

int
lsp_fd(const struct Lsp *lsp) {
	return lsp->receiver_fd;
}

void
lsp_cleanup(struct Lsp *lsp) {
	lsp_recv_buffer_cleanup(&lsp->recv_buf);
	cx_hash_map_cleanup(&lsp->pending_requests);
	if (lsp->pid > 0) {
		// Client mode: close sender and receiver, kill subprocess
		if (lsp->sender) {
			fclose(lsp->sender);
			lsp->sender = NULL;
		}
		if (lsp->receiver_fd >= 0) {
			close(lsp->receiver_fd);
			lsp->receiver_fd = -1;
		}
		kill(lsp->pid, SIGTERM);
		waitpid(lsp->pid, NULL, 0);
		lsp->pid = -1;
	} else {
		// Server mode: don't close stdin/stdout
		lsp->sender = NULL;
		lsp->receiver_fd = -1;
	}
}

static void
handle_response(struct Lsp *lsp, json_object *msg) {
	json_object *id_obj = json_object_object_get(msg, "id");
	if (id_obj == NULL) {
		return;
	}
	uint64_t id = (uint64_t)json_object_get_int64(id_obj);

	struct LspPendingRequest *pending =
			cx_hash_map_get(&lsp->pending_requests, id);
	if (pending == NULL) {
		return;
	}

	// Call the wrapper with the response
	if (pending->wrapper != NULL) {
		pending->wrapper(msg, pending->callback, pending->userdata);
	}

	// Remove from pending
	cx_hash_map_delete(&lsp->pending_requests, id);
}

static json_object *
auto_handle_initialize(
		struct Lsp *lsp, json_object *msg,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs) {
	json_object *id = json_object_object_get(msg, "id");
	json_object *result = lsp_build_initialize_result(
			lsp->name, lsp->version, req_cbs, notif_cbs);
	return lsp_response_new(id, result);
}

static int
handle_c2s_request(
		struct Lsp *lsp, json_object *msg,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs, void *userdata) {
	int rv = 0;
	const char *method =
			json_object_get_string(json_object_object_get(msg, "method"));
	enum LspC2SMethod req_type = lsp_c2s_method_lookup(method);

	json_object *response = NULL;

	/* Auto-handle initialize if no callback registered */
	if (req_type == LSP_C2S_METHOD_INITIALIZE &&
			(!req_cbs || !req_cbs->initialize)) {
		response = auto_handle_initialize(lsp, msg, req_cbs, notif_cbs);
	} else {
		response = lsp_server_dispatch_request(
				req_cbs, req_type, msg, userdata);
	}

	if (response) {
		rv = lsp_send(lsp, response);
		json_object_put(response);
	}
	return rv;
}

static void
handle_c2s_notification(
		struct Lsp *lsp, json_object *msg,
		const struct LspServerNotificationCallbacks *notif_cbs, void *userdata) {
	const char *method =
			json_object_get_string(json_object_object_get(msg, "method"));
	enum LspC2SNotification notif_type = lsp_c2s_notification_lookup(method);
	if (notif_type == LSP_C2S_NOTIFICATION_EXIT) {
		lsp->running = false;
	}
	lsp_server_dispatch_notification(notif_cbs, notif_type, msg, userdata);
}

static int
handle_s2c_request(
		struct Lsp *lsp, json_object *msg,
		const struct LspClientRequestCallbacks *req_cbs, void *userdata) {
	int rv = 0;
	const char *method =
			json_object_get_string(json_object_object_get(msg, "method"));
	enum LspS2CMethod req_type = lsp_s2c_method_lookup(method);
	json_object *response =
			lsp_client_dispatch_request(req_cbs, req_type, msg, userdata);
	if (response) {
		rv = lsp_send(lsp, response);
		json_object_put(response);
	}
	return rv;
}

static void
handle_s2c_notification(
		json_object *msg, const struct LspClientNotificationCallbacks *notif_cbs,
		void *userdata) {
	const char *method =
			json_object_get_string(json_object_object_get(msg, "method"));
	enum LspS2CNotification notif_type = lsp_s2c_notification_lookup(method);
	lsp_client_dispatch_notification(notif_cbs, notif_type, msg, userdata);
}

int
lsp_server_process(
		struct Lsp *lsp,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs,
		void *userdata) {
	int rv = 0;

	rv = lsp_recv_buffer_read(&lsp->recv_buf, lsp->receiver_fd);
	if (rv < 0) {
		return rv;
	}

	json_object *msg = lsp_recv_buffer_extract(&lsp->recv_buf);
	if (!msg) {
		lsp_recv_buffer_reset(&lsp->recv_buf);
		return -EINVAL;
	}

	enum LspMessageKind kind = lsp_message_classify(msg);

	switch (kind) {
	case LSP_MESSAGE_REQUEST:
		rv = handle_c2s_request(lsp, msg, req_cbs, notif_cbs, userdata);
		break;
	case LSP_MESSAGE_NOTIFICATION:
		handle_c2s_notification(lsp, msg, notif_cbs, userdata);
		break;
	case LSP_MESSAGE_RESPONSE:
		handle_response(lsp, msg);
		break;
	case LSP_MESSAGE_INVALID:
		break;
	}

	json_object_put(msg);
	lsp_recv_buffer_reset(&lsp->recv_buf);
	return rv;
}

int
lsp_client_process(
		struct Lsp *lsp,
		const struct LspClientRequestCallbacks *req_cbs,
		const struct LspClientNotificationCallbacks *notif_cbs,
		void *userdata) {
	int rv = 0;

	rv = lsp_recv_buffer_read(&lsp->recv_buf, lsp->receiver_fd);
	if (rv < 0) {
		if (rv == -ECONNRESET) {
			lsp->running = false;
		}
		return rv;
	}

	json_object *msg = lsp_recv_buffer_extract(&lsp->recv_buf);
	if (!msg) {
		lsp_recv_buffer_reset(&lsp->recv_buf);
		return -EINVAL;
	}

	enum LspMessageKind kind = lsp_message_classify(msg);

	switch (kind) {
	case LSP_MESSAGE_REQUEST:
		rv = handle_s2c_request(lsp, msg, req_cbs, userdata);
		break;
	case LSP_MESSAGE_NOTIFICATION:
		handle_s2c_notification(msg, notif_cbs, userdata);
		break;
	case LSP_MESSAGE_RESPONSE:
		handle_response(lsp, msg);
		break;
	case LSP_MESSAGE_INVALID:
		break;
	}

	json_object_put(msg);
	lsp_recv_buffer_reset(&lsp->recv_buf);
	return rv;
}
