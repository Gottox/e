#include <errno.h>
#include <lsp_recv_buffer.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INITIAL_CAP 4096
#define READ_SIZE 4096

void
lsp_recv_buffer_init(struct LspRecvBuffer *buf) {
	buf->data = NULL;
	buf->len = 0;
	buf->cap = 0;
	buf->content_length = 0;
	buf->header_end = 0;
}

void
lsp_recv_buffer_cleanup(struct LspRecvBuffer *buf) {
	free(buf->data);
	buf->data = NULL;
	buf->len = 0;
	buf->cap = 0;
	buf->content_length = 0;
	buf->header_end = 0;
}

static int
ensure_capacity(struct LspRecvBuffer *buf, size_t needed) {
	if (buf->cap >= needed) {
		return 0;
	}
	size_t new_cap = buf->cap == 0 ? INITIAL_CAP : buf->cap;
	while (new_cap < needed) {
		new_cap *= 2;
	}
	char *new_data = realloc(buf->data, new_cap);
	if (!new_data) {
		return -ENOMEM;
	}
	buf->data = new_data;
	buf->cap = new_cap;
	return 0;
}

static void
parse_headers(struct LspRecvBuffer *buf) {
	if (buf->header_end != 0) {
		return; // Already parsed
	}
	// Look for \r\n\r\n
	for (size_t i = 0; i + 3 < buf->len; i++) {
		if (buf->data[i] == '\r' && buf->data[i + 1] == '\n' &&
				buf->data[i + 2] == '\r' && buf->data[i + 3] == '\n') {
			buf->header_end = i + 4;
			// Parse Content-Length
			const char *cl = "Content-Length:";
			size_t cl_len = strlen(cl);
			for (size_t j = 0; j < i; j++) {
				if (j + cl_len <= i &&
						strncasecmp(&buf->data[j], cl, cl_len) == 0) {
					buf->content_length = (size_t)atol(&buf->data[j + cl_len]);
					break;
				}
			}
			break;
		}
	}
}

int
lsp_recv_buffer_read(struct LspRecvBuffer *buf, int fd) {
	int rv = ensure_capacity(buf, buf->len + READ_SIZE);
	if (rv < 0) {
		return rv;
	}

	ssize_t n = read(fd, buf->data + buf->len, READ_SIZE);
	if (n < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -EAGAIN;
		}
		return -errno;
	}
	if (n == 0) {
		return -ECONNRESET; // EOF
	}

	buf->len += (size_t)n;

	// Try to parse headers if not yet done
	parse_headers(buf);

	// Check if message is complete
	if (buf->header_end != 0 && buf->content_length != 0) {
		size_t total_needed = buf->header_end + buf->content_length;
		if (buf->len >= total_needed) {
			return 0; // Complete
		}
	}

	return -EAGAIN; // Partial
}

json_object *
lsp_recv_buffer_extract(struct LspRecvBuffer *buf) {
	if (buf->header_end == 0 || buf->content_length == 0) {
		return NULL;
	}
	// Temporarily null-terminate the JSON content
	char *json_start = buf->data + buf->header_end;
	char saved = json_start[buf->content_length];
	json_start[buf->content_length] = '\0';

	json_object *obj = json_tokener_parse(json_start);

	json_start[buf->content_length] = saved;
	return obj;
}

void
lsp_recv_buffer_reset(struct LspRecvBuffer *buf) {
	if (buf->header_end == 0 || buf->content_length == 0) {
		// No complete message, just reset
		buf->len = 0;
		buf->content_length = 0;
		buf->header_end = 0;
		return;
	}

	size_t msg_end = buf->header_end + buf->content_length;
	if (buf->len > msg_end) {
		// Move remaining data to front
		memmove(buf->data, buf->data + msg_end, buf->len - msg_end);
		buf->len -= msg_end;
	} else {
		buf->len = 0;
	}
	buf->content_length = 0;
	buf->header_end = 0;
}
