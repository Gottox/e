#include <e_klient.h> // Temporary for debugging
#include <e_konstrukt.h>
#include <e_list.h>
#include <e_struktur.h>

#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int
e_handle_event(struct EKonstrukt *k) {
	size_t poll_list_cap = k->klients.cap * 2;

	// TODO: rather exitting directly, wait for new connections for a certain
	// time.
	if (poll_list_cap == 0) {
		k->running = false;
		return 0;
	}

	k->poll_list = realloc(k->poll_list, sizeof(struct pollfd) * poll_list_cap);
	if (k->poll_list == NULL) {
		return -ENOMEM;
	}
	memset(k->poll_list, 0, sizeof(struct pollfd) * poll_list_cap);

	struct pollfd *poll_list = k->poll_list;

	size_t idx = 0;
	union EStruktur e;
	for (uint64_t it = 0; e_list_it(&e, k, &k->klients, &it);) {
		poll_list[idx].fd = e.klient->reader_fd;
		poll_list[idx].events = POLLIN;
		idx++;
		if (rope_size(&e.klient->output_buffer, ROPE_BYTE) == 0) {
			continue;
		}
		poll_list[idx].fd = e.klient->writer_fd;
		poll_list[idx].events = POLLOUT;
		idx++;
	}
	size_t timeout_ms = -1;
	if (k->timeout_ms > 0) {
		timeout_ms = k->timeout_ms;
	}
	int rv = poll(poll_list, idx, timeout_ms);
	if (rv < 0) {
		return -errno;
	}

	idx = 0;
	for (uint64_t it = 0; e_list_it(&e, k, &k->klients, &it);) {
		if (poll_list[idx].revents & POLLIN) {
			rv = e_klient_read_input(&e);
		}
		idx++;
		if (rope_size(&e.klient->output_buffer, ROPE_BYTE) == 0) {
			continue;
		}
		if (poll_list[idx].revents & POLLOUT) {
			rv = e_klient_flush_output(&e);
			if (rv < 0) {
				goto out;
			}
		}
		idx++;
	}
out:
	return rv;
}

int
e_print_error(struct EKonstrukt *konstrukt, const char *format, ...) {
	(void)konstrukt;
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	return 0;
}

void
e_cleanup(struct EKonstrukt *konstrukt) {
	cx_rc_hash_map_cleanup(&konstrukt->struktur);
	e_list_cleanup(&konstrukt->dokuments);
	e_list_cleanup(&konstrukt->klients);
	rope_pool_cleanup(&konstrukt->rope_pool);
	free(konstrukt->poll_list);
}

int
e_main(struct EKonstrukt *konstrukt, int argc, char **argv) {
	int rv = e_init(konstrukt, argc, argv);
	if (rv < 0) {
		e_print_error(konstrukt, "Error initializing konstrukt: %d", rv);
		goto out;
	}

	union EStruktur e = {0};
	rv = e_klient_new(&e, konstrukt, STDIN_FILENO, STDOUT_FILENO);
	if (rv < 0) {
		e_print_error(
				konstrukt, "Error creating initial klient: %d", rv);
		goto out;
	}

	while (konstrukt->running) {
		rv = e_handle_event(konstrukt);
		if (rv < 0) {
			e_print_error(konstrukt, "Error handling event: %d", rv);
		}
		rv = 0;
	}

	e_cleanup(konstrukt);

out:
	return rv;
}

static void
struktur_cleanup(void *data) {
	union EStruktur e = {.any = data};
	e.base->type->cleanup(&e);
}

int
e_init(struct EKonstrukt *konstrukt, int argc, char **argv) {
	int rv = 0;

	rv = cx_rc_hash_map_init(
			&konstrukt->struktur, 1024, sizeof(union EStrukturStorage),
			struktur_cleanup);
	if (rv < 0) {
		goto out;
	}
	konstrukt->running = true;
	konstrukt->timeout_ms = 1000; // TODO: should be -1 by default.
	rv = e_rand_gen_init(&konstrukt->rand_gen);
	rope_pool_init(&konstrukt->rope_pool);
out:
	return rv;
}
