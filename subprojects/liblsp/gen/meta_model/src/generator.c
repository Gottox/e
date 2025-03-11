#include <jw_quickjs.h>
#include <metamodel_gen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
generator_init(
		struct Generator *generator, struct Jw *jw, struct JwVal *meta_model) {
	memset(generator, 0, sizeof(struct Generator));

	generator->jw = jw;
	generator->meta_model = meta_model;

	cx_prealloc_pool_init(&generator->type_pool, sizeof(struct Type));
	return 0;
}

static void
list_push(struct GeneratorTypeList *list, struct Type *type) {
	if (list->head == NULL) {
		list->head = type;
		list->tail = type;
		type->prev = NULL;
		type->next = NULL;
	} else {
		list->tail->next = type;
		type->prev = list->tail;
		type->next = NULL;
		list->tail = type;
	}
	list->size++;
}

static void
list_remove(struct GeneratorTypeList *list, struct Type *type) {
	if (type->prev != NULL) {
		type->prev->next = type->next;
	} else {
		list->head = type->next;
	}
	if (type->next != NULL) {
		type->next->prev = type->prev;
	} else {
		list->tail = type->prev;
	}
	type->next = NULL;
	type->prev = NULL;
	list->size--;
}

static struct Type *
list_search_clone(struct GeneratorTypeList *list, struct Type *type) {
	struct Type *current = list->tail;
	fputs("  search clone\n", stderr);
	while (current != NULL) {
		fprintf(stderr, "  %s %s\n", current->name, type->name);
		if (strcmp(current->name, type->name) == 0 && current != type) {
			fputs("  not found\n", stderr);
			return current;
		}
		current = current->prev;
	}
	fputs("  not found\n", stderr);

	return NULL;
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
	(void)generator_lookup(ud->generator, ud->kind, item, &rv);
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

static int depth = 0;

struct Type *
generator_lookup(
		struct Generator *generator, char *kind, struct JwVal *item, int *err) {
	int rv = 0;
	struct Type *type = NULL, *candidate = NULL;
	depth++;

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

	list_push(&generator->preparation_list, candidate);
	rv = type_init(candidate);
	type = list_search_clone(&generator->preparation_list, candidate);

	if (type != NULL) {
		printf("Found recursion: %s %s\n", kind, candidate->name);
		for (struct Type *prep = generator->preparation_list.head; prep != NULL;
			 prep = prep->next) {
			printf("  %s\n", prep->name);
		}
		sleep(1);
	} else {
		if (rv < 0) {
			goto out;
		}

		type = list_search_clone(&generator->type_list, candidate);
	}
	list_remove(&generator->preparation_list, candidate);
	if (type == NULL) {
		type = candidate;
		type->id = generator->type_list.size;
		list_push(&generator->type_list, type);
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
	depth--;
	return type;
}

int
generator_cleanup(struct Generator *gen) {
	while (gen->type_list.head != NULL) {
		struct Type *type = gen->type_list.head;
		gen->type_list.head = type->next;
		type_free(type);
	}
	while (gen->preparation_list.head != NULL) {
		struct Type *type = gen->preparation_list.head;
		gen->preparation_list.head = type->next;
		type_free(type);
	}
	cx_prealloc_pool_cleanup(&gen->type_pool);
	return 0;
}
