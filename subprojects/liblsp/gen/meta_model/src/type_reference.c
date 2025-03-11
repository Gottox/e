#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int reference_resolve_iter(struct Jw *jw, struct JwVal *val, int index, void *data) {
	int rv = 0;
	struct Type *type = data;
	struct Generator *generator = type->generator;
	char *name = NULL;
	rv = jw_obj_get_str(generator->jw, val, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}

	if (strcmp(name, type->name) == 0) {
		rv = 1;
		jw_dup(generator->jw, &type->definition, val);

		goto out;
	}


out:
	free(name);
	return rv;
}

static int reference_resolve(struct Type *type, const char *kind_haystack) {
	int rv = 0;
	struct Generator *generator = type->generator;
	struct JwVal items = {0};

	rv = jw_obj_get(generator->jw, type->generator->meta_model, kind_haystack, &items);
	if (rv < 0) {
		goto out;
	}

	rv = jw_arr_foreach(generator->jw, &items, reference_resolve_iter, type);
	if (rv < 0) {
		goto out;
	} else if (rv == 0) {
		// No match
		rv = -1;
		goto out;
	}

out:
	jw_cleanup(generator->jw, &items);
	return rv;
}

int
type_reference_init(struct Type *type) {
	int rv = 0;
	struct Generator *generator = type->generator;
	struct JwVal *definition = &type->definition;
	char *name = NULL;
	rv = jw_obj_get_str(generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}

	rv = type_name_copy(type, name);
	if (rv < 0) {
		goto out;
	}
	
	jw_cleanup(generator->jw, definition);

	rv = reference_resolve(type, "structures");
	if (rv >= 0) {
		rv = type_structure_init(type);
		goto out;
	}

	rv = reference_resolve(type, "enumerations");
	if (rv >= 0) {
		rv = type_enumeration_init(type);
		goto out;
	}

	rv = reference_resolve(type, "typeAliases");
	if (rv >= 0) {
		rv = type_alias_init(type);
		goto out;
	}


	rv = -1;

out:
	free(name);
	return rv;
}
