#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
literal_generate_type(struct Type *type) {
	return 0;
}

static int
literal_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_literal_init(struct Type *type) {
	int rv = 0;
	struct JwVal value = {0};
	type->generate_type = literal_generate_type;
	type->generate_field = literal_generate_field;

	rv = jw_obj_get(type->generator->jw, type->definition, "value", &value);
	if (rv < 0) {
		goto out;
	}

	jw_debug(type->generator->jw, &value);
	rv = type_compound_init(type, "Literal", "properties", &value);
	if (rv < 0) {
		goto out;
	}
out:
	jw_cleanup(type->generator->jw, &value);
	exit(1);
	return rv;
}
