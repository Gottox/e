#include <jw.h>
#include <quickjs.h>
#include <stddef.h>
#include <stdlib.h>

int
jw_obj_get_str(struct JwVal *obj, const char *key, char **str, size_t *size) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_str(&val, str, size);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_obj_get_int(struct JwVal *obj, const char *key, int *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_int(&val, number);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_obj_get_float(struct JwVal *obj, const char *key, double *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(obj, key, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_float(&val, number);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_obj_get_bool(struct JwVal *obj, const char *key, bool *boolean) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_obj_get(obj, key, &val);
	if (rv < 0)
		goto out;
	rv = jw_bool(&val, boolean);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_arr_get_str(struct JwVal *arr, size_t index, char **str, size_t *size) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_str(&val, str, size);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_arr_get_int(struct JwVal *arr, size_t index, int *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_int(&val, number);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_arr_get_float(struct JwVal *arr, size_t index, double *number) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(arr, index, &val);
	if (rv < 0) {
		goto out;
	}
	rv = jw_float(&val, number);
out:
	jw_cleanup(&val);
	return rv;
}

int
jw_arr_get_bool(struct JwVal *arr, size_t index, bool *boolean) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(arr, index, &val);
	if (rv < 0)
		goto out;
	rv = jw_bool(&val, boolean);
out:
	jw_cleanup(&val);
	return rv;
}

void
jw_debug(struct JwVal *val) {
	char *str = NULL;
	size_t size = 0;

	int rv = jw_serialize(val, &str, &size);
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
		struct JwVal *arr, int index, jw_arr_foreach_fn cb, void *data) {
	int rv = 0;
	struct JwVal val = {0};
	rv = jw_arr_get(arr, index, &val);
	if (rv < 0) {
		goto out;
	}

	rv = cb(&val, index, data);
	if (rv < 0) {
		goto out;
	}

out:
	jw_cleanup(&val);
	return rv;
}

int
jw_arr_foreach(struct JwVal *arr, jw_arr_foreach_fn cb, void *data) {
	int rv = 0;
	size_t len = jw_arr_len(arr);
	if (len < 0) {
		return len;
	}

	for (size_t i = 0; i < len && rv == 0; i++) {
		rv = jw_arr_foreach_inner(arr, i, cb, data);
	}

	return rv;
}
