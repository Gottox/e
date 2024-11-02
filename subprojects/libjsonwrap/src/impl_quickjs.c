#include "jw.h"
#include "jw_quickjs.h"
#include <cextras/memory.h>
#include <quickjs.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define EXCEPTION(v) \
	if (JS_IsException(v)) { \
		rv = -1; \
		goto out; \
	}

int
jw_init(struct Jw *jw, void *context) {
	jw->context = context;
	return 0;
}

int
jw_parse(struct Jw *jw, struct JwVal *val, const char *json, size_t size) {
	int rv = 0;
	JSValue value = JS_ParseJSON(jw->context, json, size, "<json>");
	EXCEPTION(value);

	val->value = value;
out:
	if (rv < 0) {
		JS_FreeValue(jw->context, value);
	}
	return rv;
}

int
jw_serialize(struct Jw *jw, struct JwVal *val, char **json, size_t *size) {
	int rv = 0;
	JSValue value = JS_JSONStringify(
			jw->context, val->value, JS_UNDEFINED, JS_UNDEFINED);
	EXCEPTION(value);

	const char *js_json = JS_ToCString(jw->context, value);
	*json = strdup(js_json);
	if (*json == NULL) {
		rv = -1;
		goto out;
	}

	*size = strlen(*json);
out:
	JS_FreeCString(jw->context, js_json);
	JS_FreeValue(jw->context, value);
	if (rv < 0) {
		free(*json);
	}
	return rv;
}

int
jw_obj_get(
		struct Jw *jw, struct JwVal *object, const char *key,
		struct JwVal *target) {
	int rv = 0;
	target->value = JS_GetPropertyStr(jw->context, object->value, key);
	EXCEPTION(target->value);
out:
	if (rv < 0) {
		JS_FreeValue(jw->context, target->value);
	}
	return rv;
}

int
jw_arr_get(
		struct Jw *jw, struct JwVal *arr, size_t index, struct JwVal *target) {
	int rv = 0;
	target->value = JS_GetPropertyUint32(jw->context, arr->value, index);
	EXCEPTION(target->value);
out:
	if (rv < 0) {
		JS_FreeValue(jw->context, target->value);
	}
	return rv;
}

ssize_t
jw_arr_len(struct Jw *jw, struct JwVal *arr) {
	int rv = 0;
	int length;
	rv = jw_obj_get_int(jw, arr, "length", &length);
	if (rv < 0) {
		return rv;
	}
	return length;
}

int
jw_int(struct Jw *jw, struct JwVal *val, int *int_val) {
	int rv = 0;
	if (!JS_IsNumber(val->value) && JS_IsBool(val->value)) {
		rv = -1;
		goto out;
	}
	*int_val = JS_VALUE_GET_INT(val->value);
out:
	return rv;
}

int
jw_float(struct Jw *jw, struct JwVal *val, double *f_val) {
	int rv = 0;
	if (!JS_IsNumber(val->value)) {
		rv = -1;
		goto out;
	}
	*f_val = JS_VALUE_GET_FLOAT64(val->value);
out:
	return rv;
}

int
jw_str(struct Jw *jw, struct JwVal *val, char **str, size_t *size) {
	int rv = 0;
	size_t str_size;
	const char *js_str = JS_ToCStringLen(jw->context, &str_size, val->value);
	*str = cx_memdup(js_str, str_size);
	if (*str == NULL) {
		rv = -1;
		goto out;
	}
	if (size != NULL) {
		*size = str_size;
	}
out:
	if (rv < 0) {
		free(*str);
	}
	JS_FreeCString(jw->context, js_str);
	return rv;
}

bool
jw_is_str(struct Jw *jw, struct JwVal *val) {
	return JS_IsString(val->value);
}

bool
jw_is_int(struct Jw *jw, struct JwVal *val) {
	return JS_IsNumber(val->value) &&
			JS_VALUE_GET_TAG(val->value) == JS_TAG_INT;
}

bool
jw_is_float(struct Jw *jw, struct JwVal *val) {
	return JS_IsNumber(val->value) &&
			JS_VALUE_GET_TAG(val->value) == JS_TAG_FLOAT64;
}

bool
jw_is_obj(struct Jw *jw, struct JwVal *val) {
	return JS_IsObject(val->value);
}

bool
jw_is_arr(struct Jw *jw, struct JwVal *val) {
	return JS_IsArray(jw->context, val->value);
}

bool
jw_is_null(struct Jw *jw, struct JwVal *val) {
	return JS_IsNull(val->value);
}

bool
jw_is_bool(struct Jw *jw, struct JwVal *val) {
	return JS_IsBool(val->value);
}

int
jw_cleanup(struct Jw *jw, struct JwVal *val) {
	JS_FreeValue(jw->context, val->value);
	memset(val, 0, sizeof(struct JwVal));
	return 0;
}
