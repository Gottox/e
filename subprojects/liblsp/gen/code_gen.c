#include "common.h"

/*
 * IR type kinds (inferred from structure)
 */

enum IrTypeKind {
	IR_TYPE_STRUCTURE,
	IR_TYPE_ENUMERATION,
	IR_TYPE_ARRAY,
	IR_TYPE_MAP,
	IR_TYPE_OR,
	IR_TYPE_TUPLE,
	IR_TYPE_UNKNOWN
};

enum IrPropTypeKind {
	IR_PROP_BASE,
	IR_PROP_REFERENCE,
	IR_PROP_STRING_LITERAL,
	IR_PROP_UNKNOWN_TYPE
};

/*
 * IR Reader context and functions
 */

struct IrReaderCtx {
	json_object *ir;
	json_object *types;
	const char *version;
};

static void
ir_reader_init(struct IrReaderCtx *ctx, json_object *ir) {
	ctx->ir = ir;
	ctx->types = json_get_array(ir, "types");
	ctx->version = json_get_string(ir, "version");
}

static enum IrTypeKind
ir_reader_type_kind(json_object *type_def) {
	if (json_has_key(type_def, "properties")) {
		return IR_TYPE_STRUCTURE;
	} else if (json_has_key(type_def, "values")) {
		return IR_TYPE_ENUMERATION;
	} else if (json_has_key(type_def, "element")) {
		return IR_TYPE_ARRAY;
	} else if (json_has_key(type_def, "elements")) {
		return IR_TYPE_TUPLE;
	} else if (json_has_key(type_def, "items")) {
		return IR_TYPE_OR;
	} else if (json_has_key(type_def, "value")) {
		return IR_TYPE_MAP;
	}
	return IR_TYPE_UNKNOWN;
}

static enum IrPropTypeKind
ir_reader_prop_type_kind(json_object *type) {
	const char *kind = json_get_string(type, "kind");
	if (kind == NULL) {
		return IR_PROP_UNKNOWN_TYPE;
	}
	if (strcmp(kind, "base") == 0) {
		return IR_PROP_BASE;
	} else if (strcmp(kind, "reference") == 0) {
		return IR_PROP_REFERENCE;
	} else if (strcmp(kind, "stringLiteral") == 0) {
		return IR_PROP_STRING_LITERAL;
	}
	return IR_PROP_UNKNOWN_TYPE;
}

static bool
ir_reader_enum_is_string(json_object *type_def) {
	json_object *type = json_get_object(type_def, "type");
	if (type == NULL) {
		return false;
	}
	const char *name = json_get_string(type, "name");
	return name && strcmp(name, "string") == 0;
}

/*
 * typeKind helpers - use typeKind from references instead of looking up type definitions
 */

static enum IrTypeKind
type_kind_from_string(const char *type_kind) {
	if (type_kind == NULL) return IR_TYPE_UNKNOWN;
	if (strcmp(type_kind, "structure") == 0) return IR_TYPE_STRUCTURE;
	if (strcmp(type_kind, "enumeration") == 0) return IR_TYPE_ENUMERATION;
	if (strcmp(type_kind, "stringEnumeration") == 0) return IR_TYPE_ENUMERATION;
	if (strcmp(type_kind, "array") == 0) return IR_TYPE_ARRAY;
	if (strcmp(type_kind, "map") == 0) return IR_TYPE_MAP;
	if (strcmp(type_kind, "or") == 0) return IR_TYPE_OR;
	if (strcmp(type_kind, "tuple") == 0) return IR_TYPE_TUPLE;
	return IR_TYPE_UNKNOWN;
}

static bool
type_kind_is_string_enum(const char *type_kind) {
	return type_kind && strcmp(type_kind, "stringEnumeration") == 0;
}

/*
 * Naming functions
 */

static char *
naming_c_type(json_object *prop_type) {
	enum IrPropTypeKind kind = ir_reader_prop_type_kind(prop_type);

	if (kind == IR_PROP_BASE) {
		const char *name = json_get_string(prop_type, "name");
		const char *ctype = base_to_c_type(name);
		if (ctype) {
			return strdup(ctype);
		}
		return strdup("json_object *");
	} else if (kind == IR_PROP_REFERENCE) {
		const char *name = json_get_string(prop_type, "name");
		const char *type_kind = json_get_string(prop_type, "typeKind");
		enum IrTypeKind tk = type_kind_from_string(type_kind);
		if (tk == IR_TYPE_ENUMERATION) {
			return to_enum_type(name);
		}
		return to_struct_type(name);
	} else if (kind == IR_PROP_STRING_LITERAL) {
		return strdup("const char *");
	}

	return strdup("json_object *");
}

static const char *
naming_json_suffix(json_object *prop_type) {
	enum IrPropTypeKind kind = ir_reader_prop_type_kind(prop_type);
	if (kind != IR_PROP_BASE) {
		return NULL;
	}
	const char *name = json_get_string(prop_type, "name");
	return base_type_json_suffix(name);
}

static const char *
naming_json_type(json_object *prop_type) {
	enum IrPropTypeKind kind = ir_reader_prop_type_kind(prop_type);
	if (kind != IR_PROP_BASE) {
		return NULL;
	}
	const char *name = json_get_string(prop_type, "name");
	return base_type_json_type(name);
}

/*
 * Code generation context and emit helpers
 */

struct GenCtx {
	struct IrReaderCtx *ir;
	FILE *hdr;
	FILE *src;
	json_object *generated;
};

enum EmitInitKind {
	EMIT_INIT_OBJECT,
	EMIT_INIT_ARRAY,
	EMIT_INIT_NULL,
};

enum EmitCollectionKind {
	EMIT_COLLECTION_ARRAY,
	EMIT_COLLECTION_OBJECT,
};

#define emit_header(ctx, ...) fprintf((ctx)->hdr, __VA_ARGS__)
#define emit_source(ctx, ...) fprintf((ctx)->src, __VA_ARGS__)

static void
gen_ctx_init(struct GenCtx *ctx, struct IrReaderCtx *ir, FILE *hdr, FILE *src) {
	ctx->ir = ir;
	ctx->hdr = hdr;
	ctx->src = src;
	ctx->generated = json_new_object();
}

static void
gen_ctx_cleanup(struct GenCtx *ctx) {
	if (ctx->generated) {
		json_object_put(ctx->generated);
		ctx->generated = NULL;
	}
}

static bool
gen_ctx_mark(struct GenCtx *ctx, const char *category, const char *name) {
	json_object *cat = json_get_object(ctx->generated, category);
	if (cat == NULL) {
		cat = json_new_object();
		json_add_object(ctx->generated, category, cat);
	}

	if (json_has_key(cat, name)) {
		return false;
	}
	json_add_bool(cat, name, true);
	return true;
}

static void
emit_struct_decl(struct GenCtx *ctx, const char *norm_name) {
	emit_header(ctx, "struct Lsp%s { json_object *json; };\n", norm_name);
}

static void
emit_lifecycle(struct GenCtx *ctx, const char *func_name, const char *struct_type,
               enum EmitInitKind init_kind) {
	emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__from_json(%s *obj, json_object *json);\n",
	            func_name, struct_type);
	emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__init(%s *obj);\n", func_name, struct_type);
	emit_header(ctx, "void lsp_%s__cleanup(%s *obj);\n", func_name, struct_type);

	emit_source(ctx, "int lsp_%s__from_json(%s *obj, json_object *json) { "
	            "obj->json = json_object_get(json); return LSP_OK; }\n",
	            func_name, struct_type);

	emit_source(ctx, "int lsp_%s__init(%s *obj) { ", func_name, struct_type);
	switch (init_kind) {
	case EMIT_INIT_OBJECT:
		emit_source(ctx, "obj->json = json_object_new_object(); ");
		emit_source(ctx, "if (obj->json == NULL) { return LSP_ERR_OOM; } ");
		emit_source(ctx, "return LSP_OK; }\n");
		break;
	case EMIT_INIT_ARRAY:
		emit_source(ctx, "obj->json = json_object_new_array(); ");
		emit_source(ctx, "if (obj->json == NULL) { return LSP_ERR_OOM; } ");
		emit_source(ctx, "return LSP_OK; }\n");
		break;
	case EMIT_INIT_NULL:
		emit_source(ctx, "obj->json = NULL; return LSP_OK; }\n");
		break;
	}

	emit_source(ctx, "void lsp_%s__cleanup(%s *obj) { "
	            "if (obj->json != NULL) { json_object_put(obj->json); obj->json = NULL; } }\n",
	            func_name, struct_type);
}

static void
emit_len_func(struct GenCtx *ctx, const char *func_prefix,
              const char *struct_type, enum EmitCollectionKind kind) {
	emit_header(ctx, "size_t lsp_%s__len(const %s *obj);\n", func_prefix, struct_type);
	emit_source(ctx, "size_t lsp_%s__len(const %s *obj) { ", func_prefix, struct_type);
	if (kind == EMIT_COLLECTION_ARRAY) {
		emit_source(ctx, "return obj->json ? json_object_array_length(obj->json) : 0; }\n");
	} else {
		emit_source(ctx, "return obj->json ? json_object_object_length(obj->json) : 0; }\n");
	}
}

static void
emit_header_prologue(struct GenCtx *ctx) {
	emit_header(ctx, "/* Generated by gen - do not edit */\n");
	emit_header(ctx, "#ifndef LSP_MODEL_H\n");
	emit_header(ctx, "#define LSP_MODEL_H\n\n");
	emit_header(ctx, "#include <json.h>\n");
	emit_header(ctx, "#include <stdbool.h>\n");
	emit_header(ctx, "#include <stddef.h>\n");
	emit_header(ctx, "#include <stdint.h>\n\n");

	emit_header(ctx, "#ifndef LSP_NO_UNUSED\n");
	emit_header(ctx, "#define LSP_NO_UNUSED __attribute__((warn_unused_result))\n");
	emit_header(ctx, "#endif\n\n");

	emit_header(ctx, "/* Error codes */\n");
	emit_header(ctx, "#define LSP_OK 0\n");
	emit_header(ctx, "#define LSP_ERR_MISSING_FIELD -1\n");
	emit_header(ctx, "#define LSP_ERR_INVALID_TYPE -2\n");
	emit_header(ctx, "#define LSP_ERR_INDEX_OUT_OF_BOUNDS -3\n");
	emit_header(ctx, "#define LSP_ERR_OOM -4\n\n");
}

static void
emit_header_epilogue(struct GenCtx *ctx) {
	emit_header(ctx, "#endif /* LSP_MODEL_H */\n");
}

static void
emit_source_prologue(struct GenCtx *ctx) {
	emit_source(ctx, "/* Generated by gen - do not edit */\n");
	emit_source(ctx, "#include \"model.h\"\n");
	emit_source(ctx, "#include <string.h>\n\n");
}

/*
 * Structure generation
 */

/* Collect string literal properties from a type definition */
static json_object *
collect_string_literal_props(json_object *type_def) {
	json_object *properties = json_get_array(type_def, "properties");
	size_t prop_count = json_array_len(properties);
	json_object *literals = NULL;

	for (size_t i = 0; i < prop_count; i++) {
		json_object *prop = json_array_get(properties, i);
		json_object *prop_type = json_get_object(prop, "type");
		enum IrPropTypeKind pk = ir_reader_prop_type_kind(prop_type);
		if (pk == IR_PROP_STRING_LITERAL) {
			if (literals == NULL) {
				literals = json_new_array();
			}
			json_array_add(literals, json_object_get(prop));
		}
	}

	return literals;
}

static void
gen_getter(struct GenCtx *ctx, const char *struct_name, const char *func_prefix,
           json_object *prop) {
	const char *prop_name = json_get_string(prop, "name");
	json_object *prop_type = json_get_object(prop, "type");
	char *prop_snake = str_to_snake_case(prop_name);

	enum IrPropTypeKind pk = ir_reader_prop_type_kind(prop_type);

	if (pk == IR_PROP_STRING_LITERAL) {
		const char *literal_value = json_get_string(prop_type, "value");

		emit_header(ctx, "const char * lsp_%s__%s(const struct Lsp%s *obj);\n",
		            func_prefix, prop_snake, struct_name);

		emit_source(ctx, "const char * lsp_%s__%s(const struct Lsp%s *obj) { ",
		            func_prefix, prop_snake, struct_name);
		emit_source(ctx, "json_object *field = json_object_object_get(obj->json, \"%s\"); ",
		            prop_name);
		emit_source(ctx, "return field ? json_object_get_string(field) : \"%s\"; }\n",
		            literal_value ? literal_value : "");

		free(prop_snake);
		return;
	}

	if (pk == IR_PROP_REFERENCE) {
		const char *ref_name = json_get_string(prop_type, "name");
		const char *type_kind = json_get_string(prop_type, "typeKind");
		enum IrTypeKind tk = type_kind_from_string(type_kind);

		if (tk == IR_TYPE_ENUMERATION) {
			// Enum reference - use from_string or int conversion
			char *ref_enum = to_enum_type(ref_name);
			char *ref_func = to_func_name(ref_name);
			bool is_string_enum = type_kind_is_string_enum(type_kind);

			if (is_string_enum) {
				// String enum - need to use output parameter since from_string can fail
				emit_header(ctx, "int lsp_%s__%s(const struct Lsp%s *obj, %s *out);\n",
				            func_prefix, prop_snake, struct_name, ref_enum);

				emit_source(ctx, "int lsp_%s__%s(const struct Lsp%s *obj, %s *out) { ",
				            func_prefix, prop_snake, struct_name, ref_enum);
				emit_source(ctx, "json_object *field = json_object_object_get(obj->json, \"%s\"); ",
				            prop_name);
				emit_source(ctx, "return lsp_%s__from_string(json_object_get_string(field), out); }\n",
				            ref_func);
			} else {
				// Integer enum - can return directly
				emit_header(ctx, "%s lsp_%s__%s(const struct Lsp%s *obj);\n",
				            ref_enum, func_prefix, prop_snake, struct_name);

				emit_source(ctx, "%s lsp_%s__%s(const struct Lsp%s *obj) { ",
				            ref_enum, func_prefix, prop_snake, struct_name);
				emit_source(ctx, "json_object *field = json_object_object_get(obj->json, \"%s\"); ",
				            prop_name);
				emit_source(ctx, "return (%s)json_object_get_int(field); }\n", ref_enum);
			}

			free(ref_func);
			free(ref_enum);
		} else {
			// Struct-like reference (structure, array, map, or, alias, unknown)
			char *ref_struct = to_struct_type(ref_name);
			char *ref_func = to_func_name(ref_name);

			emit_header(ctx, "int lsp_%s__%s(const struct Lsp%s *obj, %s *child);\n",
			            func_prefix, prop_snake, struct_name, ref_struct);

			emit_source(ctx, "int lsp_%s__%s(const struct Lsp%s *obj, %s *child) { ",
			            func_prefix, prop_snake, struct_name, ref_struct);
			emit_source(ctx, "json_object *field = json_object_object_get(obj->json, \"%s\"); ",
			            prop_name);
			emit_source(ctx, "return lsp_%s__from_json(child, field); }\n", ref_func);

			free(ref_func);
			free(ref_struct);
		}
	} else if (pk == IR_PROP_BASE) {
		// Base type property
		char *ctype = naming_c_type(prop_type);
		const char *json_suffix = naming_json_suffix(prop_type);

		emit_header(ctx, "%s lsp_%s__%s(const struct Lsp%s *obj);\n",
		            ctype, func_prefix, prop_snake, struct_name);

		emit_source(ctx, "%s lsp_%s__%s(const struct Lsp%s *obj) { ",
		            ctype, func_prefix, prop_snake, struct_name);
		emit_source(ctx, "json_object *field = json_object_object_get(obj->json, \"%s\"); ",
		            prop_name);

		if (json_suffix) {
			emit_source(ctx, "return json_object_get_%s(field); }\n", json_suffix);
		} else {
			emit_source(ctx, "return field; }\n");
		}

		free(ctype);
	}

	free(prop_snake);
}

static void
gen_setter(struct GenCtx *ctx, const char *struct_name, const char *func_prefix,
           json_object *prop) {
	const char *prop_name = json_get_string(prop, "name");
	json_object *prop_type = json_get_object(prop, "type");
	char *prop_snake = str_to_snake_case(prop_name);

	enum IrPropTypeKind pk = ir_reader_prop_type_kind(prop_type);

	if (pk == IR_PROP_STRING_LITERAL) {
		free(prop_snake);
		return;
	}

	if (pk == IR_PROP_REFERENCE) {
		const char *ref_name = json_get_string(prop_type, "name");
		const char *type_kind = json_get_string(prop_type, "typeKind");
		enum IrTypeKind tk = type_kind_from_string(type_kind);

		if (tk == IR_TYPE_ENUMERATION) {
			// Enum reference - use to_string or int conversion
			char *ref_enum = to_enum_type(ref_name);
			char *ref_func = to_func_name(ref_name);
			bool is_string_enum = type_kind_is_string_enum(type_kind);

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__set_%s(struct Lsp%s *obj, %s value);\n",
			            func_prefix, prop_snake, struct_name, ref_enum);

			emit_source(ctx, "int lsp_%s__set_%s(struct Lsp%s *obj, %s value) { ",
			            func_prefix, prop_snake, struct_name, ref_enum);

			if (is_string_enum) {
				emit_source(ctx, "json_object *val = json_object_new_string(lsp_%s__to_string(value)); ",
				            ref_func);
				emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "if (json_object_object_add(obj->json, \"%s\", val) < 0) { return LSP_ERR_OOM; } ",
				            prop_name);
				emit_source(ctx, "return LSP_OK; }\n");
			} else {
				emit_source(ctx, "json_object *val = json_object_new_int(value); ");
				emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "if (json_object_object_add(obj->json, \"%s\", val) < 0) { return LSP_ERR_OOM; } ",
				            prop_name);
				emit_source(ctx, "return LSP_OK; }\n");
			}

			free(ref_func);
			free(ref_enum);
		} else {
			// Struct-like reference (structure, array, map, or, alias, unknown)
			char *ref_struct = to_struct_type(ref_name);

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__set_%s(struct Lsp%s *obj, %s *value);\n",
			            func_prefix, prop_snake, struct_name, ref_struct);

			emit_source(ctx, "int lsp_%s__set_%s(struct Lsp%s *obj, %s *value) { ",
			            func_prefix, prop_snake, struct_name, ref_struct);
			emit_source(ctx, "if (json_object_object_add(obj->json, \"%s\", json_object_get(value->json)) < 0) { return LSP_ERR_OOM; } ",
			            prop_name);
			emit_source(ctx, "return LSP_OK; }\n");

			free(ref_struct);
		}
	} else if (pk == IR_PROP_BASE) {
		// Base type property
		char *ctype = naming_c_type(prop_type);
		const char *json_suffix = naming_json_suffix(prop_type);

		emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__set_%s(struct Lsp%s *obj, %s value);\n",
		            func_prefix, prop_snake, struct_name, ctype);

		emit_source(ctx, "int lsp_%s__set_%s(struct Lsp%s *obj, %s value) { ",
		            func_prefix, prop_snake, struct_name, ctype);

		if (json_suffix) {
			emit_source(ctx, "json_object *val = json_object_new_%s(value); ", json_suffix);
			emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "if (json_object_object_add(obj->json, \"%s\", val) < 0) { return LSP_ERR_OOM; } ",
			            prop_name);
			emit_source(ctx, "return LSP_OK; }\n");
		} else {
			emit_source(ctx, "if (json_object_object_add(obj->json, \"%s\", json_object_get(value)) < 0) { return LSP_ERR_OOM; } ",
			            prop_name);
			emit_source(ctx, "return LSP_OK; }\n");
		}

		free(ctype);
	}

	free(prop_snake);
}

static void
emit_string_literal_lifecycle(struct GenCtx *ctx, const char *func_prefix,
                              const char *struct_type, json_object *literals) {
	size_t n = json_array_len(literals);

	/* from_json: validate each literal field */
	emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__from_json(%s *obj, json_object *json);\n",
	            func_prefix, struct_type);
	emit_source(ctx, "int lsp_%s__from_json(%s *obj, json_object *json) { ",
	            func_prefix, struct_type);
	for (size_t i = 0; i < n; i++) {
		json_object *prop = json_array_get(literals, i);
		const char *prop_name = json_get_string(prop, "name");
		json_object *prop_type = json_get_object(prop, "type");
		const char *literal_value = json_get_string(prop_type, "value");
		emit_source(ctx, "{ json_object *f = json_object_object_get(json, \"%s\"); ",
		            prop_name);
		emit_source(ctx, "if (f != NULL && strcmp(json_object_get_string(f), \"%s\") != 0) "
		            "return LSP_ERR_INVALID_TYPE; } ",
		            literal_value);
	}
	emit_source(ctx, "obj->json = json_object_get(json); return LSP_OK; }\n");

	/* init: auto-populate literal fields */
	emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__init(%s *obj);\n",
	            func_prefix, struct_type);
	emit_source(ctx, "int lsp_%s__init(%s *obj) { ", func_prefix, struct_type);
	emit_source(ctx, "obj->json = json_object_new_object(); ");
	emit_source(ctx, "if (obj->json == NULL) { return LSP_ERR_OOM; } ");
	for (size_t i = 0; i < n; i++) {
		json_object *prop = json_array_get(literals, i);
		const char *prop_name = json_get_string(prop, "name");
		json_object *prop_type = json_get_object(prop, "type");
		const char *literal_value = json_get_string(prop_type, "value");
		emit_source(ctx, "json_object_object_add(obj->json, \"%s\", "
		            "json_object_new_string(\"%s\")); ",
		            prop_name, literal_value);
	}
	emit_source(ctx, "return LSP_OK; }\n");

	/* cleanup: standard */
	emit_header(ctx, "void lsp_%s__cleanup(%s *obj);\n", func_prefix, struct_type);
	emit_source(ctx, "void lsp_%s__cleanup(%s *obj) { "
	            "if (obj->json != NULL) { json_object_put(obj->json); obj->json = NULL; } }\n",
	            func_prefix, struct_type);
}

static void
gen_structure(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "struct", name)) {
		return;
	}

	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *struct_type = to_struct_type(name);

	emit_struct_decl(ctx, norm_name);

	json_object *literals = collect_string_literal_props(type_def);
	if (literals != NULL) {
		emit_string_literal_lifecycle(ctx, func_prefix, struct_type, literals);
		json_object_put(literals);
	} else {
		emit_lifecycle(ctx, func_prefix, struct_type, EMIT_INIT_OBJECT);
	}

	json_object *properties = json_get_array(type_def, "properties");
	size_t prop_count = json_array_len(properties);
	for (size_t i = 0; i < prop_count; i++) {
		json_object *prop = json_array_get(properties, i);
		gen_getter(ctx, norm_name, func_prefix, prop);
		gen_setter(ctx, norm_name, func_prefix, prop);
	}

	free(struct_type);
	free(func_prefix);
}

/*
 * Array type generation
 */

static void
gen_array_type(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "array", name)) {
		return;
	}

	json_object *element = json_get_object(type_def, "element");
	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *struct_type = to_struct_type(name);

	emit_struct_decl(ctx, norm_name);
	emit_lifecycle(ctx, func_prefix, struct_type, EMIT_INIT_ARRAY);
	emit_len_func(ctx, func_prefix, struct_type, EMIT_COLLECTION_ARRAY);

	enum IrPropTypeKind epk = ir_reader_prop_type_kind(element);

	if (epk == IR_PROP_REFERENCE) {
		const char *elem_name = json_get_string(element, "name");
		const char *type_kind = json_get_string(element, "typeKind");
		enum IrTypeKind tk = type_kind_from_string(type_kind);

		if (tk == IR_TYPE_ENUMERATION) {
			// Enum element type
			char *elem_enum = to_enum_type(elem_name);
			char *elem_func = to_func_name(elem_name);
			bool is_string_enum = type_kind_is_string_enum(type_kind);

			emit_header(ctx, "int lsp_%s__get(const %s *obj, %s *value, size_t index);\n",
			            func_prefix, struct_type, elem_enum);
			emit_source(ctx, "int lsp_%s__get(const %s *obj, %s *value, size_t index) { ",
			            func_prefix, struct_type, elem_enum);
			emit_source(ctx, "if (obj->json == NULL || index >= json_object_array_length(obj->json)) "
			            "return LSP_ERR_INDEX_OUT_OF_BOUNDS; ");
			if (is_string_enum) {
				emit_source(ctx, "return lsp_%s__from_string(json_object_get_string(json_object_array_get_idx(obj->json, index)), value); }\n",
				            elem_func);
			} else {
				emit_source(ctx, "*value = (%s)json_object_get_int(json_object_array_get_idx(obj->json, index)); ",
				            elem_enum);
				emit_source(ctx, "return LSP_OK; }\n");
			}

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__add(%s *obj, %s value);\n",
			            func_prefix, struct_type, elem_enum);
			emit_source(ctx, "int lsp_%s__add(%s *obj, %s value) { ",
			            func_prefix, struct_type, elem_enum);
			if (is_string_enum) {
				emit_source(ctx, "json_object *val = json_object_new_string(lsp_%s__to_string(value)); ",
				            elem_func);
				emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "if (json_object_array_add(obj->json, val) < 0) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "return LSP_OK; }\n");
			} else {
				emit_source(ctx, "json_object *val = json_object_new_int(value); ");
				emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "if (json_object_array_add(obj->json, val) < 0) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "return LSP_OK; }\n");
			}

			free(elem_func);
			free(elem_enum);
		} else {
			// Struct-like element type (structure, array, map, or, alias, unknown)
			char *elem_struct = to_struct_type(elem_name);
			char *elem_func = to_func_name(elem_name);

			emit_header(ctx, "int lsp_%s__get(const %s *obj, %s *child, size_t index);\n",
			            func_prefix, struct_type, elem_struct);
			emit_source(ctx, "int lsp_%s__get(const %s *obj, %s *child, size_t index) { ",
			            func_prefix, struct_type, elem_struct);
			emit_source(ctx, "if (obj->json == NULL || index >= json_object_array_length(obj->json)) "
			            "return LSP_ERR_INDEX_OUT_OF_BOUNDS; ");
			emit_source(ctx, "return lsp_%s__from_json(child, json_object_array_get_idx(obj->json, index)); }\n",
			            elem_func);

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__add(%s *obj, %s *elem);\n",
			            func_prefix, struct_type, elem_struct);
			emit_source(ctx, "int lsp_%s__add(%s *obj, %s *elem) { ",
			            func_prefix, struct_type, elem_struct);
			emit_source(ctx, "if (json_object_array_add(obj->json, json_object_get(elem->json)) < 0) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "return LSP_OK; }\n");

			free(elem_func);
			free(elem_struct);
		}
	}

	free(struct_type);
	free(func_prefix);
}

/*
 * Map type generation
 */

static void
gen_map_type(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "map", name)) {
		return;
	}

	json_object *value = json_get_object(type_def, "value");
	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *struct_type = to_struct_type(name);

	emit_struct_decl(ctx, norm_name);
	emit_lifecycle(ctx, func_prefix, struct_type, EMIT_INIT_OBJECT);
	emit_len_func(ctx, func_prefix, struct_type, EMIT_COLLECTION_OBJECT);

	enum IrPropTypeKind vpk = ir_reader_prop_type_kind(value);

	if (vpk == IR_PROP_REFERENCE) {
		const char *val_name = json_get_string(value, "name");
		const char *type_kind = json_get_string(value, "typeKind");
		enum IrTypeKind tk = type_kind_from_string(type_kind);

		if (tk != IR_TYPE_ENUMERATION) {
			// Struct-like value type (structure, array, map, or, alias, unknown)
			char *val_struct = to_struct_type(val_name);
			char *val_func = to_func_name(val_name);

			emit_header(ctx, "int lsp_%s__get(const %s *obj, const char *key, %s *result);\n",
			            func_prefix, struct_type, val_struct);
			emit_source(ctx, "int lsp_%s__get(const %s *obj, const char *key, %s *result) { ",
			            func_prefix, struct_type, val_struct);
			emit_source(ctx, "json_object *val = json_object_object_get(obj->json, key); ");
			emit_source(ctx, "if (val == NULL) return LSP_ERR_MISSING_FIELD; ");
			emit_source(ctx, "return lsp_%s__from_json(result, val); }\n", val_func);

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__put(%s *obj, const char *key, %s *value);\n",
			            func_prefix, struct_type, val_struct);
			emit_source(ctx, "int lsp_%s__put(%s *obj, const char *key, %s *value) { ",
			            func_prefix, struct_type, val_struct);
			emit_source(ctx, "if (json_object_object_add(obj->json, key, json_object_get(value->json)) < 0) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "return LSP_OK; }\n");

			free(val_func);
			free(val_struct);
		}
		// Note: enum values in maps are not currently supported
	} else {
		// Base type value
		char *ctype = naming_c_type(value);
		const char *json_suffix = naming_json_suffix(value);

		emit_header(ctx, "int lsp_%s__get(const %s *obj, const char *key, %s *result);\n",
		            func_prefix, struct_type, ctype);
		emit_source(ctx, "int lsp_%s__get(const %s *obj, const char *key, %s *result) { ",
		            func_prefix, struct_type, ctype);
		emit_source(ctx, "json_object *val = json_object_object_get(obj->json, key); ");
		emit_source(ctx, "if (val == NULL) return LSP_ERR_MISSING_FIELD; ");
		if (json_suffix) {
			emit_source(ctx, "*result = json_object_get_%s(val); ", json_suffix);
		} else {
			emit_source(ctx, "*result = val; ");
		}
		emit_source(ctx, "return LSP_OK; }\n");

		emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__put(%s *obj, const char *key, %s value);\n",
		            func_prefix, struct_type, ctype);
		emit_source(ctx, "int lsp_%s__put(%s *obj, const char *key, %s value) { ",
		            func_prefix, struct_type, ctype);
		if (json_suffix) {
			emit_source(ctx, "json_object *val = json_object_new_%s(value); ", json_suffix);
			emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "if (json_object_object_add(obj->json, key, val) < 0) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "return LSP_OK; }\n");
		} else {
			emit_source(ctx, "if (json_object_object_add(obj->json, key, json_object_get(value)) < 0) { return LSP_ERR_OOM; } ");
			emit_source(ctx, "return LSP_OK; }\n");
		}

		free(ctype);
	}

	free(struct_type);
	free(func_prefix);
}

/*
 * Tuple type generation
 */

static void
gen_tuple_type(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "tuple", name)) {
		return;
	}

	json_object *elements = json_get_array(type_def, "elements");
	size_t elem_count = json_array_len(elements);
	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *struct_type = to_struct_type(name);

	emit_struct_decl(ctx, norm_name);
	emit_lifecycle(ctx, func_prefix, struct_type, EMIT_INIT_ARRAY);
	emit_len_func(ctx, func_prefix, struct_type, EMIT_COLLECTION_ARRAY);

	for (size_t i = 0; i < elem_count; i++) {
		json_object *elem = json_array_get(elements, i);
		enum IrPropTypeKind epk = ir_reader_prop_type_kind(elem);

		if (epk == IR_PROP_BASE) {
			char *ctype = naming_c_type(elem);
			const char *json_suffix = naming_json_suffix(elem);

			emit_header(ctx, "%s lsp_%s__get_%zu(const %s *obj);\n",
			            ctype, func_prefix, i, struct_type);
			emit_source(ctx, "%s lsp_%s__get_%zu(const %s *obj) { ",
			            ctype, func_prefix, i, struct_type);
			if (json_suffix) {
				emit_source(ctx, "return json_object_get_%s(json_object_array_get_idx(obj->json, %zu)); }\n",
				            json_suffix, i);
			} else {
				emit_source(ctx, "return json_object_array_get_idx(obj->json, %zu); }\n", i);
			}

			emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__set_%zu(%s *obj, %s value);\n",
			            func_prefix, i, struct_type, ctype);
			emit_source(ctx, "int lsp_%s__set_%zu(%s *obj, %s value) { ",
			            func_prefix, i, struct_type, ctype);
			if (json_suffix) {
				emit_source(ctx, "json_object *val = json_object_new_%s(value); ", json_suffix);
				emit_source(ctx, "if (val == NULL) { return LSP_ERR_OOM; } ");
				emit_source(ctx, "if (json_object_array_put_idx(obj->json, %zu, val) < 0) { return LSP_ERR_OOM; } ",
				            i);
				emit_source(ctx, "return LSP_OK; }\n");
			} else {
				emit_source(ctx, "if (json_object_array_put_idx(obj->json, %zu, json_object_get(value)) < 0) { return LSP_ERR_OOM; } ",
				            i);
				emit_source(ctx, "return LSP_OK; }\n");
			}

			free(ctype);
		} else if (epk == IR_PROP_REFERENCE) {
			const char *ref_name = json_get_string(elem, "name");
			const char *type_kind = json_get_string(elem, "typeKind");
			enum IrTypeKind tk = type_kind_from_string(type_kind);

			if (tk == IR_TYPE_ENUMERATION) {
				char *ref_enum = to_enum_type(ref_name);
				char *ref_func = to_func_name(ref_name);
				bool is_string_enum = type_kind_is_string_enum(type_kind);

				emit_header(ctx, "int lsp_%s__get_%zu(const %s *obj, %s *out);\n",
				            func_prefix, i, struct_type, ref_enum);
				emit_source(ctx, "int lsp_%s__get_%zu(const %s *obj, %s *out) { ",
				            func_prefix, i, struct_type, ref_enum);
				if (is_string_enum) {
					emit_source(ctx, "return lsp_%s__from_string(json_object_get_string(json_object_array_get_idx(obj->json, %zu)), out); }\n",
					            ref_func, i);
				} else {
					emit_source(ctx, "*out = (%s)json_object_get_int(json_object_array_get_idx(obj->json, %zu)); ",
					            ref_enum, i);
					emit_source(ctx, "return LSP_OK; }\n");
				}

				free(ref_func);
				free(ref_enum);
			} else {
				char *ref_struct = to_struct_type(ref_name);
				char *ref_func = to_func_name(ref_name);

				emit_header(ctx, "int lsp_%s__get_%zu(const %s *obj, %s *child);\n",
				            func_prefix, i, struct_type, ref_struct);
				emit_source(ctx, "int lsp_%s__get_%zu(const %s *obj, %s *child) { ",
				            func_prefix, i, struct_type, ref_struct);
				emit_source(ctx, "return lsp_%s__from_json(child, json_object_array_get_idx(obj->json, %zu)); }\n",
				            ref_func, i);

				emit_header(ctx, "LSP_NO_UNUSED int lsp_%s__set_%zu(%s *obj, %s *value);\n",
				            func_prefix, i, struct_type, ref_struct);
				emit_source(ctx, "int lsp_%s__set_%zu(%s *obj, %s *value) { ",
				            func_prefix, i, struct_type, ref_struct);
				emit_source(ctx, "if (json_object_array_put_idx(obj->json, %zu, json_object_get(value->json)) < 0) { return LSP_ERR_OOM; } ",
				            i);
				emit_source(ctx, "return LSP_OK; }\n");

				free(ref_func);
				free(ref_struct);
			}
		}
	}

	free(struct_type);
	free(func_prefix);
}

/*
 * Or-type variant helpers (used by both or-types and aliases)
 */

static char *
gen_variant_enum_name(const char *const_prefix, json_object *item, size_t index) {
	enum IrPropTypeKind pk = ir_reader_prop_type_kind(item);

	if (pk == IR_PROP_BASE) {
		const char *name = json_get_string(item, "name");
		char *name_upper = str_to_upper_snake(name);
		char *result = NULL;
		asprintf(&result, "LSP_%s_KIND_%s", const_prefix, name_upper);
		free(name_upper);
		return result;
	} else if (pk == IR_PROP_REFERENCE) {
		const char *name = json_get_string(item, "name");
		char *name_upper = str_to_upper_snake(name);
		char *result = NULL;
		asprintf(&result, "LSP_%s_KIND_%s", const_prefix, name_upper);
		free(name_upper);
		return result;
	}

	char *result = NULL;
	asprintf(&result, "LSP_%s_KIND_OTHER%zu", const_prefix, index);
	return result;
}

static char *
gen_variant_func_suffix(json_object *item) {
	enum IrPropTypeKind pk = ir_reader_prop_type_kind(item);

	if (pk == IR_PROP_BASE) {
		const char *name = json_get_string(item, "name");
		return str_to_snake_case(name);
	} else if (pk == IR_PROP_REFERENCE) {
		const char *name = json_get_string(item, "name");
		return str_to_snake_case(name);
	}

	return strdup("other");
}

/*
 * Or-type generation
 */

static void
gen_or_type(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "or", name)) {
		return;
	}

	json_object *items = json_get_array(type_def, "items");
	size_t item_count = json_array_len(items);

	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *const_prefix = to_constant_name(name);
	char *struct_type = to_struct_type(name);

	emit_struct_decl(ctx, norm_name);

	emit_header(ctx, "enum Lsp%sVariant { LSP_%s_KIND_UNKNOWN", norm_name, const_prefix);
	for (size_t i = 0; i < item_count; i++) {
		json_object *item = json_array_get(items, i);
		char *enum_name = gen_variant_enum_name(const_prefix, item, i);
		emit_header(ctx, ", %s", enum_name);
		free(enum_name);
	}
	emit_header(ctx, " };\n");

	emit_lifecycle(ctx, func_prefix, struct_type, EMIT_INIT_NULL);

	emit_header(ctx, "enum Lsp%sVariant lsp_%s__kind(const %s *obj);\n",
	            norm_name, func_prefix, struct_type);

	emit_source(ctx, "enum Lsp%sVariant lsp_%s__kind(const %s *obj) { ",
	            norm_name, func_prefix, struct_type);
	emit_source(ctx, "json_object *json = obj->json; ");
	emit_source(ctx, "if (json == NULL) return LSP_%s_KIND_UNKNOWN; ", const_prefix);

	for (size_t i = 0; i < item_count; i++) {
		json_object *item = json_array_get(items, i);
		enum IrPropTypeKind pk = ir_reader_prop_type_kind(item);
		char *enum_name = gen_variant_enum_name(const_prefix, item, i);

		if (pk == IR_PROP_BASE) {
			const char *json_type = naming_json_type(item);
			if (json_type) {
				emit_source(ctx, "if (json_object_is_type(json, %s)) return %s; ",
				            json_type, enum_name);
			}
		} else if (pk == IR_PROP_REFERENCE) {
			// Use typeKind from the reference
			const char *type_kind = json_get_string(item, "typeKind");
			enum IrTypeKind rk = type_kind_from_string(type_kind);
			if (rk == IR_TYPE_ARRAY || rk == IR_TYPE_TUPLE) {
				emit_source(ctx, "if (json_object_is_type(json, json_type_array)) return %s; ",
				            enum_name);
			} else {
				emit_source(ctx, "if (json_object_is_type(json, json_type_object)) return %s; ",
				            enum_name);
			}
		}

		free(enum_name);
	}

	emit_source(ctx, "return LSP_%s_KIND_UNKNOWN; }\n", const_prefix);

	for (size_t i = 0; i < item_count; i++) {
		json_object *item = json_array_get(items, i);
		enum IrPropTypeKind pk = ir_reader_prop_type_kind(item);
		char *enum_name = gen_variant_enum_name(const_prefix, item, i);
		char *func_suffix = gen_variant_func_suffix(item);

		emit_header(ctx, "bool lsp_%s__is_%s(const %s *obj);\n",
		            func_prefix, func_suffix, struct_type);
		emit_source(ctx, "bool lsp_%s__is_%s(const %s *obj) { ",
		            func_prefix, func_suffix, struct_type);
		emit_source(ctx, "return lsp_%s__kind(obj) == %s; }\n", func_prefix, enum_name);

		if (pk == IR_PROP_BASE) {
			const char *base_name = json_get_string(item, "name");
			if (strcmp(base_name, "null") != 0) {
				char *ctype = naming_c_type(item);
				const char *json_suffix = naming_json_suffix(item);

				emit_header(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out);\n",
				            func_prefix, func_suffix, struct_type, ctype);
				emit_source(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out) { ",
				            func_prefix, func_suffix, struct_type, ctype);
				emit_source(ctx, "if (!lsp_%s__is_%s(obj)) return LSP_ERR_MISSING_FIELD; ",
				            func_prefix, func_suffix);
				if (json_suffix) {
					emit_source(ctx, "*out = json_object_get_%s(obj->json); ", json_suffix);
				} else {
					emit_source(ctx, "*out = obj->json; ");
				}
				emit_source(ctx, "return LSP_OK; }\n");

				free(ctype);
			}
		} else if (pk == IR_PROP_REFERENCE) {
			const char *ref_name = json_get_string(item, "name");
			const char *type_kind = json_get_string(item, "typeKind");
			enum IrTypeKind ref_kind = type_kind_from_string(type_kind);

			if (ref_kind == IR_TYPE_ENUMERATION) {
				// Enum reference - use enum type
				char *ref_enum = to_enum_type(ref_name);
				bool is_string_enum = type_kind_is_string_enum(type_kind);
				const char *json_suffix = is_string_enum ? NULL : "int";

				emit_header(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out);\n",
				            func_prefix, func_suffix, struct_type, ref_enum);
				emit_source(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out) { ",
				            func_prefix, func_suffix, struct_type, ref_enum);
				emit_source(ctx, "if (!lsp_%s__is_%s(obj)) return LSP_ERR_MISSING_FIELD; ",
				            func_prefix, func_suffix);
				if (json_suffix) {
					emit_source(ctx, "*out = (%s)json_object_get_%s(obj->json); ", ref_enum, json_suffix);
				} else {
					emit_source(ctx, "*out = obj->json; ");
				}
				emit_source(ctx, "return LSP_OK; }\n");

				free(ref_enum);
			} else {
				// Struct-like reference (structure, array, map, or, alias, unknown)
				char *ref_struct = to_struct_type(ref_name);
				char *ref_func = to_func_name(ref_name);

				emit_header(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out);\n",
				            func_prefix, func_suffix, struct_type, ref_struct);
				emit_source(ctx, "int lsp_%s__as_%s(const %s *obj, %s *out) { ",
				            func_prefix, func_suffix, struct_type, ref_struct);
				emit_source(ctx, "if (!lsp_%s__is_%s(obj)) return LSP_ERR_MISSING_FIELD; ",
				            func_prefix, func_suffix);
				emit_source(ctx, "return lsp_%s__from_json(out, obj->json); }\n", ref_func);

				free(ref_func);
				free(ref_struct);
			}
		}

		free(func_suffix);
		free(enum_name);
	}

	free(struct_type);
	free(const_prefix);
	free(func_prefix);
}

/*
 * Enumeration generation
 */

static void
gen_enumeration(struct GenCtx *ctx, json_object *type_def) {
	const char *name = json_get_string(type_def, "name");
	if (!gen_ctx_mark(ctx, "enum", name)) {
		return;
	}

	const char *norm_name = lsp_name_normalize(name);
	char *func_prefix = to_func_name(name);
	char *const_prefix = to_constant_name(name);
	bool is_string = ir_reader_enum_is_string(type_def);

	json_object *values = json_get_array(type_def, "values");
	size_t val_count = json_array_len(values);

	if (is_string) {
		emit_header(ctx, "enum Lsp%s { ", norm_name);
		for (size_t i = 0; i < val_count; i++) {
			json_object *val = json_array_get(values, i);
			const char *vname = json_get_string(val, "name");
			char *vname_upper = str_to_upper_snake(vname);
			emit_header(ctx, "LSP_%s_%s%s", const_prefix, vname_upper,
			            i + 1 < val_count ? ", " : "");
			free(vname_upper);
		}
		emit_header(ctx, " };\n");

		emit_header(ctx, "int lsp_%s__from_string(const char *str, enum Lsp%s *out);\n",
		            func_prefix, norm_name);
		emit_header(ctx, "const char *lsp_%s__to_string(enum Lsp%s value);\n",
		            func_prefix, norm_name);

		emit_source(ctx, "int lsp_%s__from_string(const char *str, enum Lsp%s *out) { ",
		            func_prefix, norm_name);
		emit_source(ctx, "if (str == NULL) return LSP_ERR_MISSING_FIELD; ");
		for (size_t i = 0; i < val_count; i++) {
			json_object *val = json_array_get(values, i);
			const char *vname = json_get_string(val, "name");
			json_object *vvalue = json_get_object(val, "value");
			const char *vstr = json_object_get_string(vvalue);
			char *vname_upper = str_to_upper_snake(vname);
			emit_source(ctx, "if (strcmp(str, \"%s\") == 0) { *out = LSP_%s_%s; return LSP_OK; } ",
			            vstr, const_prefix, vname_upper);
			free(vname_upper);
		}
		emit_source(ctx, "return LSP_ERR_INVALID_TYPE; }\n");

		emit_source(ctx, "const char *lsp_%s__to_string(enum Lsp%s value) { ",
		            func_prefix, norm_name);
		emit_source(ctx, "switch (value) { ");
		for (size_t i = 0; i < val_count; i++) {
			json_object *val = json_array_get(values, i);
			const char *vname = json_get_string(val, "name");
			json_object *vvalue = json_get_object(val, "value");
			const char *vstr = json_object_get_string(vvalue);
			char *vname_upper = str_to_upper_snake(vname);
			emit_source(ctx, "case LSP_%s_%s: return \"%s\"; ",
			            const_prefix, vname_upper, vstr);
			free(vname_upper);
		}
		emit_source(ctx, "default: return NULL; } }\n");
	} else {
		emit_header(ctx, "enum Lsp%s { ", norm_name);
		for (size_t i = 0; i < val_count; i++) {
			json_object *val = json_array_get(values, i);
			const char *vname = json_get_string(val, "name");
			json_object *vvalue = json_get_object(val, "value");
			int64_t vint = json_object_get_int64(vvalue);
			char *vname_upper = str_to_upper_snake(vname);
			emit_header(ctx, "LSP_%s_%s = %lld%s", const_prefix, vname_upper,
			            (long long)vint, i + 1 < val_count ? ", " : "");
			free(vname_upper);
		}
		emit_header(ctx, " };\n");
	}

	free(const_prefix);
	free(func_prefix);
}

/*
 * Main emit all types - emits in proper order for forward references
 */

static void
emit_all_types(struct GenCtx *ctx) {
	size_t type_count = json_array_len(ctx->ir->types);

	// Pass 1: Emit all enumerations first (they're referenced by other types)
	emit_header(ctx, "/* Enumerations */\n\n");
	for (size_t i = 0; i < type_count; i++) {
		json_object *type_def = json_array_get(ctx->ir->types, i);
		if (ir_reader_type_kind(type_def) == IR_TYPE_ENUMERATION) {
			gen_enumeration(ctx, type_def);
		}
	}

	// Pass 2: Forward declarations for struct-like types
	emit_header(ctx, "/* Forward declarations */\n");
	for (size_t i = 0; i < type_count; i++) {
		json_object *type_def = json_array_get(ctx->ir->types, i);
		enum IrTypeKind kind = ir_reader_type_kind(type_def);
		if (kind == IR_TYPE_STRUCTURE || kind == IR_TYPE_ARRAY ||
		    kind == IR_TYPE_MAP || kind == IR_TYPE_OR ||
		    kind == IR_TYPE_TUPLE) {
			const char *name = json_get_string(type_def, "name");
			const char *norm_name = lsp_name_normalize(name);
			emit_header(ctx, "struct Lsp%s;\n", norm_name);
		}
	}
	emit_header(ctx, "\n");

	// Pass 3: Emit all other types
	for (size_t i = 0; i < type_count; i++) {
		json_object *type_def = json_array_get(ctx->ir->types, i);
		enum IrTypeKind kind = ir_reader_type_kind(type_def);

		switch (kind) {
		case IR_TYPE_STRUCTURE:
			gen_structure(ctx, type_def);
			break;
		case IR_TYPE_ENUMERATION:
			// Already emitted in pass 1
			break;
		case IR_TYPE_ARRAY:
			gen_array_type(ctx, type_def);
			break;
		case IR_TYPE_MAP:
			gen_map_type(ctx, type_def);
			break;
		case IR_TYPE_OR:
			gen_or_type(ctx, type_def);
			break;
		case IR_TYPE_TUPLE:
			gen_tuple_type(ctx, type_def);
			break;
		default:
			break;
		}
	}
}

/*
 * Main
 */

static void
usage(const char *prog) {
	fprintf(stderr, "Usage: %s <ir.json> <output.h> <output.c>\n", prog);
	exit(1);
}

int
main(int argc, char **argv) {
	if (argc != 4) {
		usage(argv[0]);
	}

	const char *ir_path = argv[1];
	const char *hdr_path = argv[2];
	const char *src_path = argv[3];

	json_object *ir = json_object_from_file(ir_path);
	if (ir == NULL) {
		fprintf(stderr, "Failed to read %s\n", ir_path);
		return 1;
	}

	FILE *hdr = fopen(hdr_path, "w");
	if (hdr == NULL) {
		fprintf(stderr, "Failed to open %s for writing\n", hdr_path);
		json_object_put(ir);
		return 1;
	}

	FILE *src = fopen(src_path, "w");
	if (src == NULL) {
		fprintf(stderr, "Failed to open %s for writing\n", src_path);
		fclose(hdr);
		json_object_put(ir);
		return 1;
	}

	struct IrReaderCtx ir_ctx;
	ir_reader_init(&ir_ctx, ir);

	struct GenCtx gen_ctx;
	gen_ctx_init(&gen_ctx, &ir_ctx, hdr, src);

	emit_header_prologue(&gen_ctx);
	emit_source_prologue(&gen_ctx);
	emit_all_types(&gen_ctx);
	emit_header_epilogue(&gen_ctx);

	gen_ctx_cleanup(&gen_ctx);
	fclose(src);
	fclose(hdr);
	json_object_put(ir);

	return 0;
}
