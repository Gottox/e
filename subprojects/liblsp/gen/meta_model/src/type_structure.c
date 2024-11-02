#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
structure_generate_type(struct Type *type) {
	return 0;
}

static int
structure_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

static int
property_init_iter(struct Jw *jw, struct JwVal *val, int index, void *data) {
	int rv = 0;
	struct JwVal prop_type = {0};
	struct Type *type = data;
	rv = jw_obj_get(jw, val, "type", &prop_type);
	if (rv < 0) {
		goto out;
	}
	(void)generator_lookup(type->generator, NULL, &prop_type, &rv);
	if (rv < 0) {
		goto out;
	}
out:
	jw_cleanup(jw, &prop_type);
	return rv;
}

static int
property_init(struct Type *type, struct JwVal *property) {
	int rv = 0;
	struct JwVal properties = {0};
	rv = jw_obj_get(
			type->generator->jw, type->definition, "properties", &properties);
	if (rv < 0) {
		goto out;
	}
	rv = jw_arr_foreach(
			type->generator->jw, &properties, property_init_iter, type);
	if (rv < 0) {
		goto out;
	}
out:
	jw_cleanup(type->generator->jw, &properties);
	return rv;
}

int
type_structure_init(struct Type *type) {
	int rv = 0;
	char *name = NULL;
	type->generate_type = structure_generate_type;
	type->generate_field = structure_generate_field;

	// We ignore failing compound inits
	(void)type_compound_init(type, "", "extends", type->definition);
	(void)type_compound_init(type, "", "mixins", type->definition);

	rv = property_init(type, type->definition);
	if (rv < 0) {
		goto out;
	}

	rv = jw_obj_get_str(
			type->generator->jw, type->definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}
	type_name_copy(type, name);
out:
	free(name);
	return rv;
}
