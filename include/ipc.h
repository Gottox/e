#ifndef E_IPC_H
#define E_IPC_H
#include <rope.h>
#include <e.h>

struct EIpcChannel {
	int fd;
	struct Rope read_buffer;
	struct Rope write_buffer;
	size_t expected_receive_size;
};

struct EIpcMessage {
	char command[16];
	size_t arg_count;
	size_t *arg_sizes;
	struct Rope payload;
};

#endif /* E_IPC_H */
