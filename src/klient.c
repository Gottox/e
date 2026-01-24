#include "rope.h"
#include "rope_str.h"
#include <e_klient.h>
#include <e_struktur.h>
#include <e_konstrukt.h>
#include <e_list.h>
#include <stdio.h>
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

	rope_append_str(&e->klient->output_buffer, "Hello World!\n");

out:
	return rv;
}

static int
e_klient_notify(union EStruktur *e, struct EMessage *message) {
	int rv = 0;
	char *msg_data;
	E_TYPE_ASSERT(e);
	struct EKonstrukt *k = e->base->konstrukt;

	struct Rope *rope = &message->content;

	// TODO: this currently copies the entire message into the output buffer.
	// instead librope should add an API to get the RopeStr from the iterator
	// and a way to insert RopeStr directly into the output buffer rope.
	size_t size = rope_size(rope, ROPE_BYTE);
	msg_data = rope_to_str(rope, 0);
	if (msg_data == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	rv = rope_append(&e->klient->output_buffer, (uint8_t *)msg_data, size);
	if (rv < 0) {
		goto out;
	}

out:
	free(msg_data);
	return rv;
}

int
e_klient_read_input(union EStruktur *e) {
	int rv = 0;
	uint8_t buffer[ROPE_STR_FAST_SIZE];
	struct RopeCursor cursor = {0};
	E_TYPE_ASSERT(e);

	struct Rope *rope = &e->klient->input_buffer;

	rv = read(e->klient->writer_fd, buffer, sizeof(buffer));
	if (rv < 0) {
		rv = -errno;
		goto out;
	}
	size_t bytes_read = (size_t)rv;

	rv = rope_append(rope, buffer, bytes_read);
	if (rv < 0) {
		goto out;
	}

out:
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
	rv = rope_cursor_move_to(&cursor, ROPE_BYTE, 0, 0);
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
}

E_TYPE_END(klient);
