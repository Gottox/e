#include <e_klient.h>
#include <e_konstrukt.h>
#include <e_command.h>
#include <e_list.h>
#include <e_struktur.h>
#include <fcntl.h>
#include <rope.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

E_TYPE_BEGIN(klient);

int
e_klient_new(
		union EStruktur *e, struct EKonstrukt *k, int reader_fd,
		int writer_fd) {
	int rv = E_TYPE_ALLOC(e, k);

	e->klient->reader_fd = reader_fd;
	e->klient->writer_fd = writer_fd;

	rv = e_list_add(&k->klients, e);
	if (rv < 0) {
		goto out;
	}
	rv = rope_init(&e->klient->output_buffer, &k->rope_pool);
	if (rv < 0) {
		goto out;
	}
	rv = rope_init(&e->klient->input_buffer, &k->rope_pool);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

static int
e_klient_notify(union EStruktur *e, struct Rope *message) {
	E_TYPE_ASSERT(e);
	int rv = 0;
	struct EKlient *klient = e->klient;

	struct RopeRange source = {0};
	struct RopeCursor output = {0};

	rv = rope_to_range(message, &source);
	if (rv < 0) {
		goto out;
	}

	rv = rope_to_end_cursor(&klient->output_buffer, &output);
	if (rv < 0) {
		goto out;
	}

	rv = rope_range_copy_to(&source, &output, 0);
	if (rv < 0) {
		goto out;
	}

out:
	rope_range_cleanup(&source);
	rope_cursor_cleanup(&output);
	return rv;
}

int
e_klient_handle_input(union EStruktur *e, struct pollfd *pfd) {
	E_TYPE_ASSERT(e);
	int rv = 0;
	struct EKonstrukt *k = e->base->konstrukt;
	uint8_t buffer[ROPE_STR_FAST_SIZE];
	struct RopeCursor cursor = {0};
	char *command_name = NULL;

	if ((pfd->revents & POLLIN) == 0) {
		return 0;
	}

	struct Rope *rope = &e->klient->input_buffer;

	rv = read(e->klient->reader_fd, buffer, sizeof(buffer));
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	size_t bytes_read = (size_t)rv;
	if (bytes_read == 0) {
		rv = -EPIPE;
		goto out;
	}

	rv = rope_append(rope, buffer, bytes_read);
	if (rv < 0) {
		goto out;
	}

	struct EMessageParser parser = {0};
	rv = e_message_parser_init(&parser, &e->klient->input_buffer);
	if (rv < 0) {
		goto out;
	}

	struct RopeRange field = {0};
	while (e_message_parser_next(&parser, &field, &rv)) {
		if (rv < 0) {
			break;
		}
		if (command_name == NULL) {
			command_name = rope_range_to_cstr(&field, 0);
		}
	}
	if (rv == -ROPE_ERROR_OOB) {
		// Incomplete message, wait for more data.
		rv = 0;
	} else if (rv < 0) {
		rope_append_str(&e->klient->output_buffer, "error \"parse error\"\n");
		rv = e_message_parse_consume(&parser);
	} else {
		bool found = false;
		for (size_t i = 0; k->commands[i].function; i++) {
			if (strcmp(command_name, k->commands[i].name) == 0) {
				rv = k->commands[i].function(e->base->konstrukt, e);
				found = true;
				break;
			}
		}
		if (!found) {
			rope_append_str(&e->klient->output_buffer, "error \"unknown command\"\n");
		}
		rv = e_message_parse_consume(&parser);
	}

	if (rv < 0) {
		goto out;
	}

out:
	free(command_name);
	rope_range_cleanup(&field);
	e_message_parser_cleanup(&parser);
	rope_cursor_cleanup(&cursor);
	return rv;
}

int
e_klient_flush_output(union EStruktur *e) {
	int rv = 0;
	char *output_data;
	struct RopeCursor cursor = {0};
	E_TYPE_ASSERT(e);

	struct Rope *rope = &e->klient->output_buffer;

	// TODO: same as above, avoid copying the entire rope to a string.
	size_t size = rope_size(rope, ROPE_BYTE);
	output_data = rope_to_str(rope, 0);
	if (output_data == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	rv = write(e->klient->writer_fd, output_data, size);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	size_t bytes_written = (size_t)rv;

	rv = rope_cursor_init(&cursor, rope);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_delete(&cursor, ROPE_BYTE, bytes_written);
	if (rv < 0) {
		goto out;
	}

out:
	rope_cursor_cleanup(&cursor);
	free(output_data);
	return rv;
}

static void
e_klient_cleanup(union EStruktur *e) {
	E_TYPE_ASSERT(e);
	close(e->klient->reader_fd);
	close(e->klient->writer_fd);
	rope_cleanup(&e->klient->output_buffer);
	rope_cleanup(&e->klient->input_buffer);
}

E_TYPE_END(klient);
