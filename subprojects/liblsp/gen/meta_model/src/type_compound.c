#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
type_compound_iter(
		struct Type *type, const char *field_name, struct JwVal *field_type_json,
		void *userdata) {
	int rv = 0;
	bool *auto_name = userdata;
	struct Generator *generator = type->generator;

	struct Type *field_type = generator_lookup(generator, NULL, field_type_json, &rv);
	if (rv < 0) {
		goto out;
	}
	
	if (*auto_name) {
		type_name_append(type, field_type->name);
	}

out:
	return rv;
}

int
type_compound_init(struct Type *type, const char *field, bool auto_name) {
	return property_iterator(type, field, type_compound_iter, &auto_name);
}

int
type_named_compound_init(
		struct Type *type, const char *name, struct JwVal *compound) {
	return 0;
}
