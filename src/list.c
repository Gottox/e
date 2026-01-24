#include <e_konstrukt.h>
#include <e_list.h>
#include <e_struktur.h>
#include <stdlib.h>
#include <string.h>

int
e_list_add(struct EList *list, union EStruktur *e) {
	for (size_t i = 0; i < list->cap; i++) {
		if (list->ids[i] == 0) {
			list->ids[i] = e->base->id;
			return 0;
		}
	}
	size_t new_cap = CX_MAX(1, list->cap * 2);
	list->ids = realloc(list->ids, sizeof(uint64_t) * new_cap);
	if (!list->ids) {
		return -1;
	}
	memset(&list->ids[list->cap], 0, sizeof(uint64_t) * (new_cap - list->cap));
	list->ids[list->cap] = e->base->id;
	list->cap = new_cap;
	return 0;
}

bool
e_list_it(
		union EStruktur *e, struct EKonstrukt *k, struct EList *list,
		uint64_t *it) {
	e->any = NULL;
	if (*it != 0) {
		cx_rc_hash_map_release_key(&k->struktur, list->ids[*it - 1]);
	}
	for (; *it < list->cap; (*it)++) {
		if (list->ids[*it] == 0) {
			continue;
		}
		e->any = cx_rc_hash_map_retain(&k->struktur, list->ids[*it]);
		if (e->any == NULL) {
			list->ids[*it] = 0;
		} else {
			break;
		}
	}
	*it += 1;
	return e->any != NULL;
}

void
e_list_cleanup(struct EList *list) {
	free(list->ids);
}
