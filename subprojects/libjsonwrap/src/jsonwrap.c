#include <jw.h>
#include <jw_quickjs.h>
#include <quickjs.h>
#include <stddef.h>
#include <stdlib.h>

int
jw_obj_get_str(
		struct Jw *jw, struct JwVal *obj, const char *key, char **str,
		size_t *size) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(jw, obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_str(jw, &val, str, size);
out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_obj_get_int(struct Jw *jw, struct JwVal *obj, const char *key, int *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(jw, obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_int(jw, &val, number);
out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_obj_get_float(
		struct Jw *jw, struct JwVal *obj, const char *key, double *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(jw, obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_float(jw, &val, number);
out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_arr_get_str(
		struct Jw *jw, struct JwVal *arr, size_t index, char **str,
		size_t *size) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(jw, arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_str(jw, &val, str, size);
out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_arr_get_int(struct Jw *jw, struct JwVal *arr, size_t index, int *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(jw, arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_int(jw, &val, number);
out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_arr_get_float(
		struct Jw *jw, struct JwVal *arr, size_t index, double *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(jw, arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_float(jw, &val, number);
out:
	jw_cleanup(jw, &val);
	return rv;
}

void
jw_debug(struct Jw *jw, struct JwVal *val) {
	char *str = NULL;
	size_t size = 0;

	int rv = jw_serialize(jw, val, &str, &size);
	if (rv < 0) {
		goto out;
	}

	fwrite(str, 1, size, stderr);
	fputc('\n', stderr);

out:
	free(str);
}

static int
jw_arr_foreach_inner(
		struct Jw *jw, struct JwVal *arr, int index, jw_arr_foreach_fn cb,
		void *data) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(jw, arr, index, &val);
	if (rv < 0) {
		goto out;
	}

	rv = cb(jw, &val, index, data);
	if (rv < 0) {
		goto out;
	}

out:
	jw_cleanup(jw, &val);
	return rv;
}

int
jw_arr_foreach(
		struct Jw *jw, struct JwVal *arr, jw_arr_foreach_fn cb, void *data) {
	int rv = 0;
	size_t len = jw_arr_len(jw, arr);
	if (len < 0) {
		return len;
	}

	for (size_t i = 0; i < len && rv == 0; i++) {
		rv = jw_arr_foreach_inner(jw, arr, i, cb, data);
	}

	return rv;
}
