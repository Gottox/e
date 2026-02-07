#include "common.h"

/*
 * Namer functions - generate names for anonymous types
 */

static char *namer_type_to_name(json_object *type);
static char *namer_array_name(json_object *element_type);
static char *namer_map_name(json_object *value_type);
static char *namer_or_name(json_object *items_array);
static char *namer_tuple_name(json_object *items_array);
static char *namer_and_name(json_object *items_array);
static char *namer_literal_name(json_object *properties_array);

static char *
namer_type_to_name(json_object *type) {
	const char *kind = json_get_string(type, "kind");
	if (kind == NULL) {
		return strdup("Unknown");
	}

	if (strcmp(kind, "base") == 0) {
		const char *name = json_get_string(type, "name");
		return str_capitalize(name ? name : "unknown");
	} else if (strcmp(kind, "reference") == 0) {
		const char *name = json_get_string(type, "name");
		return strdup(lsp_name_normalize(name ? name : "Unknown"));
	} else if (strcmp(kind, "array") == 0) {
		json_object *elem = json_get_object(type, "element");
		return namer_array_name(elem);
	} else if (strcmp(kind, "map") == 0) {
		json_object *val = json_get_object(type, "value");
		return namer_map_name(val);
	} else if (strcmp(kind, "literal") == 0) {
		json_object *value = json_get_object(type, "value");
		json_object *properties = json_get_array(value, "properties");
		return namer_literal_name(properties);
	} else if (strcmp(kind, "tuple") == 0) {
		json_object *items = json_get_array(type, "items");
		return namer_tuple_name(items);
	} else if (strcmp(kind, "stringLiteral") == 0) {
		return strdup("StringLiteral");
	} else if (strcmp(kind, "or") == 0) {
		json_object *items = json_get_array(type, "items");
		return namer_or_name(items);
	}

	return strdup("Unknown");
}

static char *
namer_array_name(json_object *element_type) {
	char *elem_name = namer_type_to_name(element_type);
	char *result = NULL;
	int ret = asprintf(&result, "%sArray", elem_name ? elem_name : "Unknown");
	assert(ret >= 0 && result != NULL);
	free(elem_name);
	return result;
}

static char *
namer_map_name(json_object *value_type) {
	char *val_name = namer_type_to_name(value_type);
	char *result = NULL;
	int ret = asprintf(&result, "Map%s", val_name ? val_name : "Unknown");
	assert(ret >= 0 && result != NULL);
	free(val_name);
	return result;
}

static char *
namer_or_name(json_object *items_array) {
	size_t n = json_array_len(items_array);
	if (n == 0) {
		return strdup("Empty");
	}

	json_object *first = json_array_get(items_array, 0);
	char *result = namer_type_to_name(first);

	for (size_t i = 1; i < n; i++) {
		json_object *item = json_array_get(items_array, i);
		char *item_name = namer_type_to_name(item);
		char *new_result = NULL;
		int ret = asprintf(
				&new_result, "%sOr%s", result,
				item_name ? item_name : "Unknown");
		assert(ret >= 0 && new_result != NULL);
		free(result);
		free(item_name);
		result = new_result;
	}

	return result;
}

static char *
namer_tuple_name(json_object *items_array) {
	size_t n = json_array_len(items_array);
	if (n == 0) {
		return strdup("EmptyTuple");
	}

	json_object *first = json_array_get(items_array, 0);
	char *result = namer_type_to_name(first);

	for (size_t i = 1; i < n; i++) {
		json_object *item = json_array_get(items_array, i);
		char *item_name = namer_type_to_name(item);
		char *new_result = NULL;
		int ret = asprintf(
				&new_result, "%sAnd%s", result,
				item_name ? item_name : "Unknown");
		assert(ret >= 0 && new_result != NULL);
		free(result);
		free(item_name);
		result = new_result;
	}

	char *final = NULL;
	int ret = asprintf(&final, "%sTuple", result);
	assert(ret >= 0 && final != NULL);
	free(result);
	return final;
}

static char *
namer_and_name(json_object *items_array) {
	size_t n = json_array_len(items_array);
	if (n == 0) {
		return strdup("Empty");
	}

	json_object *first = json_array_get(items_array, 0);
	char *result = namer_type_to_name(first);

	for (size_t i = 1; i < n; i++) {
		json_object *item = json_array_get(items_array, i);
		char *item_name = namer_type_to_name(item);
		char *new_result = NULL;
		int ret = asprintf(
				&new_result, "%sAnd%s", result,
				item_name ? item_name : "Unknown");
		assert(ret >= 0 && new_result != NULL);
		free(result);
		free(item_name);
		result = new_result;
	}

	return result;
}

static char *
namer_literal_name(json_object *properties_array) {
	size_t n = json_array_len(properties_array);
	char *result = strdup("");

	for (size_t i = 0; i < n; i++) {
		json_object *prop = json_array_get(properties_array, i);
		const char *prop_name = json_get_string(prop, "name");
		if (prop_name == NULL || prop_name[0] == '\0') {
			continue;
		}

		json_object *optional_obj = json_get_object(prop, "optional");
		bool optional = optional_obj ? json_object_get_boolean(optional_obj) : false;

		json_object *prop_type = json_get_object(prop, "type");
		char *type_name = namer_type_to_name(prop_type);

		char *capitalized = str_capitalize(prop_name);
		char *new_result = NULL;
		int ret = asprintf(&new_result, "%s%s%s%s", result, capitalized,
		                   type_name ? type_name : "",
		                   optional ? "Opt" : "");
		assert(ret >= 0 && new_result != NULL);
		free(result);
		free(capitalized);
		free(type_name);
		result = new_result;
	}

	char *final = NULL;
	int ret = asprintf(&final, "%sLiteral", result);
	assert(ret >= 0 && final != NULL);
	free(result);
	return final;
}

/*
 * IR generation context and functions
 */

struct IrCtx {
	json_object *model;
	json_object *types;
	json_object *references;    /* {"name": {resolved type}} - internal map for resolution */
	json_object *requests;
	json_object *notifications;
};

static void
ir_ctx_init(struct IrCtx *ctx, json_object *model) {
	ctx->model = model;
	ctx->types = json_new_array();
	ctx->references = json_new_object();
	ctx->requests = json_new_array();
	ctx->notifications = json_new_array();
}

static void
ir_ctx_cleanup(struct IrCtx *ctx) {
	json_object_put(ctx->types);
	json_object_put(ctx->references);
	json_object_put(ctx->requests);
	json_object_put(ctx->notifications);
}

static json_object *ir_flatten_type(struct IrCtx *ctx, json_object *type);
static json_object *
ir_collect_all_properties(struct IrCtx *ctx, json_object *structure);

static json_object *
find_structure(struct IrCtx *ctx, const char *name) {
	json_object *array = json_get_array(ctx->model, "structures");
	size_t n = json_array_len(array);
	for (size_t i = 0; i < n; i++) {
		json_object *item = json_array_get(array, i);
		const char *item_name = json_get_string(item, "name");
		if (item_name && strcmp(item_name, name) == 0) {
			return item;
		}
	}
	return NULL;
}

static json_object *
ir_make_reference(const char *name, const char *type_kind) {
	json_object *ref = json_new_object();
	json_add_string(ref, "kind", "reference");
	json_add_string(ref, "name", name);
	if (type_kind != NULL) {
		json_add_string(ref, "typeKind", type_kind);
	}
	return ref;
}

static void
ir_add_reference(struct IrCtx *ctx, const char *name, const char *type_kind) {
	json_object_object_add(ctx->references, name, ir_make_reference(name, type_kind));
}

/*
 * Register a type definition if not already registered.
 * Returns a reference to the type. Takes ownership of name and definition.
 * If the type already exists, the definition is freed and an existing reference is returned.
 */
static json_object *
ir_register_type(struct IrCtx *ctx, char *name, const char *type_kind,
                 json_object *definition) {
	json_object *existing = json_get_object(ctx->references, name);
	if (existing != NULL) {
		json_object_put(definition);
		json_object *ref = ir_make_reference(name, type_kind);
		free(name);
		return ref;
	}

	json_add_string(definition, "name", name);
	json_array_add(ctx->types, definition);
	ir_add_reference(ctx, name, type_kind);

	json_object *ref = ir_make_reference(name, type_kind);
	free(name);
	return ref;
}

static json_object *
ir_flatten_property(struct IrCtx *ctx, json_object *prop) {
	const char *name = json_get_string(prop, "name");
	json_object *type = json_get_object(prop, "type");
	json_object *optional_obj = json_get_object(prop, "optional");
	bool optional =
			optional_obj ? json_object_get_boolean(optional_obj) : false;

	json_object *flattened_type = ir_flatten_type(ctx, type);

	json_object *result = json_new_object();
	json_add_string(result, "name", name);
	json_add_object(result, "type", flattened_type);
	json_add_bool(result, "optional", optional);

	return result;
}

static void
add_props_to_array(
		struct IrCtx *ctx, json_object *result, json_object *seen_names,
		json_object *props) {
	size_t n = json_array_len(props);
	for (size_t i = 0; i < n; i++) {
		json_object *prop = json_array_get(props, i);
		const char *pname = json_get_string(prop, "name");
		if (pname && !json_has_key(seen_names, pname)) {
			json_add_bool(seen_names, pname, true);
			json_object *flattened = ir_flatten_property(ctx, prop);
			json_array_add(result, flattened);
		}
	}
}

static void
inline_inherited_props(
		struct IrCtx *ctx, json_object *result, json_object *seen_names,
		json_object *structure, const char *field_name) {
	json_object *refs = json_get_array(structure, field_name);
	size_t count = json_array_len(refs);
	for (size_t i = 0; i < count; i++) {
		json_object *ref = json_array_get(refs, i);
		const char *ref_name = json_get_string(ref, "name");
		if (ref_name == NULL) {
			continue;
		}
		json_object *parent = find_structure(ctx, ref_name);
		if (parent == NULL) {
			continue;
		}
		json_object *parent_props = ir_collect_all_properties(ctx, parent);
		size_t pn = json_array_len(parent_props);
		for (size_t j = 0; j < pn; j++) {
			json_object *prop = json_array_get(parent_props, j);
			const char *pname = json_get_string(prop, "name");
			if (!pname) {
				continue;
			}
			if (json_has_key(seen_names, pname)) {
				fprintf(stderr,
						"Warning: property '%s' from '%s' is overridden in "
						"'%s'\n",
						pname, ref_name, json_get_string(structure, "name"));
				continue;
			}
			json_add_bool(seen_names, pname, true);
			json_array_add(result, json_object_get(prop));
		}
		json_object_put(parent_props);
	}
}

static json_object *
ir_collect_all_properties(struct IrCtx *ctx, json_object *structure) {
	json_object *result = json_new_array();
	json_object *seen_names = json_new_object();

	json_object *own_props = json_get_array(structure, "properties");
	add_props_to_array(ctx, result, seen_names, own_props);

	inline_inherited_props(ctx, result, seen_names, structure, "extends");
	inline_inherited_props(ctx, result, seen_names, structure, "mixins");

	json_object_put(seen_names);
	return result;
}

static json_object *
ir_flatten_type(struct IrCtx *ctx, json_object *type) {
	if (type == NULL) {
		return NULL;
	}

	const char *kind = json_get_string(type, "kind");
	if (kind == NULL) {
		return json_object_get(type);
	}

	if (strcmp(kind, "base") == 0 || strcmp(kind, "reference") == 0 ||
		strcmp(kind, "stringLiteral") == 0) {
		return json_object_get(type);
	} else if (strcmp(kind, "array") == 0) {
		json_object *elem = json_get_object(type, "element");
		json_object *flat_elem = ir_flatten_type(ctx, elem);
		char *name = namer_array_name(flat_elem);

		json_object *def = json_new_object();
		json_add_object(def, "element", flat_elem);
		return ir_register_type(ctx, name, "array", def);
	} else if (strcmp(kind, "map") == 0) {
		json_object *val = json_get_object(type, "value");
		json_object *flat_val = ir_flatten_type(ctx, val);
		char *name = namer_map_name(flat_val);

		json_object *def = json_new_object();
		json_add_object(def, "value", flat_val);
		return ir_register_type(ctx, name, "map", def);
	} else if (strcmp(kind, "or") == 0) {
		json_object *items = json_get_array(type, "items");
		json_object *flat_items = json_new_array();
		size_t n = json_array_len(items);
		for (size_t i = 0; i < n; i++) {
			json_object *item = json_array_get(items, i);
			json_object *flat_item = ir_flatten_type(ctx, item);
			json_array_add(flat_items, flat_item);
		}

		char *name = namer_or_name(flat_items);

		json_object *def = json_new_object();
		json_add_object(def, "items", flat_items);
		return ir_register_type(ctx, name, "or", def);
	} else if (strcmp(kind, "literal") == 0) {
		json_object *value = json_get_object(type, "value");
		json_object *props = json_get_array(value, "properties");

		json_object *flat_props = json_new_array();
		size_t n = json_array_len(props);
		for (size_t i = 0; i < n; i++) {
			json_object *prop = json_array_get(props, i);
			json_object *flat_prop = ir_flatten_property(ctx, prop);
			json_array_add(flat_props, flat_prop);
		}

		char *name = namer_literal_name(flat_props);

		json_object *def = json_new_object();
		json_add_object(def, "properties", flat_props);
		return ir_register_type(ctx, name, "structure", def);
	} else if (strcmp(kind, "tuple") == 0) {
		json_object *items = json_get_array(type, "items");
		json_object *flat_elements = json_new_array();
		size_t n = json_array_len(items);
		for (size_t i = 0; i < n; i++) {
			json_object *item = json_array_get(items, i);
			json_object *flat_item = ir_flatten_type(ctx, item);
			json_array_add(flat_elements, flat_item);
		}

		char *name = namer_tuple_name(flat_elements);

		json_object *def = json_new_object();
		json_add_object(def, "elements", flat_elements);
		return ir_register_type(ctx, name, "tuple", def);
	} else if (strcmp(kind, "and") == 0) {
		json_object *items = json_get_array(type, "items");
		size_t n = json_array_len(items);
		if (n == 0) {
			json_object *ref = json_new_object();
			json_add_string(ref, "kind", "base");
			json_add_string(ref, "name", "LSPAny");
			return ref;
		}

		json_object *merged_props = json_new_array();
		json_object *seen_names = json_new_object();

		for (size_t i = 0; i < n; i++) {
			json_object *item = json_array_get(items, i);
			const char *item_kind = json_get_string(item, "kind");

			if (item_kind && strcmp(item_kind, "reference") == 0) {
				const char *ref_name = json_get_string(item, "name");
				json_object *structure = find_structure(ctx, ref_name);
				if (structure != NULL) {
					json_object *props = ir_collect_all_properties(ctx, structure);
					size_t pn = json_array_len(props);
					for (size_t j = 0; j < pn; j++) {
						json_object *prop = json_array_get(props, j);
						const char *pname = json_get_string(prop, "name");
						if (pname && !json_has_key(seen_names, pname)) {
							json_add_bool(seen_names, pname, true);
							json_array_add(merged_props, json_object_get(prop));
						}
					}
					json_object_put(props);
				}
			} else if (item_kind && strcmp(item_kind, "literal") == 0) {
				json_object *value = json_get_object(item, "value");
				json_object *props = json_get_array(value, "properties");
				add_props_to_array(ctx, merged_props, seen_names, props);
			}
		}

		json_object_put(seen_names);

		char *name = namer_and_name(items);

		json_object *def = json_new_object();
		json_add_object(def, "properties", merged_props);
		return ir_register_type(ctx, name, "structure", def);
	}

	return json_object_get(type);
}

static void
ir_flatten_structure(struct IrCtx *ctx, json_object *structure) {
	const char *name = json_get_string(structure, "name");
	if (name == NULL) {
		return;
	}

	json_object *all_props = ir_collect_all_properties(ctx, structure);

	json_object *def = json_new_object();
	json_add_string(def, "name", name);
	json_add_object(def, "properties", all_props);

	json_array_add(ctx->types, def);
	ir_add_reference(ctx, name, "structure");
}

static void
ir_flatten_enumeration(struct IrCtx *ctx, json_object *enumeration) {
	const char *name = json_get_string(enumeration, "name");
	if (name == NULL) {
		return;
	}

	json_object *type = json_get_object(enumeration, "type");
	json_object *values = json_get_array(enumeration, "values");

	json_object *values_copy = json_new_array();
	size_t n = json_array_len(values);
	for (size_t i = 0; i < n; i++) {
		json_object *val = json_array_get(values, i);
		const char *vname = json_get_string(val, "name");
		json_object *vvalue = json_get_object(val, "value");

		json_object *vcopy = json_new_object();
		json_add_string(vcopy, "name", vname);
		if (json_object_is_type(vvalue, json_type_string)) {
			json_add_string(vcopy, "value", json_object_get_string(vvalue));
		} else if (json_object_is_type(vvalue, json_type_int)) {
			json_object_object_add(
					vcopy, "value",
					json_object_new_int64(json_object_get_int64(vvalue)));
		}
		json_array_add(values_copy, vcopy);
	}

	json_object *def = json_new_object();
	json_add_string(def, "name", name);
	json_add_object(def, "type", json_object_get(type));
	json_add_object(def, "values", values_copy);

	json_array_add(ctx->types, def);

	/* Determine if string-based enumeration */
	const char *type_kind = "enumeration";
	const char *type_name = json_get_string(type, "name");
	if (type_name && strcmp(type_name, "string") == 0) {
		type_kind = "stringEnumeration";
	}

	ir_add_reference(ctx, name, type_kind);
}

static void
ir_flatten_type_alias(struct IrCtx *ctx, json_object *alias) {
	const char *name = json_get_string(alias, "name");
	if (name == NULL) {
		return;
	}

	json_object *type = json_get_object(alias, "type");
	const char *kind = json_get_string(type, "kind");

	/* Reference aliases: store for resolution (will be resolved away) */
	if (kind && strcmp(kind, "reference") == 0) {
		json_object *flat_type = ir_flatten_type(ctx, type);
		json_object_object_add(ctx->references, name, flat_type);
		return;
	}

	/* Autotype aliases: create type with alias name */
	if (kind && strcmp(kind, "or") == 0) {
		json_object *items = json_get_array(type, "items");
		json_object *flat_items = json_new_array();
		size_t n = json_array_len(items);
		for (size_t i = 0; i < n; i++) {
			json_object *item = json_array_get(items, i);
			json_object *flat_item = ir_flatten_type(ctx, item);
			json_array_add(flat_items, flat_item);
		}

		json_object *def = json_new_object();
		json_add_string(def, "name", name);
		json_add_object(def, "items", flat_items);
		json_array_add(ctx->types, def);
		ir_add_reference(ctx, name, "or");
		return;
	}

	if (kind && strcmp(kind, "array") == 0) {
		json_object *elem = json_get_object(type, "element");
		json_object *flat_elem = ir_flatten_type(ctx, elem);

		json_object *def = json_new_object();
		json_add_string(def, "name", name);
		json_add_object(def, "element", flat_elem);
		json_array_add(ctx->types, def);
		ir_add_reference(ctx, name, "array");
		return;
	}

	if (kind && strcmp(kind, "map") == 0) {
		json_object *val = json_get_object(type, "value");
		json_object *flat_val = ir_flatten_type(ctx, val);

		json_object *def = json_new_object();
		json_add_string(def, "name", name);
		json_add_object(def, "value", flat_val);
		json_array_add(ctx->types, def);
		ir_add_reference(ctx, name, "map");
		return;
	}

	if (kind && strcmp(kind, "literal") == 0) {
		json_object *value = json_get_object(type, "value");
		json_object *props = json_get_array(value, "properties");

		json_object *flat_props = json_new_array();
		size_t n = json_array_len(props);
		for (size_t i = 0; i < n; i++) {
			json_object *prop = json_array_get(props, i);
			json_object *flat_prop = ir_flatten_property(ctx, prop);
			json_array_add(flat_props, flat_prop);
		}

		json_object *def = json_new_object();
		json_add_string(def, "name", name);
		json_add_object(def, "properties", flat_props);
		json_array_add(ctx->types, def);
		ir_add_reference(ctx, name, "structure");
		return;
	}

	/* Fallback: base types or other - store for resolution */
	json_object *flat_type = ir_flatten_type(ctx, type);
	json_object_object_add(ctx->references, name, flat_type);
}

static void
ir_flatten_request(struct IrCtx *ctx, json_object *request) {
	json_object *result = json_new_object();

	const char *method = json_get_string(request, "method");
	json_add_string(result, "method", method);

	const char *direction = json_get_string(request, "messageDirection");
	if (direction) {
		json_add_string(result, "messageDirection", direction);
	}

	json_object *params = json_get_object(request, "params");
	if (params) {
		json_object *flat_params = ir_flatten_type(ctx, params);
		json_add_object(result, "params", flat_params);
	}

	json_object *res = json_get_object(request, "result");
	if (res) {
		json_object *flat_result = ir_flatten_type(ctx, res);
		json_add_object(result, "result", flat_result);
	}

	json_array_add(ctx->requests, result);
}

static void
ir_flatten_notification(struct IrCtx *ctx, json_object *notification) {
	json_object *result = json_new_object();

	const char *method = json_get_string(notification, "method");
	json_add_string(result, "method", method);

	const char *direction = json_get_string(notification, "messageDirection");
	if (direction) {
		json_add_string(result, "messageDirection", direction);
	}

	json_object *params = json_get_object(notification, "params");
	if (params) {
		json_object *flat_params = ir_flatten_type(ctx, params);
		json_add_object(result, "params", flat_params);
	}

	json_array_add(ctx->notifications, result);
}

/*
 * Alias resolution - replaces alias references with resolved types
 */

/* Single-step lookup (call after ir_resolve_references_map) */
static json_object *
ir_resolve_alias(struct IrCtx *ctx, json_object *ref) {
	if (ref == NULL) {
		return NULL;
	}
	const char *kind = json_get_string(ref, "kind");
	if (kind == NULL || strcmp(kind, "reference") != 0) {
		return ref;
	}

	const char *name = json_get_string(ref, "name");
	if (name == NULL) {
		return ref;
	}

	json_object *resolved = json_object_object_get(ctx->references, name);
	if (resolved == NULL) {
		return ref;  /* Not in references map */
	}

	/* Self-reference means concrete type, not an alias */
	const char *resolved_name = json_get_string(resolved, "name");
	if (resolved_name != NULL && strcmp(resolved_name, name) == 0) {
		return ref;
	}

	return resolved;
}

/* Pre-resolve chained aliases so all lookups become single-step */
static void
ir_resolve_references_map(struct IrCtx *ctx) {
	bool changed = true;
	int max_iterations = 100;  /* Prevent infinite loops */

	while (changed && max_iterations-- > 0) {
		changed = false;
		json_object_object_foreach(ctx->references, key, ref) {
			const char *kind = json_get_string(ref, "kind");
			if (kind == NULL || strcmp(kind, "reference") != 0) {
				continue;
			}

			const char *target_name = json_get_string(ref, "name");
			if (target_name == NULL || strcmp(target_name, key) == 0) {
				continue;  /* Self-reference, skip */
			}

			json_object *target = json_object_object_get(ctx->references, target_name);
			if (target == NULL) {
				continue;  /* Target not in map */
			}

			const char *target_ref_name = json_get_string(target, "name");
			if (target_ref_name != NULL && strcmp(target_ref_name, target_name) == 0) {
				continue;  /* Target is self-reference (concrete type) */
			}

			/* Replace with resolved target */
			json_object_object_add(ctx->references, key, json_object_get(target));
			changed = true;
		}
	}
}

static void
ir_resolve_and_replace(struct IrCtx *ctx, json_object *parent, const char *key) {
	json_object *ref = json_get_object(parent, key);
	if (ref == NULL) {
		return;
	}
	json_object *resolved = ir_resolve_alias(ctx, ref);
	if (resolved != ref) {
		json_object_object_add(parent, key, json_object_get(resolved));
	}
}

static void
ir_resolve_aliases_in_types(struct IrCtx *ctx) {
	size_t type_count = json_array_len(ctx->types);
	for (size_t t = 0; t < type_count; t++) {
		json_object *def = json_array_get(ctx->types, t);

		/* Structure properties */
		json_object *props = json_get_array(def, "properties");
		size_t prop_count = json_array_len(props);
		for (size_t i = 0; i < prop_count; i++) {
			json_object *prop = json_array_get(props, i);
			ir_resolve_and_replace(ctx, prop, "type");
		}

		/* Array element */
		ir_resolve_and_replace(ctx, def, "element");

		/* Map value */
		ir_resolve_and_replace(ctx, def, "value");

		/* Or-type items */
		json_object *items = json_get_array(def, "items");
		size_t item_count = json_array_len(items);
		for (size_t i = 0; i < item_count; i++) {
			json_object *item = json_array_get(items, i);
			json_object *resolved = ir_resolve_alias(ctx, item);
			if (resolved != item) {
				json_object_array_put_idx(items, i, json_object_get(resolved));
			}
		}

		/* Tuple elements */
		json_object *elements = json_get_array(def, "elements");
		size_t elem_count = json_array_len(elements);
		for (size_t i = 0; i < elem_count; i++) {
			json_object *elem = json_array_get(elements, i);
			json_object *resolved = ir_resolve_alias(ctx, elem);
			if (resolved != elem) {
				json_object_array_put_idx(elements, i, json_object_get(resolved));
			}
		}
	}

	/* Requests: params and result */
	size_t req_count = json_array_len(ctx->requests);
	for (size_t i = 0; i < req_count; i++) {
		json_object *req = json_array_get(ctx->requests, i);
		ir_resolve_and_replace(ctx, req, "params");
		ir_resolve_and_replace(ctx, req, "result");
	}

	/* Notifications: params */
	size_t notif_count = json_array_len(ctx->notifications);
	for (size_t i = 0; i < notif_count; i++) {
		json_object *notif = json_array_get(ctx->notifications, i);
		ir_resolve_and_replace(ctx, notif, "params");
	}
}

/*
 * Reference enrichment - adds typeKind to references by looking up ctx->references
 */

static const char *
ir_get_type_kind_string(struct IrCtx *ctx, const char *name) {
	json_object *ref = json_get_object(ctx->references, name);
	if (ref == NULL) {
		return NULL;
	}
	return json_get_string(ref, "typeKind");
}

static void
ir_enrich_reference(struct IrCtx *ctx, json_object *ref) {
	if (ref == NULL) {
		return;
	}
	const char *kind = json_get_string(ref, "kind");
	if (kind == NULL || strcmp(kind, "reference") != 0) {
		return;
	}

	const char *name = json_get_string(ref, "name");
	if (name == NULL) {
		return;
	}

	const char *type_kind = ir_get_type_kind_string(ctx, name);
	if (type_kind != NULL) {
		json_add_string(ref, "typeKind", type_kind);
	}
}

static void
ir_enrich_references(struct IrCtx *ctx) {
	/* Walk all types and enrich references */
	size_t type_count = json_array_len(ctx->types);
	for (size_t t = 0; t < type_count; t++) {
		json_object *def = json_array_get(ctx->types, t);

		/* Structure properties: types[X].properties[Y].type */
		json_object *props = json_get_array(def, "properties");
		size_t prop_count = json_array_len(props);
		for (size_t i = 0; i < prop_count; i++) {
			json_object *prop = json_array_get(props, i);
			json_object *prop_type = json_get_object(prop, "type");
			ir_enrich_reference(ctx, prop_type);
		}

		/* Array element: types[X].element */
		json_object *element = json_get_object(def, "element");
		ir_enrich_reference(ctx, element);

		/* Map value: types[X].value */
		json_object *value = json_get_object(def, "value");
		ir_enrich_reference(ctx, value);

		/* Or-type items: types[X].items[Y] */
		json_object *items = json_get_array(def, "items");
		size_t item_count = json_array_len(items);
		for (size_t i = 0; i < item_count; i++) {
			json_object *item = json_array_get(items, i);
			ir_enrich_reference(ctx, item);
		}

		/* Tuple elements: types[X].elements[Y] */
		json_object *elements = json_get_array(def, "elements");
		size_t elem_count = json_array_len(elements);
		for (size_t i = 0; i < elem_count; i++) {
			json_object *elem = json_array_get(elements, i);
			ir_enrich_reference(ctx, elem);
		}

		/* Alias target: types[X].type */
		json_object *type = json_get_object(def, "type");
		ir_enrich_reference(ctx, type);
	}

	/* Walk requests: params and result */
	size_t req_count = json_array_len(ctx->requests);
	for (size_t i = 0; i < req_count; i++) {
		json_object *req = json_array_get(ctx->requests, i);
		json_object *params = json_get_object(req, "params");
		ir_enrich_reference(ctx, params);
		json_object *result = json_get_object(req, "result");
		ir_enrich_reference(ctx, result);
	}

	/* Walk notifications: params */
	size_t notif_count = json_array_len(ctx->notifications);
	for (size_t i = 0; i < notif_count; i++) {
		json_object *notif = json_array_get(ctx->notifications, i);
		json_object *params = json_get_object(notif, "params");
		ir_enrich_reference(ctx, params);
	}
}

static json_object *
ir_flatten(struct IrCtx *ctx) {
	json_object *structures = json_get_array(ctx->model, "structures");
	size_t struct_count = json_array_len(structures);
	for (size_t i = 0; i < struct_count; i++) {
		ir_flatten_structure(ctx, json_array_get(structures, i));
	}

	json_object *enumerations = json_get_array(ctx->model, "enumerations");
	size_t enum_count = json_array_len(enumerations);
	for (size_t i = 0; i < enum_count; i++) {
		ir_flatten_enumeration(ctx, json_array_get(enumerations, i));
	}

	json_object *aliases = json_get_array(ctx->model, "typeAliases");
	size_t alias_count = json_array_len(aliases);
	for (size_t i = 0; i < alias_count; i++) {
		ir_flatten_type_alias(ctx, json_array_get(aliases, i));
	}

	json_object *requests = json_get_array(ctx->model, "requests");
	size_t req_count = json_array_len(requests);
	for (size_t i = 0; i < req_count; i++) {
		ir_flatten_request(ctx, json_array_get(requests, i));
	}

	json_object *notifications = json_get_array(ctx->model, "notifications");
	size_t notif_count = json_array_len(notifications);
	for (size_t i = 0; i < notif_count; i++) {
		ir_flatten_notification(ctx, json_array_get(notifications, i));
	}

	ir_resolve_references_map(ctx);
	ir_resolve_aliases_in_types(ctx);
	ir_enrich_references(ctx);

	json_object *ir = json_new_object();

	json_object *metadata = json_get_object(ctx->model, "metaData");
	const char *version = json_get_string(metadata, "version");
	if (version) {
		json_add_string(ir, "version", version);
	}

	json_add_object(ir, "types", json_object_get(ctx->types));
	json_add_object(ir, "requests", json_object_get(ctx->requests));
	json_add_object(ir, "notifications", json_object_get(ctx->notifications));

	return ir;
}

/*
 * Main
 */

static void
usage(const char *prog) {
	fprintf(stderr, "Usage: %s <metaModel.json>\n", prog);
	exit(1);
}

int
main(int argc, char **argv) {
	int rv = 0;
	if (argc != 2) {
		usage(argv[0]);
	}

	const char *input_path = argv[1];

	json_object *model = json_object_from_file(input_path);
	if (model == NULL) {
		fprintf(stderr, "Failed to read %s\n", input_path);
		return 1;
	}

	struct IrCtx ctx;
	ir_ctx_init(&ctx, model);
	json_object *ir = ir_flatten(&ctx);
	ir_ctx_cleanup(&ctx);

	int flags = JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED;
	const char *output = json_object_to_json_string_ext(ir, flags);
	if (output == NULL) {
		rv = 1;
	} else {
		puts(output);
	}

	json_object_put(ir);
	json_object_put(model);

	return rv;
}
