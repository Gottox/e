#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int
structure_generate_field_iter(
		struct Type *type, const char *field_name, struct JwVal *field_type_id,
		void *data) {
	int rv = 0;
	struct Type *field_type =
			generator_lookup(type->generator, NULL, field_type_id, &rv);
	jw_debug(type->generator->jw, field_type_id);

	printf("name: %s %p\n", field_name, field_type);
	// field_type->generate_field(field_type, field_name);
	printf(";\n");
	return 0;
}

static int
structure_generate_type(struct Type *type) {
	printf("struct %s {\n", type->name);

	property_iterator(type, "properties", structure_generate_field_iter, NULL);
	return 0;
}

static int
structure_generate_field(struct Type *type, const char *field_name) {
	return 0;
}

int
type_structure_init(struct Type *type) {
	int rv = 0;
	char *name = NULL;
	struct JwVal *definition = &type->definition;
	type->generate_type = structure_generate_type;
	type->generate_field = structure_generate_field;

	rv = jw_obj_get_str(type->generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}
	type_name_copy(type, name);

	fputs("------\n", stderr);
	for (struct Type *prep = type->generator->preparation_list.head; prep;
		 prep = prep->next) {
		fprintf(stderr, "prep: %s, kind %s\t", prep->name, prep->kind);
		jw_debug(type->generator->jw, &prep->definition);
	}
	usleep(100000);

	//// We ignore failing compound inits
	//(void)type_compound_init(type, "extends", false);
	//(void)type_compound_init(type, "mixins", false);

	rv = type_compound_init(type, "properties", false);
	if (rv < 0) {
		goto out;
	}

out:
	free(name);
	return rv;
}
