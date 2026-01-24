#include <e_konstrukt.h>
#include <e_struktur.h>

int
e_struktur_alloc(
		union EStruktur *e, struct EKonstrukt *k,
		const struct EStrukturType *type) {
	const uint32_t id = e_rand_gen_next(&k->rand_gen);
	union EStrukturStorage e_store = {
			.base = {
					.type = type,
					.id = id,
			}};
	e->any = cx_rc_hash_map_put(&k->struktur, id, &e_store);
	if (!e->any) {
		return -ENOMEM;
	}
	return 0;
}
