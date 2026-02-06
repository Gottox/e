#ifndef LSP_RECV_BUFFER_H
#define LSP_RECV_BUFFER_H

#include <json.h>
#include <stddef.h>

#ifndef LSP_NO_UNUSED
#define LSP_NO_UNUSED __attribute__((warn_unused_result))
#endif

struct LspRecvBuffer {
	char *data;
	size_t len;
	size_t cap;
	size_t content_length; // Parsed from headers (0 if not yet found)
	size_t header_end;     // Offset where \r\n\r\n ends (0 if not found)
};

// Initialize buffer
void lsp_recv_buffer_init(struct LspRecvBuffer *buf);

// Free buffer resources
void lsp_recv_buffer_cleanup(struct LspRecvBuffer *buf);

// Read and accumulate data from fd
// Returns 0 when complete message received, -EAGAIN when partial, negative errno on error
LSP_NO_UNUSED int lsp_recv_buffer_read(struct LspRecvBuffer *buf, int fd);

// Parse JSON from completed message
// Returns NULL on parse error
json_object *lsp_recv_buffer_extract(struct LspRecvBuffer *buf);

// Reset buffer for next message
void lsp_recv_buffer_reset(struct LspRecvBuffer *buf);

#endif // LSP_RECV_BUFFER_H
