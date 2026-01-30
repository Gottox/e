#include <e_konstrukt.h>
#include <e_struktur.h>
#include <stdio.h>

int
e_struktur_alloc(
		union EStruktur *e, struct EKonstrukt *k,
		const struct EStrukturType *type) {
	const uint32_t id = e_rand_gen_next(&k->rand_gen);
	union EStrukturStorage e_store = {
			.base = {
					.type = type,
					.id = id,
					.konstrukt = k,
			}};
	e->any = cx_rc_hash_map_put(&k->struktur, id, &e_store);
	if (!e->any) {
		return -ENOMEM;
	}
	return 0;
}

void
e_struktur_release(union EStruktur *e) {
	if (!e->any) {
		return;
	}
	struct EKonstrukt *k = e->base->konstrukt;
	cx_rc_hash_map_release_key(&k->struktur, e->any->base.id);
	e->any = NULL;
}
