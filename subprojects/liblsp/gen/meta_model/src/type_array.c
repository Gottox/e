#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
array_generate_type(struct Type *type) {
	printf("array %s\n", type->name);
	return 0;
}

static int
array_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_array_init(struct Type *type) {
	int rv = 0;
	struct JwVal element = {0};
	struct Generator *generator = type->generator;
	struct JwVal *definition = &type->definition;
	rv = jw_obj_get(generator->jw, definition, "element", &element);
	if (rv < 0) {
		goto out;
	}
	struct Type *element_type =
			generator_lookup(generator, NULL, &element, &rv);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_copy(type, element_type->name);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_append(type, "Array");
	if (rv < 0) {
		goto out;
	}

	type->generate_type = array_generate_type;
	type->generate_field = array_generate_field;

out:
	jw_cleanup(generator->jw, &element);
	return rv;
}
