/*
 * LSP Message Generator
 *
 * Reads metaModel.json and generates messages.h/messages.c containing:
 * - Request/notification enums and name tables
 * - Request/notification creation and reading functions
 * - Callback typedefs and structs
 * - Dispatch functions
 */

#include "common.h"

#define emit_header(ctx, ...) fprintf((ctx)->hdr, __VA_ARGS__)
#define emit_source(ctx, ...) fprintf((ctx)->src, __VA_ARGS__)

struct GenCtx {
	json_object *model;
	FILE *hdr;
	FILE *src;
};

/*
 * Message kind configuration - unifies notification and request generation
 */
struct MsgKind {
	const char *type_name;     /* "Notification" or "Method" */
	const char *type_lower;    /* "notification" or "method" */
	const char *enum_prefix;   /* "NOTIFICATION" or "METHOD" */
	bool has_response;         /* false for notifications, true for requests */
};

static const struct MsgKind MSG_NOTIFICATION = {
	"Notification", "notification", "NOTIFICATION", false
};

static const struct MsgKind MSG_REQUEST = {
	"Method", "method", "METHOD", true
};

/*
 * JSON traversal helpers
 */

static json_object *
model_get_requests(struct GenCtx *ctx) {
	return json_get_array(ctx->model, "requests");
}

static json_object *
model_get_notifications(struct GenCtx *ctx) {
	return json_get_array(ctx->model, "notifications");
}

static const char *
msg_get_method(json_object *msg) {
	return json_get_string(msg, "method");
}

static const char *
msg_get_direction(json_object *msg) {
	return json_get_string(msg, "messageDirection");
}

static json_object *
msg_get_params(json_object *msg) {
	return json_get_object(msg, "params");
}

static json_object *
msg_get_result(json_object *msg) {
	return json_get_object(msg, "result");
}

static const char *
type_get_kind(json_object *type) {
	return json_get_string(type, "kind");
}

static const char *
type_get_name(json_object *type) {
	return json_get_string(type, "name");
}

/*
 * Direction filtering
 */

static bool
is_c2s(const char *direction) {
	return direction && strcmp(direction, "clientToServer") == 0;
}

static bool
is_s2c(const char *direction) {
	return direction && strcmp(direction, "serverToClient") == 0;
}

static bool
is_both(const char *direction) {
	return direction && strcmp(direction, "both") == 0;
}

static bool
msg_matches_c2s(json_object *msg) {
	const char *dir = msg_get_direction(msg);
	return is_c2s(dir) || is_both(dir);
}

static bool
msg_matches_s2c(json_object *msg) {
	const char *dir = msg_get_direction(msg);
	return is_s2c(dir) || is_both(dir);
}

/*
 * Sorting utilities
 */

static int
msg_cmp_by_method(const void *a, const void *b) {
	json_object *const *ma = a;
	json_object *const *mb = b;
	const char *method_a = msg_get_method(*ma);
	const char *method_b = msg_get_method(*mb);
	return strcmp(method_a, method_b);
}

static json_object **
get_sorted_array(json_object *arr, size_t *out_count) {
	size_t n = json_array_len(arr);
	json_object **sorted = calloc(n, sizeof(json_object *));
	assert(sorted != NULL);

	for (size_t i = 0; i < n; i++) {
		sorted[i] = json_array_get(arr, i);
	}
	qsort(sorted, n, sizeof(json_object *), msg_cmp_by_method);
	*out_count = n;
	return sorted;
}

static json_object **
filter_by_direction(json_object **arr, size_t count, size_t *out_count,
                    bool (*matches)(json_object *)) {
	json_object **filtered = calloc(count, sizeof(json_object *));
	assert(filtered != NULL);

	size_t j = 0;
	for (size_t i = 0; i < count; i++) {
		if (matches(arr[i])) {
			filtered[j++] = arr[i];
		}
	}
	*out_count = j;
	return filtered;
}

/*
 * Type helpers
 */

static const char *
get_params_type_name(json_object *msg) {
	json_object *params = msg_get_params(msg);
	if (params == NULL) {
		return NULL;
	}
	const char *kind = type_get_kind(params);
	if (kind && strcmp(kind, "reference") == 0) {
		return type_get_name(params);
	}
	return NULL;
}

static bool
is_reference_result(json_object *result) {
	if (result == NULL) return false;
	const char *kind = type_get_kind(result);
	return kind && strcmp(kind, "reference") == 0;
}

static bool
is_null_result(json_object *result) {
	if (result == NULL) return false;
	const char *kind = type_get_kind(result);
	if (kind && strcmp(kind, "base") == 0) {
		const char *name = type_get_name(result);
		return name && strcmp(name, "null") == 0;
	}
	return false;
}

/*
 * Handler name generation
 */

static char *
method_to_handler_name(const char *method) {
	char *func_prefix = method_to_func_prefix(method);
	size_t len = strlen(func_prefix);
	char *result = calloc(len + 16, 1);
	assert(result != NULL);

	strcpy(result, "Lsp");
	char *dst = result + 3;
	bool cap_next = true;

	for (size_t i = 0; i < len; i++) {
		if (func_prefix[i] == '_') {
			cap_next = true;
		} else {
			*dst++ = cap_next ? (char)toupper(func_prefix[i]) : func_prefix[i];
			cap_next = false;
		}
	}
	strcpy(dst, "Handler");
	free(func_prefix);
	return result;
}

/*
 * Unified message generation
 */

static void
gen_message_enum(struct GenCtx *ctx, const struct MsgKind *kind,
                 json_object **sorted, size_t count, const char *prefix) {
	emit_header(ctx, "enum Lsp%s%s {\n", prefix, kind->type_name);
	emit_header(ctx, "\tLSP_%s_%s_UNKNOWN = 0,\n", prefix, kind->enum_prefix);

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		char *suffix = method_to_enum_suffix(method);
		emit_header(ctx, "\tLSP_%s_%s_%s,\n", prefix, kind->enum_prefix, suffix);
		free(suffix);
	}

	emit_header(ctx, "\tLSP_%s_%s_COUNT\n", prefix, kind->enum_prefix);
	emit_header(ctx, "};\n\n");
}

static void
gen_message_table_decl(struct GenCtx *ctx, const struct MsgKind *kind,
                       const char *prefix, const char *prefix_lower) {
	emit_header(ctx, "extern const char *lsp_%s_%s_names"
	            "[LSP_%s_%s_COUNT];\n",
	            prefix_lower, kind->type_lower, prefix, kind->enum_prefix);
}

static void
gen_message_table_impl(struct GenCtx *ctx, const struct MsgKind *kind,
                       json_object **sorted, size_t count,
                       const char *prefix, const char *prefix_lower) {
	emit_source(ctx, "const char *lsp_%s_%s_names"
	            "[LSP_%s_%s_COUNT] = {\n",
	            prefix_lower, kind->type_lower, prefix, kind->enum_prefix);
	emit_source(ctx, "\t[LSP_%s_%s_UNKNOWN] = NULL,\n", prefix, kind->enum_prefix);

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		char *suffix = method_to_enum_suffix(method);
		emit_source(ctx, "\t[LSP_%s_%s_%s] = \"%s\",\n",
		            prefix, kind->enum_prefix, suffix, method);
		free(suffix);
	}

	emit_source(ctx, "};\n\n");
}

static void
gen_message_lookup_decl(struct GenCtx *ctx, const struct MsgKind *kind,
                        const char *prefix, const char *prefix_lower) {
	emit_header(ctx, "enum Lsp%s%s lsp_%s_%s_lookup"
	            "(const char *method);\n",
	            prefix, kind->type_name, prefix_lower, kind->type_lower);
}

static void
gen_message_lookup_impl(struct GenCtx *ctx, const struct MsgKind *kind,
                        json_object **sorted, size_t count,
                        const char *prefix, const char *prefix_lower) {
	emit_source(ctx, "enum Lsp%s%s lsp_%s_%s_lookup"
	            "(const char *method) {\n",
	            prefix, kind->type_name, prefix_lower, kind->type_lower);
	emit_source(ctx, "\tif (method == NULL) return LSP_%s_%s_UNKNOWN;\n",
	            prefix, kind->enum_prefix);

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		char *suffix = method_to_enum_suffix(method);
		emit_source(ctx, "\tif (strcmp(method, \"%s\") == 0) "
		            "return LSP_%s_%s_%s;\n",
		            method, prefix, kind->enum_prefix, suffix);
		free(suffix);
	}

	emit_source(ctx, "\treturn LSP_%s_%s_UNKNOWN;\n", prefix, kind->enum_prefix);
	emit_source(ctx, "}\n\n");
}

static void
gen_message_create_decl(struct GenCtx *ctx, const struct MsgKind *kind,
                        json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		if (kind->has_response) {
			emit_header(ctx, "json_object *lsp_%s__request"
			            "(const %s *params, json_object *id);\n",
			            func_prefix, params_type);
		} else {
			emit_header(ctx, "json_object *lsp_%s__notification"
			            "(const %s *params);\n",
			            func_prefix, params_type);
		}
		free(params_type);
	} else {
		if (kind->has_response) {
			emit_header(ctx, "json_object *lsp_%s__request(json_object *id);\n",
			            func_prefix);
		} else {
			emit_header(ctx, "json_object *lsp_%s__notification(void);\n",
			            func_prefix);
		}
	}

	free(func_prefix);
}

static void
gen_message_create_impl(struct GenCtx *ctx, const struct MsgKind *kind,
                        json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		if (kind->has_response) {
			emit_source(ctx, "json_object *lsp_%s__request"
			            "(const %s *params, json_object *id) {\n",
			            func_prefix, params_type);
			emit_source(ctx, "\tjson_object *p = (params != NULL) ? "
			            "json_object_get(params->json) : NULL;\n");
			emit_source(ctx, "\treturn lsp_request_new(\"%s\", p, id);\n",
			            method);
		} else {
			emit_source(ctx, "json_object *lsp_%s__notification"
			            "(const %s *params) {\n",
			            func_prefix, params_type);
			emit_source(ctx, "\tjson_object *p = (params != NULL) ? "
			            "json_object_get(params->json) : NULL;\n");
			emit_source(ctx, "\treturn lsp_notification_new(\"%s\", p);\n",
			            method);
		}
		emit_source(ctx, "}\n\n");
		free(params_type);
	} else {
		if (kind->has_response) {
			emit_source(ctx, "json_object *lsp_%s__request(json_object *id) {\n",
			            func_prefix);
			emit_source(ctx, "\treturn lsp_request_new(\"%s\", NULL, id);\n",
			            method);
		} else {
			emit_source(ctx, "json_object *lsp_%s__notification(void) {\n",
			            func_prefix);
			emit_source(ctx, "\treturn lsp_notification_new(\"%s\", NULL);\n",
			            method);
		}
		emit_source(ctx, "}\n\n");
	}

	free(func_prefix);
}

static void
gen_message_read_decl(struct GenCtx *ctx, const struct MsgKind *kind,
                      json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		if (kind->has_response) {
			emit_header(ctx, "int lsp_%s__read_request"
			            "(json_object *req, json_object **id, %s *params);\n",
			            func_prefix, params_type);
		} else {
			emit_header(ctx, "int lsp_%s__read_notification"
			            "(json_object *notif, %s *params);\n",
			            func_prefix, params_type);
		}
		free(params_type);
	} else if (kind->has_response) {
		emit_header(ctx, "json_object *lsp_%s__read_request(json_object *req);\n",
		            func_prefix);
	}

	free(func_prefix);
}

static void
gen_message_read_impl(struct GenCtx *ctx, const struct MsgKind *kind,
                      json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		char *params_func = to_func_name(params_name);
		if (kind->has_response) {
			emit_source(ctx, "int lsp_%s__read_request"
			            "(json_object *req, json_object **id, %s *params) {\n",
			            func_prefix, params_type);
			emit_source(ctx, "\t*id = json_object_object_get(req, \"id\");\n");
			emit_source(ctx, "\tjson_object *p = json_object_object_get"
			            "(req, \"params\");\n");
		} else {
			emit_source(ctx, "int lsp_%s__read_notification"
			            "(json_object *notif, %s *params) {\n",
			            func_prefix, params_type);
			emit_source(ctx, "\tjson_object *p = json_object_object_get"
			            "(notif, \"params\");\n");
		}
		emit_source(ctx, "\treturn lsp_%s__from_json(params, p);\n",
		            params_func);
		emit_source(ctx, "}\n\n");
		free(params_func);
		free(params_type);
	} else if (kind->has_response) {
		emit_source(ctx, "json_object *lsp_%s__read_request(json_object *req) {\n",
		            func_prefix);
		emit_source(ctx, "\treturn json_object_object_get(req, \"id\");\n");
		emit_source(ctx, "}\n\n");
	}

	free(func_prefix);
}

static void
gen_notification_send_decl(struct GenCtx *ctx, json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		emit_header(ctx, "int lsp_%s__send"
		            "(const %s *params, struct Lsp *lsp);\n",
		            func_prefix, params_type);
		free(params_type);
	} else {
		emit_header(ctx, "int lsp_%s__send(struct Lsp *lsp);\n", func_prefix);
	}

	free(func_prefix);
}

static void
gen_notification_send_impl(struct GenCtx *ctx, json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *func_prefix = method_to_func_prefix(method);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		emit_source(ctx, "int lsp_%s__send"
		            "(const %s *params, struct Lsp *lsp) {\n",
		            func_prefix, params_type);
		emit_source(ctx, "\treturn lsp_notify(lsp, lsp_%s__notification(params));\n",
		            func_prefix);
		free(params_type);
	} else {
		emit_source(ctx, "int lsp_%s__send(struct Lsp *lsp) {\n", func_prefix);
		emit_source(ctx, "\treturn lsp_notify(lsp, lsp_%s__notification());\n",
		            func_prefix);
	}
	emit_source(ctx, "}\n\n");

	free(func_prefix);
}

static void
gen_message_callback_typedef(struct GenCtx *ctx, const struct MsgKind *kind,
                             json_object *msg) {
	const char *method = msg_get_method(msg);
	const char *params_name = get_params_type_name(msg);
	char *handler_name = method_to_handler_name(method);

	emit_header(ctx, "typedef int (*%s)(", handler_name);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		emit_header(ctx, "%s *params, ", params_type);
		free(params_type);
	}

	if (kind->has_response) {
		json_object *result = msg_get_result(msg);
		if (result != NULL && !is_null_result(result) && is_reference_result(result)) {
			const char *ref_name = type_get_name(result);
			char *result_type = to_struct_type(ref_name);
			emit_header(ctx, "%s *result, ", result_type);
			free(result_type);
		}
		emit_header(ctx, "struct LspResponseError *error, ");
	}

	emit_header(ctx, "void *userdata);\n");

	free(handler_name);
}

static void
gen_message_callback_struct(struct GenCtx *ctx, json_object **sorted,
                            size_t count, const char *struct_name) {
	emit_header(ctx, "struct %s {\n", struct_name);

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		char *handler_name = method_to_handler_name(method);
		char *field_name = method_to_func_prefix(method);
		emit_header(ctx, "\t%s %s;\n", handler_name, field_name);
		free(field_name);
		free(handler_name);
	}

	emit_header(ctx, "};\n\n");
}

static void
gen_notification_dispatch_decl(struct GenCtx *ctx, const char *prefix,
                               const char *prefix_lower,
                               const char *struct_name) {
	emit_header(ctx, "int lsp_%s_dispatch_notification(\n", prefix_lower);
	emit_header(ctx, "\tconst struct %s *callbacks,\n", struct_name);
	emit_header(ctx, "\tenum Lsp%sNotification notification,\n", prefix);
	emit_header(ctx, "\tjson_object *notif,\n");
	emit_header(ctx, "\tvoid *userdata);\n\n");
}

static void
gen_notification_dispatch_impl(struct GenCtx *ctx, json_object **sorted,
                               size_t count, const char *prefix,
                               const char *prefix_lower,
                               const char *struct_name) {
	emit_source(ctx, "int lsp_%s_dispatch_notification(\n", prefix_lower);
	emit_source(ctx, "\tconst struct %s *callbacks,\n", struct_name);
	emit_source(ctx, "\tenum Lsp%sNotification notification,\n", prefix);
	emit_source(ctx, "\tjson_object *notif,\n");
	emit_source(ctx, "\tvoid *userdata) {\n");

	emit_source(ctx, "\tint err = 0;\n");
	emit_source(ctx, "\tswitch (notification) {\n");

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		const char *params_name = get_params_type_name(sorted[i]);
		char *suffix = method_to_enum_suffix(method);
		char *field_name = method_to_func_prefix(method);

		emit_source(ctx, "\tcase LSP_%s_NOTIFICATION_%s: {\n", prefix, suffix);
		emit_source(ctx, "\t\tif (callbacks->%s == NULL) return 0;\n",
		            field_name);

		if (params_name != NULL) {
			char *params_type = to_struct_type(params_name);
			char *params_func = to_func_name(params_name);
			emit_source(ctx, "\t\t%s params = {0};\n", params_type);
			emit_source(ctx, "\t\tjson_object *p = json_object_object_get"
			            "(notif, \"params\");\n");
			emit_source(ctx, "\t\terr = lsp_%s__from_json(&params, p);\n",
			            params_func);
			emit_source(ctx, "\t\tif (err != 0) return err;\n");
			emit_source(ctx, "\t\terr = callbacks->%s(&params, userdata);\n",
			            field_name);
			emit_source(ctx, "\t\tlsp_%s__cleanup(&params);\n", params_func);
			free(params_func);
			free(params_type);
		} else {
			emit_source(ctx, "\t\terr = callbacks->%s(userdata);\n", field_name);
		}

		emit_source(ctx, "\t\treturn err;\n");
		emit_source(ctx, "\t}\n");

		free(field_name);
		free(suffix);
	}

	emit_source(ctx, "\tcase LSP_%s_NOTIFICATION_UNKNOWN:\n", prefix);
	emit_source(ctx, "\tdefault:\n");
	emit_source(ctx, "\t\treturn -1;\n");
	emit_source(ctx, "\t}\n");
	emit_source(ctx, "}\n\n");
}

static void
gen_request_dispatch_decl(struct GenCtx *ctx, const char *prefix,
                          const char *prefix_lower, const char *struct_name) {
	emit_header(ctx, "json_object *lsp_%s_dispatch_request(\n", prefix_lower);
	emit_header(ctx, "\tconst struct %s *callbacks,\n", struct_name);
	emit_header(ctx, "\tenum Lsp%sMethod method,\n", prefix);
	emit_header(ctx, "\tjson_object *request,\n");
	emit_header(ctx, "\tvoid *userdata);\n\n");
}

static void
gen_request_dispatch_impl(struct GenCtx *ctx, json_object **sorted, size_t count,
                          const char *prefix, const char *prefix_lower,
                          const char *struct_name) {
	emit_source(ctx, "json_object *lsp_%s_dispatch_request(\n", prefix_lower);
	emit_source(ctx, "\tconst struct %s *callbacks,\n", struct_name);
	emit_source(ctx, "\tenum Lsp%sMethod method,\n", prefix);
	emit_source(ctx, "\tjson_object *request,\n");
	emit_source(ctx, "\tvoid *userdata) {\n");

	emit_source(ctx, "\tjson_object *id = json_object_object_get"
	            "(request, \"id\");\n");
	emit_source(ctx, "\tif (id == NULL) return NULL;\n\n");

	emit_source(ctx, "\tjson_object *response = json_object_new_object();\n");
	emit_source(ctx, "\tjson_object_object_add(response, \"jsonrpc\", "
	            "json_object_new_string(\"2.0\"));\n");
	emit_source(ctx, "\tjson_object_object_add(response, \"id\", "
	            "json_object_get(id));\n\n");

	emit_source(ctx, "\tint err = 0;\n");
	emit_source(ctx, "\tswitch (method) {\n");

	for (size_t i = 0; i < count; i++) {
		const char *method = msg_get_method(sorted[i]);
		const char *params_name = get_params_type_name(sorted[i]);
		json_object *result = msg_get_result(sorted[i]);
		char *suffix = method_to_enum_suffix(method);
		char *field_name = method_to_func_prefix(method);

		emit_source(ctx, "\tcase LSP_%s_METHOD_%s: {\n", prefix, suffix);
		emit_source(ctx, "\t\tif (callbacks->%s == NULL) { "
		            "json_object_put(response); return NULL; }\n",
		            field_name);

		/* Parse params */
		if (params_name != NULL) {
			char *params_type = to_struct_type(params_name);
			char *params_func = to_func_name(params_name);
			emit_source(ctx, "\t\t%s params = {0};\n", params_type);
			emit_source(ctx, "\t\tjson_object *p = json_object_object_get"
			            "(request, \"params\");\n");
			emit_source(ctx, "\t\terr = lsp_%s__from_json(&params, p);\n",
			            params_func);
			emit_source(ctx, "\t\tif (err != 0) {\n");
			emit_source(ctx, "\t\t\tjson_object *error_obj = json_object_new_object();\n");
			emit_source(ctx, "\t\t\tjson_object_object_add(error_obj, \"code\", "
			            "json_object_new_int(-32602));\n");
			emit_source(ctx, "\t\t\tjson_object_object_add(error_obj, \"message\", "
			            "json_object_new_string(\"Invalid params\"));\n");
			emit_source(ctx, "\t\t\tjson_object_object_add(response, \"error\", error_obj);\n");
			emit_source(ctx, "\t\t\treturn response;\n");
			emit_source(ctx, "\t\t}\n");
			free(params_func);
			free(params_type);
		}

		/* Prepare result */
		bool has_result = result != NULL && !is_null_result(result) &&
		                  is_reference_result(result);
		char *result_type = NULL;
		char *result_func = NULL;

		if (has_result) {
			const char *ref_name = type_get_name(result);
			result_type = to_struct_type(ref_name);
			result_func = to_func_name(ref_name);
			emit_source(ctx, "\t\t%s result = {0};\n", result_type);
			emit_source(ctx, "\t\terr = lsp_%s__init(&result);\n", result_func);
			emit_source(ctx, "\t\tif (err < 0) {\n");
			if (params_name != NULL) {
				char *params_func = to_func_name(params_name);
				emit_source(ctx, "\t\t\tlsp_%s__cleanup(&params);\n", params_func);
				free(params_func);
			}
			emit_source(ctx, "\t\t\tjson_object_put(response);\n");
			emit_source(ctx, "\t\t\treturn NULL;\n");
			emit_source(ctx, "\t\t}\n");
		}

		/* Error object */
		emit_source(ctx, "\t\tstruct LspResponseError error = {0};\n");
		emit_source(ctx, "\t\terr = lsp_response_error__init(&error);\n");
		emit_source(ctx, "\t\tif (err < 0) {\n");
		if (has_result) {
			emit_source(ctx, "\t\t\tlsp_%s__cleanup(&result);\n", result_func);
		}
		if (params_name != NULL) {
			char *params_func = to_func_name(params_name);
			emit_source(ctx, "\t\t\tlsp_%s__cleanup(&params);\n", params_func);
			free(params_func);
		}
		emit_source(ctx, "\t\t\tjson_object_put(response);\n");
		emit_source(ctx, "\t\t\treturn NULL;\n");
		emit_source(ctx, "\t\t}\n");

		/* Call handler */
		emit_source(ctx, "\t\terr = callbacks->%s(", field_name);
		if (params_name != NULL) {
			emit_source(ctx, "&params, ");
		}
		if (has_result) {
			emit_source(ctx, "&result, ");
		}
		emit_source(ctx, "&error, userdata);\n");

		/* Handle error */
		emit_source(ctx, "\t\tif (err < 0) {\n");
		if (has_result) {
			emit_source(ctx, "\t\t\tlsp_%s__cleanup(&result);\n", result_func);
		}
		emit_source(ctx, "\t\t\tif (error.json != NULL) {\n");
		emit_source(ctx, "\t\t\t\tjson_object_object_add(response, \"error\", "
		            "json_object_get(error.json));\n");
		emit_source(ctx, "\t\t\t} else {\n");
		emit_source(ctx, "\t\t\t\tjson_object *error_obj = json_object_new_object();\n");
		emit_source(ctx, "\t\t\t\tjson_object_object_add(error_obj, \"code\", "
		            "json_object_new_int(-32603));\n");
		emit_source(ctx, "\t\t\t\tjson_object_object_add(error_obj, \"message\", "
		            "json_object_new_string(\"Internal error\"));\n");
		emit_source(ctx, "\t\t\t\tjson_object_object_add(response, \"error\", error_obj);\n");
		emit_source(ctx, "\t\t\t}\n");
		emit_source(ctx, "\t\t\tlsp_response_error__cleanup(&error);\n");
		if (params_name != NULL) {
			char *params_func = to_func_name(params_name);
			emit_source(ctx, "\t\t\tlsp_%s__cleanup(&params);\n", params_func);
			free(params_func);
		}
		emit_source(ctx, "\t\t\treturn response;\n");
		emit_source(ctx, "\t\t}\n");

		/* Success path */
		if (has_result) {
			emit_source(ctx, "\t\tif (result.json != NULL) {\n");
			emit_source(ctx, "\t\t\tjson_object_object_add(response, \"result\", "
			            "json_object_get(result.json));\n");
			emit_source(ctx, "\t\t} else {\n");
			emit_source(ctx, "\t\t\tjson_object_object_add(response, \"result\", NULL);\n");
			emit_source(ctx, "\t\t}\n");
			emit_source(ctx, "\t\tlsp_%s__cleanup(&result);\n", result_func);
		} else {
			emit_source(ctx, "\t\tjson_object_object_add(response, \"result\", NULL);\n");
		}

		emit_source(ctx, "\t\tlsp_response_error__cleanup(&error);\n");
		if (params_name != NULL) {
			char *params_func = to_func_name(params_name);
			emit_source(ctx, "\t\tlsp_%s__cleanup(&params);\n", params_func);
			free(params_func);
		}
		emit_source(ctx, "\t\tbreak;\n");
		emit_source(ctx, "\t}\n");

		free(result_func);
		free(result_type);
		free(field_name);
		free(suffix);
	}

	emit_source(ctx, "\tcase LSP_%s_METHOD_UNKNOWN:\n", prefix);
	emit_source(ctx, "\tdefault:\n");
	emit_source(ctx, "\t\tjson_object_put(response);\n");
	emit_source(ctx, "\t\treturn NULL;\n");
	emit_source(ctx, "\t}\n");

	emit_source(ctx, "\treturn response;\n");
	emit_source(ctx, "}\n\n");
}

/* Request-only: result reading functions */
static void
gen_request_result_read_decl(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	json_object *result = msg_get_result(req);
	char *func_prefix = method_to_func_prefix(method);

	if (result == NULL || is_null_result(result) || !is_reference_result(result)) {
		free(func_prefix);
		return;
	}

	const char *ref_name = type_get_name(result);
	char *result_type = to_struct_type(ref_name);
	emit_header(ctx, "int lsp_%s__read_result"
	            "(json_object *resp, %s *result);\n",
	            func_prefix, result_type);
	free(result_type);

	free(func_prefix);
}

static void
gen_request_result_read_impl(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	json_object *result = msg_get_result(req);
	char *func_prefix = method_to_func_prefix(method);

	if (result == NULL || is_null_result(result) || !is_reference_result(result)) {
		free(func_prefix);
		return;
	}

	const char *ref_name = type_get_name(result);
	char *result_type = to_struct_type(ref_name);
	char *result_func = to_func_name(ref_name);
	emit_source(ctx, "int lsp_%s__read_result"
	            "(json_object *resp, %s *result) {\n",
	            func_prefix, result_type);
	emit_source(ctx, "\tjson_object *r = json_object_object_get"
	            "(resp, \"result\");\n");
	emit_source(ctx, "\treturn lsp_%s__from_json(result, r);\n", result_func);
	emit_source(ctx, "}\n\n");
	free(result_func);
	free(result_type);

	free(func_prefix);
}

/* Request-only: response callback typedef */
static void
gen_request_response_callback_typedef(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	json_object *result = msg_get_result(req);
	char *handler_name = method_to_handler_name(method);

	/* Change "Handler" suffix to "ResponseCallback" */
	size_t len = strlen(handler_name);
	char *cb_name = malloc(len + 16);
	assert(cb_name != NULL);
	strcpy(cb_name, handler_name);
	strcpy(cb_name + len - 7, "ResponseCallback"); /* Replace "Handler" */

	emit_header(ctx, "typedef void (*%s)(", cb_name);

	/* Result parameter (if any) */
	if (result != NULL && !is_null_result(result) && is_reference_result(result)) {
		const char *ref_name = type_get_name(result);
		char *result_type = to_struct_type(ref_name);
		emit_header(ctx, "%s *result, ", result_type);
		free(result_type);
	}

	emit_header(ctx, "struct LspResponseError *error, void *userdata);\n");

	free(cb_name);
	free(handler_name);
}

/* Request-only: response wrapper function (static in .c) */
static void
gen_request_response_wrapper(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	json_object *result = msg_get_result(req);
	char *func_prefix = method_to_func_prefix(method);
	char *handler_name = method_to_handler_name(method);

	/* Build callback type name */
	size_t len = strlen(handler_name);
	char *cb_name = malloc(len + 16);
	assert(cb_name != NULL);
	strcpy(cb_name, handler_name);
	strcpy(cb_name + len - 7, "ResponseCallback");

	bool has_result = result != NULL && !is_null_result(result) &&
	                  is_reference_result(result);

	emit_source(ctx, "static void\n");
	emit_source(ctx, "lsp_%s__response_wrapper("
	            "json_object *response, void *callback, void *userdata) {\n",
	            func_prefix);
	emit_source(ctx, "\t%s cb = (%s)callback;\n", cb_name, cb_name);

	/* Check for error in response */
	emit_source(ctx, "\tjson_object *error_obj = "
	            "json_object_object_get(response, \"error\");\n");
	emit_source(ctx, "\tif (error_obj != NULL) {\n");
	emit_source(ctx, "\t\tstruct LspResponseError error = {0};\n");
	emit_source(ctx, "\t\terror.json = json_object_get(error_obj);\n");
	if (has_result) {
		emit_source(ctx, "\t\tcb(NULL, &error, userdata);\n");
	} else {
		emit_source(ctx, "\t\tcb(&error, userdata);\n");
	}
	emit_source(ctx, "\t\tlsp_response_error__cleanup(&error);\n");
	emit_source(ctx, "\t\treturn;\n");
	emit_source(ctx, "\t}\n");

	/* Parse result if any */
	if (has_result) {
		const char *ref_name = type_get_name(result);
		char *result_type = to_struct_type(ref_name);
		char *result_func = to_func_name(ref_name);

		emit_source(ctx, "\t%s result = {0};\n", result_type);
		emit_source(ctx, "\tint rv = lsp_%s__read_result(response, &result);\n",
		            func_prefix);
		emit_source(ctx, "\tif (rv != 0) {\n");
		emit_source(ctx, "\t\tcb(NULL, NULL, userdata);\n");
		emit_source(ctx, "\t\treturn;\n");
		emit_source(ctx, "\t}\n");
		emit_source(ctx, "\tcb(&result, NULL, userdata);\n");
		emit_source(ctx, "\tlsp_%s__cleanup(&result);\n", result_func);

		free(result_func);
		free(result_type);
	} else {
		emit_source(ctx, "\tcb(NULL, userdata);\n");
	}

	emit_source(ctx, "}\n\n");

	free(cb_name);
	free(handler_name);
	free(func_prefix);
}

/* Request-only: send function declaration */
static void
gen_request_send_decl(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	const char *params_name = get_params_type_name(req);
	char *func_prefix = method_to_func_prefix(method);
	char *handler_name = method_to_handler_name(method);

	/* Build callback type name */
	size_t len = strlen(handler_name);
	char *cb_name = malloc(len + 16);
	assert(cb_name != NULL);
	strcpy(cb_name, handler_name);
	strcpy(cb_name + len - 7, "ResponseCallback");

	emit_header(ctx, "int lsp_%s__send(", func_prefix);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		emit_header(ctx, "const %s *params, ", params_type);
		free(params_type);
	}

	emit_header(ctx, "struct Lsp *lsp, %s callback, void *userdata);\n",
	            cb_name);

	free(cb_name);
	free(handler_name);
	free(func_prefix);
}

/* Request-only: send function implementation */
static void
gen_request_send_impl(struct GenCtx *ctx, json_object *req) {
	const char *method = msg_get_method(req);
	const char *params_name = get_params_type_name(req);
	char *func_prefix = method_to_func_prefix(method);
	char *handler_name = method_to_handler_name(method);

	/* Build callback type name */
	size_t len = strlen(handler_name);
	char *cb_name = malloc(len + 16);
	assert(cb_name != NULL);
	strcpy(cb_name, handler_name);
	strcpy(cb_name + len - 7, "ResponseCallback");

	emit_source(ctx, "int lsp_%s__send(", func_prefix);

	if (params_name != NULL) {
		char *params_type = to_struct_type(params_name);
		emit_source(ctx, "const %s *params, ", params_type);
		free(params_type);
	}

	emit_source(ctx, "struct Lsp *lsp, %s callback, void *userdata) {\n",
	            cb_name);

	/* Generate ID and create request */
	emit_source(ctx, "\tjson_object *id = "
	            "json_object_new_int64((int64_t)lsp_next_id(lsp));\n");
	if (params_name != NULL) {
		emit_source(ctx, "\tjson_object *req = lsp_%s__request(params, id);\n",
		            func_prefix);
	} else {
		emit_source(ctx, "\tjson_object *req = lsp_%s__request(id);\n",
		            func_prefix);
	}
	emit_source(ctx, "\tjson_object_put(id);\n");
	emit_source(ctx, "\treturn lsp_request(lsp, req, "
	            "lsp_%s__response_wrapper, callback, userdata);\n",
	            func_prefix);
	emit_source(ctx, "}\n\n");

	free(cb_name);
	free(handler_name);
	free(func_prefix);
}

/*
 * Main generation
 */

static void
emit_header_prologue(struct GenCtx *ctx) {
	emit_header(ctx, "/* Generated by gen/msg_gen - do not edit */\n");
	emit_header(ctx, "#ifndef LSP_MESSAGES_H\n");
	emit_header(ctx, "#define LSP_MESSAGES_H\n\n");
	emit_header(ctx, "#include <json.h>\n");
	emit_header(ctx, "#include <stdbool.h>\n");
	emit_header(ctx, "#include <stddef.h>\n");
	emit_header(ctx, "#include <stdint.h>\n");
	emit_header(ctx, "#include \"model.h\"\n\n");
	emit_header(ctx, "/* Forward declarations */\n");
	emit_header(ctx, "struct LspResponseError;\n");
	emit_header(ctx, "struct Lsp;\n\n");
}

static void
emit_header_epilogue(struct GenCtx *ctx) {
	emit_header(ctx, "#endif /* LSP_MESSAGES_H */\n");
}

static void
emit_source_prologue(struct GenCtx *ctx) {
	emit_source(ctx, "/* Generated by gen/msg_gen - do not edit */\n");
	emit_source(ctx, "#include \"messages.h\"\n");
	emit_source(ctx, "#include \"lsp.h\"\n");
	emit_source(ctx, "#include \"lsp_util.h\"\n");
	emit_source(ctx, "#include <string.h>\n\n");
}

static void
gen_all_notifications(struct GenCtx *ctx) {
	json_object *notifications = model_get_notifications(ctx);
	if (notifications == NULL) {
		return;
	}

	size_t count;
	json_object **sorted = get_sorted_array(notifications, &count);

	size_t c2s_count, s2c_count;
	json_object **c2s = filter_by_direction(sorted, count, &c2s_count,
	                                         msg_matches_c2s);
	json_object **s2c = filter_by_direction(sorted, count, &s2c_count,
	                                         msg_matches_s2c);

	/* Header: enums */
	emit_header(ctx, "/* Notification enums */\n\n");
	gen_message_enum(ctx, &MSG_NOTIFICATION, c2s, c2s_count, "C2S");
	gen_message_enum(ctx, &MSG_NOTIFICATION, s2c, s2c_count, "S2C");

	/* Header: table declarations */
	emit_header(ctx, "/* Notification name tables */\n\n");
	gen_message_table_decl(ctx, &MSG_NOTIFICATION, "C2S", "c2s");
	gen_message_table_decl(ctx, &MSG_NOTIFICATION, "S2C", "s2c");
	emit_header(ctx, "\n");

	/* Header: lookup declarations */
	emit_header(ctx, "/* Notification lookup functions */\n\n");
	gen_message_lookup_decl(ctx, &MSG_NOTIFICATION, "C2S", "c2s");
	gen_message_lookup_decl(ctx, &MSG_NOTIFICATION, "S2C", "s2c");
	emit_header(ctx, "\n");

	/* Header: create/read declarations */
	emit_header(ctx, "/* Notification creation functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_create_decl(ctx, &MSG_NOTIFICATION, sorted[i]);
	}
	emit_header(ctx, "\n");

	emit_header(ctx, "/* Notification reading functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_read_decl(ctx, &MSG_NOTIFICATION, sorted[i]);
	}
	emit_header(ctx, "\n");

	emit_header(ctx, "/* Notification send functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_notification_send_decl(ctx, sorted[i]);
	}
	emit_header(ctx, "\n");

	/* Header: callback typedefs */
	emit_header(ctx, "/* Notification callback typedefs */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_callback_typedef(ctx, &MSG_NOTIFICATION, sorted[i]);
	}
	emit_header(ctx, "\n");

	/* Header: callback structs */
	emit_header(ctx, "/* Notification callback structs */\n\n");
	gen_message_callback_struct(ctx, c2s, c2s_count,
	                            "LspServerNotificationCallbacks");
	gen_message_callback_struct(ctx, s2c, s2c_count,
	                            "LspClientNotificationCallbacks");

	/* Header: dispatch declarations */
	emit_header(ctx, "/* Notification dispatch functions */\n\n");
	gen_notification_dispatch_decl(ctx, "C2S", "server",
	                               "LspServerNotificationCallbacks");
	gen_notification_dispatch_decl(ctx, "S2C", "client",
	                               "LspClientNotificationCallbacks");

	/* Source: tables */
	gen_message_table_impl(ctx, &MSG_NOTIFICATION, c2s, c2s_count, "C2S", "c2s");
	gen_message_table_impl(ctx, &MSG_NOTIFICATION, s2c, s2c_count, "S2C", "s2c");

	/* Source: lookup implementations */
	gen_message_lookup_impl(ctx, &MSG_NOTIFICATION, c2s, c2s_count, "C2S", "c2s");
	gen_message_lookup_impl(ctx, &MSG_NOTIFICATION, s2c, s2c_count, "S2C", "s2c");

	/* Source: create/read implementations */
	for (size_t i = 0; i < count; i++) {
		gen_message_create_impl(ctx, &MSG_NOTIFICATION, sorted[i]);
	}
	for (size_t i = 0; i < count; i++) {
		gen_message_read_impl(ctx, &MSG_NOTIFICATION, sorted[i]);
	}

	/* Source: dispatch implementations */
	gen_notification_dispatch_impl(ctx, c2s, c2s_count, "C2S", "server",
	                               "LspServerNotificationCallbacks");
	gen_notification_dispatch_impl(ctx, s2c, s2c_count, "S2C", "client",
	                               "LspClientNotificationCallbacks");

	/* Source: send implementations */
	for (size_t i = 0; i < count; i++) {
		gen_notification_send_impl(ctx, sorted[i]);
	}

	free(s2c);
	free(c2s);
	free(sorted);
}

static void
gen_all_requests(struct GenCtx *ctx) {
	json_object *requests = model_get_requests(ctx);
	if (requests == NULL) {
		return;
	}

	size_t count;
	json_object **sorted = get_sorted_array(requests, &count);

	size_t c2s_count, s2c_count;
	json_object **c2s = filter_by_direction(sorted, count, &c2s_count,
	                                         msg_matches_c2s);
	json_object **s2c = filter_by_direction(sorted, count, &s2c_count,
	                                         msg_matches_s2c);

	/* Header: enums */
	emit_header(ctx, "/* Request enums */\n\n");
	gen_message_enum(ctx, &MSG_REQUEST, c2s, c2s_count, "C2S");
	gen_message_enum(ctx, &MSG_REQUEST, s2c, s2c_count, "S2C");

	/* Header: table declarations */
	emit_header(ctx, "/* Request name tables */\n\n");
	gen_message_table_decl(ctx, &MSG_REQUEST, "C2S", "c2s");
	gen_message_table_decl(ctx, &MSG_REQUEST, "S2C", "s2c");
	emit_header(ctx, "\n");

	/* Header: lookup declarations */
	emit_header(ctx, "/* Request lookup functions */\n\n");
	gen_message_lookup_decl(ctx, &MSG_REQUEST, "C2S", "c2s");
	gen_message_lookup_decl(ctx, &MSG_REQUEST, "S2C", "s2c");
	emit_header(ctx, "\n");

	/* Header: create/read declarations */
	emit_header(ctx, "/* Request creation functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_create_decl(ctx, &MSG_REQUEST, sorted[i]);
	}
	emit_header(ctx, "\n");

	emit_header(ctx, "/* Request reading functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_read_decl(ctx, &MSG_REQUEST, sorted[i]);
	}
	emit_header(ctx, "\n");

	emit_header(ctx, "/* Result reading functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_request_result_read_decl(ctx, sorted[i]);
	}
	emit_header(ctx, "\n");

	emit_header(ctx, "/* Request send functions */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_request_response_callback_typedef(ctx, sorted[i]);
	}
	for (size_t i = 0; i < count; i++) {
		gen_request_send_decl(ctx, sorted[i]);
	}
	emit_header(ctx, "\n");

	/* Header: callback typedefs */
	emit_header(ctx, "/* Request callback typedefs */\n\n");
	for (size_t i = 0; i < count; i++) {
		gen_message_callback_typedef(ctx, &MSG_REQUEST, sorted[i]);
	}
	emit_header(ctx, "\n");

	/* Header: callback structs */
	emit_header(ctx, "/* Request callback structs */\n\n");
	gen_message_callback_struct(ctx, c2s, c2s_count, "LspServerRequestCallbacks");
	gen_message_callback_struct(ctx, s2c, s2c_count, "LspClientRequestCallbacks");

	/* Header: dispatch declarations */
	emit_header(ctx, "/* Request dispatch functions */\n\n");
	gen_request_dispatch_decl(ctx, "C2S", "server", "LspServerRequestCallbacks");
	gen_request_dispatch_decl(ctx, "S2C", "client", "LspClientRequestCallbacks");

	/* Source: tables */
	gen_message_table_impl(ctx, &MSG_REQUEST, c2s, c2s_count, "C2S", "c2s");
	gen_message_table_impl(ctx, &MSG_REQUEST, s2c, s2c_count, "S2C", "s2c");

	/* Source: lookup implementations */
	gen_message_lookup_impl(ctx, &MSG_REQUEST, c2s, c2s_count, "C2S", "c2s");
	gen_message_lookup_impl(ctx, &MSG_REQUEST, s2c, s2c_count, "S2C", "s2c");

	/* Source: create/read implementations */
	for (size_t i = 0; i < count; i++) {
		gen_message_create_impl(ctx, &MSG_REQUEST, sorted[i]);
	}
	for (size_t i = 0; i < count; i++) {
		gen_message_read_impl(ctx, &MSG_REQUEST, sorted[i]);
	}
	for (size_t i = 0; i < count; i++) {
		gen_request_result_read_impl(ctx, sorted[i]);
	}

	/* Source: dispatch implementations */
	gen_request_dispatch_impl(ctx, c2s, c2s_count, "C2S", "server",
	                          "LspServerRequestCallbacks");
	gen_request_dispatch_impl(ctx, s2c, s2c_count, "S2C", "client",
	                          "LspClientRequestCallbacks");

	/* Source: response wrappers and send implementations */
	for (size_t i = 0; i < count; i++) {
		gen_request_response_wrapper(ctx, sorted[i]);
	}
	for (size_t i = 0; i < count; i++) {
		gen_request_send_impl(ctx, sorted[i]);
	}

	free(s2c);
	free(c2s);
	free(sorted);
}

static void
usage(const char *prog) {
	fprintf(stderr, "Usage: %s <metaModel.json> <messages.h> <messages.c>\n",
	        prog);
	exit(1);
}

int
main(int argc, char **argv) {
	if (argc != 4) {
		usage(argv[0]);
	}

	const char *model_path = argv[1];
	const char *hdr_path = argv[2];
	const char *src_path = argv[3];

	json_object *model = json_object_from_file(model_path);
	if (model == NULL) {
		fprintf(stderr, "Failed to read %s\n", model_path);
		return 1;
	}

	FILE *hdr = fopen(hdr_path, "w");
	if (hdr == NULL) {
		fprintf(stderr, "Failed to open %s for writing\n", hdr_path);
		json_object_put(model);
		return 1;
	}

	FILE *src = fopen(src_path, "w");
	if (src == NULL) {
		fprintf(stderr, "Failed to open %s for writing\n", src_path);
		fclose(hdr);
		json_object_put(model);
		return 1;
	}

	struct GenCtx ctx = {
		.model = model,
		.hdr = hdr,
		.src = src,
	};

	emit_header_prologue(&ctx);
	emit_source_prologue(&ctx);

	gen_all_notifications(&ctx);
	gen_all_requests(&ctx);

	emit_header_epilogue(&ctx);

	fclose(src);
	fclose(hdr);
	json_object_put(model);

	return 0;
}
