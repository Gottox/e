#ifndef E_KLIENT_H
#define E_KLIENT_H

#include "e_konstrukt.h"
#include "e_struktur.h"

enum EKlientMode {
	E_DOKUMENT_MODE_INLINE,
	E_DOKUMENT_MODE_STOP_WORD,
	E_DOKUMENT_MODE_SIZED,
};

int e_klient_new(
		union EStruktur *e, struct EKonstrukt *k, int reader_fd, int writer_fd);

int e_klient_flush_output(union EStruktur *e);

int e_klient_handle_input(union EStruktur *e, struct pollfd *pfd);

#endif /* E_KLIENT_H */
