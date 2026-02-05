#ifndef GEN3_COMMON_H
#define GEN3_COMMON_H

#include <assert.h>
#include <ctype.h>
#include <json.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * JSON utility inline functions
 */

static inline const char *
json_get_string(json_object *obj, const char *key) {
	json_object *val = json_object_object_get(obj, key);
	return val ? json_object_get_string(val) : NULL;
}

static inline json_object *
json_get_object(json_object *obj, const char *key) {
	return json_object_object_get(obj, key);
}

static inline json_object *
json_get_array(json_object *obj, const char *key) {
	json_object *val = json_object_object_get(obj, key);
	return (val && json_object_is_type(val, json_type_array)) ? val : NULL;
}

static inline size_t
json_array_len(json_object *arr) {
	return arr ? json_object_array_length(arr) : 0;
}

static inline json_object *
json_array_get(json_object *arr, size_t idx) {
	return arr ? json_object_array_get_idx(arr, idx) : NULL;
}

static inline bool
json_has_key(json_object *obj, const char *key) {
	return json_object_object_get_ex(obj, key, NULL);
}

static inline void
json_add_string(json_object *obj, const char *key, const char *value) {
	json_object_object_add(obj, key, json_object_new_string(value));
}

static inline void
json_add_bool(json_object *obj, const char *key, bool value) {
	json_object_object_add(obj, key, json_object_new_boolean(value));
}

static inline void
json_add_object(json_object *parent, const char *key, json_object *child) {
	json_object_object_add(parent, key, child);
}

static inline json_object *
json_new_object(void) {
	return json_object_new_object();
}

static inline json_object *
json_new_array(void) {
	return json_object_new_array();
}

static inline void
json_array_add(json_object *arr, json_object *elem) {
	json_object_array_add(arr, elem);
}

/*
 * String utility declarations
 */

// Convert CamelCase to snake_case
// Caller must free the result.
char *str_to_snake_case(const char *str);

// Convert CamelCase to UPPER_SNAKE_CASE
// Caller must free the result.
char *str_to_upper_snake(const char *str);

// Capitalize first letter of a string
// Caller must free the result.
char *str_capitalize(const char *str);

// Strip "LSP" prefix from type names to avoid redundant "LspLSP..." naming.
// Exception: Known collision cases keep the prefix (e.g., LSPErrorCodes).
// Does NOT allocate - returns pointer into original string or static buffer.
const char *lsp_name_normalize(const char *name);

// Convert LSP type name to full C struct type: "TextDocument" -> "struct LspTextDocument"
// Handles LSP-prefixed names: "LSPAny" -> "struct LspAny"
// Caller must free the result.
char *to_struct_type(const char *lsp_name);

// Convert LSP type name to full C enum type: "ErrorCodes" -> "enum LspErrorCodes"
// Caller must free the result.
char *to_enum_type(const char *lsp_name);

// Convert LSP type name to C function prefix: "TextDocument" -> "text_document"
// Handles LSP-prefixed names: "LSPAny" -> "any"
// Caller must free the result.
char *to_func_name(const char *lsp_name);

// Convert LSP type name to C constant prefix: "ErrorCodes" -> "ERROR_CODES"
// Handles LSP-prefixed names: "LSPErrorCodes" -> "LSP_ERROR_CODES"
// Caller must free the result.
char *to_constant_name(const char *lsp_name);

// Map LSP base type to C type string
// Returns NULL for unknown types
const char *base_to_c_type(const char *base_name);

// Map LSP base type to json-c suffix (for json_object_get_*/json_object_new_*)
// Returns NULL for unknown types
const char *base_type_json_suffix(const char *name);

// Map LSP base type to json_type_* constant
// Returns NULL for unknown types
const char *base_type_json_type(const char *name);

// Convert method name to enum suffix: "textDocument/references" -> "TEXT_DOCUMENT__REFERENCES"
// Handles $/ prefix: "$/progress" -> "PROGRESS"
// Caller must free the result.
char *method_to_enum_suffix(const char *method);

// Convert method name to function prefix: "textDocument/references" -> "text_document__references"
// Handles $/ prefix: "$/progress" -> "progress"
// Caller must free the result.
char *method_to_func_prefix(const char *method);

// Strip "$/" prefix from method names for cleaner enum/function names
// Returns pointer into original string (no allocation)
const char *strip_dollar_prefix(const char *method);

#endif /* GEN3_COMMON_H */
