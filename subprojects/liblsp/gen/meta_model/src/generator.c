#include <jw_quickjs.h>
#include <metamodel_gen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
generator_init(
		struct Generator *generator, struct Jw *jw, struct JwVal *meta_model) {
	generator->jw = jw;
	generator->meta_model = meta_model;
	generator->type_head = NULL;
	generator->type_tail = NULL;
	generator->type_size = 0;

	cx_prealloc_pool_init(&generator->type_pool, sizeof(struct Type));
	return 0;
}

static void
push_type(struct Generator *generator, struct Type *type) {
	if (generator->type_head == NULL) {
		generator->type_head = type;
		generator->type_tail = type;
	} else {
		generator->type_tail->next = type;
		type->prev = generator->type_tail;
		generator->type_tail = type;
	}
	generator->type_size++;
}

struct GeneratorUserData {
	struct Generator *generator;
	char *kind;
};

static int
generator_load_type_iter(
		struct Jw *jw, struct JwVal *item, int index, void *data) {
	struct GeneratorUserData *ud = data;
	int rv = 0;
	generator_lookup(ud->generator, ud->kind, item, &rv);
	return rv;
}

int
generator_load_types(struct Generator *generator) {
	int rv = 0;
	struct GeneratorUserData data = {
			.generator = generator,
	};

	struct JwVal structures = {0};
	rv = jw_obj_get(
			generator->jw, generator->meta_model, "structures", &structures);
	if (rv < 0) {
		rv = -1;
		goto out;
	}

	data.kind = "structure";
	jw_arr_foreach(generator->jw, &structures, generator_load_type_iter, &data);

out:
	jw_cleanup(generator->jw, &structures);
	return rv;
}

struct Type *
generator_lookup(
		struct Generator *generator, char *kind, struct JwVal *item, int *err) {
	int rv = 0;
	struct Type *type = NULL, *candidate = NULL;

	if (kind == NULL) {
		rv = jw_obj_get_str(generator->jw, item, "kind", &kind, NULL);
		if (rv < 0) {
			goto out;
		}
	} else {
		kind = strdup(kind);
	}

	candidate = type_new(generator, kind, item, &rv);
	if (rv < 0) {
		goto out;
	}

	type = generator->type_tail;
	while (type != NULL) {
		if (strcmp(type->name, candidate->name) == 0) {
			break;
		}
		type = type->prev;
	}

	if (type == NULL) {
		type = candidate;
		type->id = generator->type_size;
		push_type(generator, type);
		candidate = NULL;
	}

out:
	free(kind);
	if (err) {
		*err = rv;
	}
	if (candidate != NULL) {
		type_free(candidate);
	}
	return type;
}

int
generator_cleanup(struct Generator *gen) {
	while (gen->type_head != NULL) {
		struct Type *type = gen->type_head;
		gen->type_head = type->next;
		type_free(type);
	}
	cx_prealloc_pool_cleanup(&gen->type_pool);
	return 0;
}
