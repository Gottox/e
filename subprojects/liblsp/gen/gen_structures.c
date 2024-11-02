#include "gen.h"
#include <jw.h>
#include <jw_quickjs.h>
#include <quickjs.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct StructureUserData {
	bool *processed;
	struct JwVal *structures;
	struct JwVal *type_aliases;
	struct JwVal *enums;
};

static int
gen_base_types(struct Jw *jw, struct JwVal *type) {
	char *name;
	size_t name_len;

	jw_obj_get_str(jw, type, "name", &name, &name_len);
	if (strcmp(name, "string") == 0) {
		fprintf(stdout, "char *");
	} else if (strcmp(name, "DocumentUri") == 0) {
		fprintf(stdout, "char *");
	} else if (strcmp(name, "number") == 0) {
		fprintf(stdout, "double ");
	} else if (strcmp(name, "boolean") == 0) {
		fprintf(stdout, "bool ");
	} else if (strcmp(name, "integer") == 0) {
		fprintf(stdout, "int ");
	} else if (strcmp(name, "null") == 0) {
		fprintf(stdout, "void ");
	} else if (strcmp(name, "any") == 0) {
		fprintf(stdout, "void ");
	} else {
		fprintf(stderr, "Unknown base type: %s\n", name);
	}
}
static int
gen_structure(struct Jw *jw, struct JwVal *request, int index, void *data);

static int
gen_ref_declaration(struct Jw *jw, struct JwVal *ref, int index, void *data) {
	struct StructureUserData *user_data = data;
	int rv = 0;
	char *kind;
	size_t kind_len;
	char *name;
	size_t name_len;

	rv = jw_obj_get_str(jw, ref, "kind", &kind, &kind_len);
	if (rv < 0) {
		goto out;
	}
	if (strcmp(kind, "reference") != 0) {
		rv = -1;
		goto out;
	}

	jw_obj_get_str(jw, ref, "name", &name, &name_len);
	if (rv < 0) {
		goto out;
	}

	size_t len = jw_arr_len(jw, user_data->structures);
	for (size_t i = 0; i < len; i++) {
		char *current_name;
		size_t current_name_len;
		struct JwVal current = {0};
		jw_arr_get(jw, user_data->structures, i, &current);
		jw_obj_get_str(jw, &current, "name", &current_name, &current_name_len);
		if (strcmp(name, current_name) == 0) {
			gen_structure(jw, &current, i, user_data);
			break;
		}
	}

out:
	return rv;
}

static int
gen_ref_declarations(
		struct Jw *jw, struct JwVal *fields, struct StructureUserData *data) {
	return jw_arr_foreach(jw, fields, gen_ref_declaration, data);
}

static int
gen_ref_field(struct Jw *jw, struct JwVal *ref, int index, void *data) {
	int rv = 0;
	char *kind;
	size_t kind_len;
	char *name;
	size_t name_len;
	char *field_name = NULL;

	rv = jw_obj_get_str(jw, ref, "kind", &kind, &kind_len);
	if (rv < 0) {
		goto out;
	}
	if (strcmp(kind, "reference") != 0) {
		rv = -1;
		goto out;
	}

	jw_obj_get_str(jw, ref, "name", &name, &name_len);
	if (rv < 0) {
		goto out;
	}
	field_name = snake_case(name);
	fprintf(stdout, "\tstruct " LSP_PREFIX_PASCAL "%s %s;\n", name, field_name);

out:
	free(field_name);
	return rv;
}

static int
gen_ref_fields(struct Jw *jw, struct JwVal *fields) {
	return jw_arr_foreach(jw, fields, gen_ref_field, NULL);
}

static int
gen_type(struct Jw *jw, struct JwVal *type, void *data, int depth) {
	char *kind;
	size_t kind_len;
	char *name;
	size_t name_len;
	char prefix[32] = {0};
	for (int i = 0; i < depth && i < 31; i++) {
		prefix[i] = '\t';
	}

	jw_obj_get_str(jw, type, "kind", &kind, &kind_len);
	jw_obj_get_str(jw, type, "name", &name, &name_len);

	if (strcmp(kind, "reference") == 0) {
		fprintf(stdout, "%s\tstruct " LSP_PREFIX_PASCAL "%s ", prefix, name);
	} else if (strcmp(kind, "base") == 0) {
		fputs("\t", stdout);
		fputs(prefix, stdout);
		gen_base_types(jw, type);
	} else if (strcmp(kind, "map") == 0) {
		struct JwVal value = {0};
		struct JwVal key = {0};
		jw_obj_get(jw, type, "key", &key);
		jw_obj_get(jw, type, "value", &value);
		fprintf(stdout,
				"%s\tstruct {\n"
				"%s\t\tsize_t count,\n",
				prefix, prefix);
		gen_type(jw, &key, data, depth + 1);
		fprintf(stdout, "*keys;\n");
		gen_type(jw, &value, data, depth + 1);
		fprintf(stdout, "*items;\n%s\t} ", prefix);
	} else if (strcmp(kind, "array") == 0) {
		struct JwVal element = {0};
		jw_obj_get(jw, type, "element", &element);
		fprintf(stdout,
				"%s\tstruct {\n"
				"%s\t\tsize_t count,\n",
				prefix, prefix);
		gen_type(jw, &element, data, depth + 1);
		fprintf(stdout, "*items;\n%s\t} ", prefix);
	}
	return 0;
}

static int
gen_property_field(struct Jw *jw, struct JwVal *field, int index, void *data) {
	int rv = 0;
	char *name;
	size_t name_len;
	char *field_name = NULL;
	struct JwVal type = {0};

	jw_obj_get_str(jw, field, "name", &name, &name_len);
	field_name = snake_case(name);

	jw_obj_get(jw, field, "type", &type);

	gen_type(jw, &type, data, 0);
	fprintf(stdout, "%s;\n", field_name);

out:
	free(field_name);
	return rv;
}

static int
gen_property_fields(struct Jw *jw, struct JwVal *fields) {
	return jw_arr_foreach(jw, fields, gen_property_field, NULL);
}

static int
gen_structure(struct Jw *jw, struct JwVal *request, int index, void *data) {
	struct StructureUserData *user_data = data;
	char *name;
	size_t name_len;
	struct JwVal extends = {0};
	struct JwVal mixins = {0};
	struct JwVal properties = {0};

	jw_obj_get_str(jw, request, "name", &name, &name_len);
	jw_obj_get(jw, request, "extends", &extends);
	jw_obj_get(jw, request, "mixins", &mixins);
	jw_obj_get(jw, request, "properties", &properties);

	gen_ref_declarations(jw, &extends, user_data);
	gen_ref_declarations(jw, &mixins, user_data);

	if (user_data->processed[index]) {
		return 0;
	}
	user_data->processed[index] = true;

	fprintf(stdout,
			"////////////////////////////\n"
			"// %s %i\n",
			name, index);

	fprintf(stdout, "\nstruct " LSP_PREFIX_PASCAL "%s {char info;\n", name);
	fputs("\t// Extends\n", stdout);
	gen_ref_fields(jw, &extends);
	fputs("\t// Mixins\n", stdout);
	gen_ref_fields(jw, &mixins);
	fputs("\t// Properties\n", stdout);
	gen_property_fields(jw, &properties);
	fputs("};\n\n", stdout);

	free(name);
	return 0;
}

int
gen_structures(
		struct Jw *jw, struct JwVal *structures, struct JwVal *type_aliases,
		struct JwVal *enums) {
	fputs("////////////////////////////\n"
		  "////////////////////////////\n"
		  "// Structures \n"
		  "\n",
		  stdout);
	ssize_t len = jw_arr_len(jw, structures);
	fprintf(stderr, "Structures: %li\n", len);
	bool *processed = calloc(len, sizeof(bool));
	struct StructureUserData data = {
			.processed = processed,
			.structures = structures,
			.type_aliases = type_aliases,
			.enums = enums,
	};
	jw_arr_foreach(jw, structures, gen_structure, &data);
	free(processed);
	return 0;
}
