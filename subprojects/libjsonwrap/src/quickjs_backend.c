#include "jw.h"
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
jw_dup(struct JwVal *target, struct JwVal *src) {
	target->value = JS_DupValue(src->context, src->value);
	target->context = src->context;
	return 0;
}

int
jw_init(struct Jw *jw, void *context) {
	if (context == NULL) {
		JSRuntime *rt = JS_NewRuntime();
		if (rt == NULL) {
			return -1;
		}
		jw->context = JS_NewContext(rt);
		JS_FreeRuntime(rt);
	} else {
		jw->context = JS_DupContext((JSContext *)context);
	}
	return 0;
}

int
jw_deinit(struct Jw *jw) {
	JS_FreeContext(jw->context);
	return 0;
}

int
jw_parse(struct JwVal *val, struct Jw *jw, const char *json, size_t size) {
	int rv = 0;
	JSValue value = JS_ParseJSON(jw->context, json, size, "<json>");
	EXCEPTION(value);

	val->value = value;
	val->context = jw->context;
out:
	if (rv < 0) {
		JS_FreeValue(jw->context, value);
	}
	return rv;
}

int
jw_serialize(struct JwVal *val, char **json, size_t *size) {
	int rv = 0;
	JSValue value = JS_JSONStringify(
			val->context, val->value, JS_UNDEFINED, JS_UNDEFINED);
	EXCEPTION(value);

	const char *js_json = JS_ToCString(val->context, value);
	*json = strdup(js_json);
	if (*json == NULL) {
		rv = -1;
		goto out;
	}

	*size = strlen(*json);
out:
	JS_FreeCString(val->context, js_json);
	JS_FreeValue(val->context, value);
	if (rv < 0) {
		free(*json);
	}
	return rv;
}

int
jw_obj_get(struct JwVal *object, const char *key, struct JwVal *target) {
	int rv = 0;
	target->value = JS_GetPropertyStr(object->context, object->value, key);
	target->context = object->context;
	EXCEPTION(target->value);
out:
	if (rv < 0) {
		JS_FreeValue(object->context, target->value);
	}
	return rv;
}

int
jw_arr_get(struct JwVal *arr, size_t index, struct JwVal *target) {
	int rv = 0;
	target->value = JS_GetPropertyUint32(arr->context, arr->value, index);
	target->context = arr->context;
	EXCEPTION(target->value);
out:
	if (rv < 0) {
		JS_FreeValue(arr->context, target->value);
	}
	return rv;
}

ssize_t
jw_arr_len(struct JwVal *arr) {
	int rv = 0;
	int length;
	rv = jw_obj_get_int(arr, "length", &length);
	if (rv < 0) {
		return rv;
	}
	return length;
}

int
jw_int(struct JwVal *val, int *int_val) {
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
jw_float(struct JwVal *val, double *f_val) {
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
jw_bool(struct JwVal *val, bool *b) {
        int rv = 0;
        if (!jw_is_bool(val)) {
                rv = -1;
                goto out;
        }
        *b = JS_VALUE_GET_BOOL(val->value);
out:
        return rv;
}

int
jw_str(struct JwVal *val, char **str, size_t *size) {
	int rv = 0;
	size_t str_size = 0;
	const char *js_str = NULL;
	*str = NULL;
	if (!jw_is_str(val)) {
		rv = -1;
		goto out;
	}

	js_str = JS_ToCStringLen(val->context, &str_size, val->value);
	*str = cx_memdup(js_str, str_size);
	if (*str == NULL) {
		rv = -1;
		goto out;
	}
out:
	if (rv < 0) {
		free(*str);
		*str = NULL;
	}
	if (size != NULL) {
		*size = str_size;
	}
	JS_FreeCString(val->context, js_str);
	return rv;
}

bool
jw_is_str(struct JwVal *val) {
	return JS_IsString(val->value);
}

bool
jw_is_int(struct JwVal *val) {
	return JS_IsNumber(val->value) &&
			JS_VALUE_GET_TAG(val->value) == JS_TAG_INT;
}

bool
jw_is_float(struct JwVal *val) {
	return JS_IsNumber(val->value) &&
			JS_VALUE_GET_TAG(val->value) == JS_TAG_FLOAT64;
}

bool
jw_is_obj(struct JwVal *val) {
	return JS_IsObject(val->value);
}

bool
jw_is_arr(struct JwVal *val) {
	return JS_IsArray(val->value);
}

bool
jw_is_null(struct JwVal *val) {
	return JS_IsNull(val->value);
}

bool
jw_is_bool(struct JwVal *val) {
	return JS_IsBool(val->value);
}

int
jw_cleanup(struct JwVal *val) {
	JS_FreeValue(val->context, val->value);
	memset(val, 0, sizeof(struct JwVal));
	return 0;
}
