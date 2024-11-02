#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int
type_name_append(struct Type *type, const char *append) {
	if (strlen(type->name) + strlen(append) > GENERATOR_IDENTIFIER_MAX) {
		return -1;
	}
	strncat(type->name, append, GENERATOR_IDENTIFIER_MAX - strlen(type->name));
	type->name[GENERATOR_IDENTIFIER_MAX] = '\0';
	return 0;
}

int
type_name_append_nbr(struct Type *type, int nbr) {
	char buf[4] = {0};
	if (strlen(type->name) + 3 > GENERATOR_IDENTIFIER_MAX) {
		return -1;
	}
	if (nbr > 0xfff) {
		return -1;
	}
	snprintf(buf, sizeof(buf), "%03x", nbr);
	return type_name_append(type, buf);
}

int
type_name_copy(struct Type *type, const char *copy) {
	if (strlen(copy) >= GENERATOR_IDENTIFIER_MAX) {
		return -1;
	}
	strncpy(type->name, copy, GENERATOR_IDENTIFIER_MAX);
	return 0;
}

static int
type_compound_iter_init(
		struct Jw *jw, struct JwVal *val, int index, void *data) {
	int rv = 0;
	struct Type *type = data;
	struct Type *item = generator_lookup(type->generator, NULL, val, &rv);
	if (rv < 0) {
		goto out;
	}
	// rv = type_name_append_nbr(data, item->id);
	rv = type_name_append(data, "_");
	rv = type_name_append(data, item->name);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

int
type_compound_init(
		struct Type *type, const char *name, const char *field,
		struct JwVal *compound) {
	int rv = 0;
	struct JwVal types = {0};
	struct Generator *generator = type->generator;
	rv = jw_obj_get(generator->jw, compound, field, &types);
	if (rv < 0) {
		goto out;
	}

	rv = type_name_copy(type, name);
	if (rv < 0) {
		goto out;
	}

	if (jw_is_arr(generator->jw, &types)) {
		rv = jw_arr_foreach(
				generator->jw, &types, type_compound_iter_init, type);
		if (rv < 0) {
			goto out;
		}
	}
out:
	jw_cleanup(generator->jw, &types);
	return rv;
}

struct Type *
type_new(
		struct Generator *generator, char *kind, struct JwVal *definition,
		int *err) {
	int rv = 0;
	struct Type *type = NULL;

	type = cx_prealloc_pool_get(&generator->type_pool);
	if (type == NULL) {
		rv = -1;
		goto out;
	}
	memset(type, 0, sizeof(*type));

	type->generator = generator;
	type->definition = definition;

	if (strcmp(kind, "structure") == 0) {
		rv = type_structure_init(type);
		//} else if (strcmp(kind, "enumeration") == 0) {
		//	rv = type_enumeration_init(type);
		//} else if (strcmp(kind, "typeAlias") == 0) {
		//	rv = type_alias_init(type);
	} else if (strcmp(kind, "base") == 0) {
		rv = type_base_init(type);
	} else if (strcmp(kind, "array") == 0) {
		rv = type_array_init(type);
	} else if (strcmp(kind, "literal") == 0) {
		rv = type_literal_init(type);
	} else if (strcmp(kind, "map") == 0) {
		rv = type_map_init(type);
	} else if (strcmp(kind, "or") == 0) {
		rv = type_or_init(type);
	} else if (strcmp(kind, "reference") == 0) {
		rv = type_reference_init(type);
	} else if (strcmp(kind, "stringLiteral") == 0) {
		rv = type_string_literal_init(type);
	} else if (strcmp(kind, "tuple") == 0) {
		rv = type_tuple_init(type);
	} else {
		fprintf(stderr, "Unknown kind: %s\n", kind);
		rv = -1;
	}
out:
	if (rv < 0) {
		type_free(type);
		type = NULL;
	}
	if (err != NULL) {
		*err = rv;
	}
	return type;
}

void
type_free(struct Type *type) {
	if (type != NULL) {
		// jw_cleanup(type->generator->jw, type->definition);
		cx_prealloc_pool_recycle(&type->generator->type_pool, type);
	}
}
