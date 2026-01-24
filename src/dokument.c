#include "e_list.h"
#include <e_struktur.h>

E_TYPE_BEGIN(dokument);

int
e_dokument_new(union EStruktur *e, struct EKonstrukt *k) {
	int rv = E_TYPE_ALLOC(e, k);
	return rv;
}

static int
e_dokument_notify(union EStruktur *e, struct EMessage *message) {
	E_TYPE_ASSERT(e);
	int rv = 0;
	struct EKonstrukt *k = e->base->konstrukt;
	union EStruktur klient;
	for(uint64_t it = 0; e_list_it(&klient, k, &e->dokument->klients, &it); ) {
		rv = klient.base->type->notify(&klient, message);
		if (rv < 0) {
			break;
		}
	}
	return rv;
}

static void
e_dokument_cleanup(union EStruktur *e) {
	E_TYPE_ASSERT(e);
}

E_TYPE_END(dokument);
