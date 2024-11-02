#include "metamodel_gen.h"
#include <jw_quickjs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
find_type_in(
		struct Generator *generator, const char *section, const char *needle) {
	int rv = 0;
	struct JwVal section_obj = {0};
	struct JwVal type = {0};
	int i = 0;
	int len = 0;
	char *name = NULL;

	rv = jw_obj_get(
			generator->jw, generator->meta_model, section, &section_obj);
	if (rv < 0) {
		goto out;
	}

	len = jw_arr_len(generator->jw, &section_obj);
	for (i = 0; i < len; i++) {
		rv = jw_arr_get(generator->jw, &section_obj, i, &type);
		if (rv < 0) {
			goto out;
		}

		rv = jw_obj_get_str(generator->jw, &type, "name", &name, NULL);
		if (rv < 0) {
			goto out;
		}

		if (strcmp(name, needle) == 0) {
			rv = i;
			goto out;
		}
		free(name);
		name = NULL;
	}

	rv = -1;
out:
	free(name);
	return rv;
}

int
type_reference_init(struct Type *type) {
	int rv = 0;
	struct Generator *generator = type->generator;
	struct JwVal *definition = type->definition;
	char *name = NULL;

	rv = jw_obj_get_str(generator->jw, definition, "name", &name, NULL);
	if (rv < 0) {
		goto out;
	}

	rv = type_name_copy(type, name);

out:
	free(name);
	return rv;
}
