#include <cextras/memory.h>
#include <jw.h>
#include <stdbool.h>

#define GENERATOR_IDENTIFIER_MAX 256

struct Generator;

struct Type {
	struct Type *next;
	struct Type *prev;
	struct Generator *generator;
	bool is_generated;

	char name[GENERATOR_IDENTIFIER_MAX + 1];
	int id;

	int (*generate_type)(struct Type *type);
	int (*generate_field)(struct Type *type, const char *field_name);

	struct JwVal *definition;
};

struct Type *type_new(
		struct Generator *generator, char *kind, struct JwVal *definition,
		int *err);

void type_free(struct Type *type);

struct Generator {
	struct Type *type_head;
	struct Type *type_tail;
	size_t type_size;

	struct Jw *jw;
	struct JwVal *meta_model;

	struct CxPreallocPool type_pool;
};

int
generator_init(struct Generator *generator, struct Jw *jw, struct JwVal *obj);

struct Type *generator_lookup(
		struct Generator *generator, char *kind, struct JwVal *item, int *err);

int generator_load_types(struct Generator *generator);

int type_compound_init(
		struct Type *type, const char *name, const char *field,
		struct JwVal *compound);

int type_name_append(struct Type *type, const char *append);

int type_name_append_nbr(struct Type *type, int nbr);

int type_name_copy(struct Type *type, const char *copy);

int generator_cleanup(struct Generator *gen);

int type_base_init(struct Type *type);

int type_array_init(struct Type *type);

int type_literal_init(struct Type *type);

int type_map_init(struct Type *type);

int type_or_init(struct Type *type);

int type_reference_init(struct Type *type);

int type_string_literal_init(struct Type *type);

int type_structure_init(struct Type *type);

int type_tuple_init(struct Type *type);
