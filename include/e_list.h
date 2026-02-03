#ifndef E_LIST_H
#define E_LIST_H

#include "e_common.h"

struct EKonstrukt;
struct EBase;
union EStruktur;

struct EList {
	uint64_t *ids;
	size_t cap;
};

int e_list_add(struct EList *list, union EStruktur *e);

bool e_list_it(
		union EStruktur *e, struct EKonstrukt *k, struct EList *list,
		uint64_t *it);

int e_list_get(
		union EStruktur *e, struct EKonstrukt *k, struct EList *list,
		size_t index);

void e_list_cleanup(struct EList *list);

#endif
