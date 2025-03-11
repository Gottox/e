#include "metamodel_gen.h"
#include <ctype.h>
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
base_generate_type(struct Type *type) {
	printf("base %s\n", type->name);
	return 0;
}

static int
base_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_base_init(struct Type *type) {
	int rv = 0;
	char *name = NULL;
	struct Generator *generator = type->generator;
	struct JwVal *definition = &type->definition;
	rv = jw_obj_get_str(generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}
	rv = type_name_copy(type, name);
	if (rv < 0) {
		goto out;
	}
	type->name[0] = toupper(name[0]);

	type->generate_type = base_generate_type;
	type->generate_field = base_generate_field;

out:
	free(name);
	return rv;
}
