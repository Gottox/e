#ifndef E_STRUKTUR_H
#define E_STRUKTUR_H

#include "e_common.h"
#include "e_list.h"
#include "e_message.h"

union EStruktur;

struct EStrukturType {
	int (*notify)(union EStruktur *, struct Rope *);
	void (*cleanup)(union EStruktur *);
};

union EStrukturStorage {
#define STRUCT(type, name, ...) struct type __VA_ARGS__ name;
#include "e_struktur.struct.h"
#undef STRUCT
};

union EStruktur {
	union EStrukturStorage *any;
#define STRUCT(type, name, ...) struct type *name;
#include "e_struktur.struct.h"
#undef STRUCT
};

#define STRUCT(type, name, ...) \
	extern const struct EStrukturType e_struktur_type_##name;
#include "e_struktur.struct.h"
#undef STRUCT

#define E_TYPE_BEGIN(type) \
	const struct EStrukturType e_struktur_type_##type; \
	static const struct EStrukturType *const e_type = &e_struktur_type_##type

#define E_TYPE_INIT(e) \
	do { \
		(e)->base->type = e_type; \
	} while (0)

#define E_TYPE_ASSERT(e) \
	do { \
		assert((e)->base->type == e_type); \
	} while (0)

#define E_TYPE_ALLOC(e, k) e_struktur_alloc((e), (k), e_type)

#define E_TYPE_END(type) \
	const struct EStrukturType e_struktur_type_##type = { \
			.cleanup = e_##type##_cleanup, \
			.notify = e_##type##_notify, \
	}

int e_struktur_alloc(
		union EStruktur *e, struct EKonstrukt *k,
		const struct EStrukturType *type);

void
e_struktur_release(union EStruktur *e);

#endif /* E_STRUKTUR_H */
