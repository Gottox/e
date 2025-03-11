#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
string_literal_generate_type(struct Type *type) {
	printf("string_literal %s\n", type->name);
	return 0;
}

static int
string_literal_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_string_literal_init(struct Type *type) {
	struct Generator *generator = type->generator;
	struct JwVal *definition = &type->definition;
	char *value = NULL;

	type->generate_type = string_literal_generate_type;
	type->generate_field = string_literal_generate_field;

	int rv = jw_obj_get_str(generator->jw, definition, "value", &value, NULL);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_copy(type, value);
	if (rv < 0) {
		goto out;
	}

out:
	free(value);
	return 0;
}
