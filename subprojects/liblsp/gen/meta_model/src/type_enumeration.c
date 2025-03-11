#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
enumeration_generate_type(struct Type *type) {
	printf("enumeration %s\n", type->name);
	return 0;
}

static int
enumeration_generate_field(struct Type *type, const char *field_name) {
	return 0;
}


int
type_enumeration_init(struct Type *type) {
	int rv = 0;
	char *name = NULL;
	struct JwVal *definition = &type->definition;
	type->generate_type = enumeration_generate_type;
	type->generate_field = enumeration_generate_field;

	rv = jw_obj_get_str(
			type->generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}
	type_name_copy(type, name);
out:
	free(name);
	return rv;
}
