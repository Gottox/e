#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
or_generate_type(struct Type *type) {
	printf("or %s\n", type->name);
	return 0;
}

static int
or_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_or_init(struct Type *type) {
	type->generate_type = or_generate_type;
	type->generate_field = or_generate_field;

	return type_compound_init(type, "Or", "items");
}
