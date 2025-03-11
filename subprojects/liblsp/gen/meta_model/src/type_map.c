#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
map_generate_type(struct Type *type) {
	printf("map %s\n", type->name);
	return 0;
}

static int
map_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_map_init(struct Type *type) {
	int rv = 0;
	struct Generator *generator = type->generator;
	struct JwVal *definition = &type->definition;
	struct JwVal key_def = {0};
	struct JwVal value_def = {0};

	type->generate_type = map_generate_type;
	type->generate_field = map_generate_field;

	rv = jw_obj_get(generator->jw, definition, "key", &key_def);
	if (rv < 0) {
		goto out;
	}
	struct Type *key = generator_lookup(generator, NULL, &key_def, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = jw_obj_get(generator->jw, definition, "value", &value_def);
	if (rv < 0) {
		goto out;
	}
	struct Type *value = generator_lookup(generator, NULL, &value_def, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = type_name_copy(type, key->name);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_append(type, "To");
	if (rv < 0) {
		goto out;
	}
	rv = type_name_append(type, value->name);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_append(type, "Map");
	if (rv < 0) {
		goto out;
	}

out:
	jw_cleanup(generator->jw, &key_def);
	jw_cleanup(generator->jw, &value_def);
	return rv;
}
