#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
alias_generate_type(struct Type *type) {
	printf("alias %s\n", type->name);
	return 0;
}

static int
alias_generate_field(struct Type *type, const char *field_name) {
	return 0;
}


int
type_alias_init(struct Type *type) {
	int rv = 0;
	char *name = NULL;
	struct JwVal *definition = &type->definition;
	type->generate_type = alias_generate_type;
	type->generate_field = alias_generate_field;

	rv = jw_obj_get_str(
			type->generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}
	type_name_copy(type, name);

	//// We ignore failing compound inits
	//(void)type_compound_init(type, "", "extends");
	//(void)type_compound_init(type, "", "mixins");

	//rv = type_compound_init(type, "", "properties");
	if (rv < 0) {
		goto out;
	}

out:
	free(name);
	return rv;
}
