#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
tuple_generate_type(struct Type *type) {
	printf("tuple %s\n", type->name);
	return 0;
}

static int
tuple_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_tuple_init(struct Type *type) {
	int rv = 0;
	type->generate_type = tuple_generate_type;
	type->generate_field = tuple_generate_field;
	rv = type_name_copy(type, "Tuple");
	if (rv < 0) {
		goto out;
	}
	rv = type_compound_init(type, "items", true);
out:
	return rv;
}
