#include <e.h>
#include <e_command.h>

int
e_command_ping(struct EKonstrukt *konstrukt, union EStruktur *e) {
	int rv = 0;
	assert(e->base->type == &e_struktur_type_klient);

	rv = rope_append_str(&e->klient->output_buffer, "pong\n");

	(void)konstrukt;
	return rv;
}

int
e_command_endpoint(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}

int
e_command_subscribe(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}

int
e_command_request(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}

int
e_command_notify(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}

int
e_command_send(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}

int
e_command_respond(struct EKonstrukt *konstrukt, union EStruktur *e) {
	(void)konstrukt;
	(void)e;
	return 0;
}
