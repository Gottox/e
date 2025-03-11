/**
 * @file jw.h
 * @brief Header file for JSON Wrapper functions using QuickJS.
 */

#ifndef JW_H
#define JW_H

#include <stddef.h>
#include <unistd.h>

struct Jw;

struct JwVal;

int jw_init(struct Jw *jw, void *context);

int jw_parse(struct Jw *jw, struct JwVal *val, const char *json, size_t size);

int jw_serialize(struct Jw *jw, struct JwVal *val, char **json, size_t *size);

int jw_obj_get(
		struct Jw *jw, struct JwVal *object, const char *key,
		struct JwVal *target);

int jw_arr_get(
		struct Jw *jw, struct JwVal *arr, size_t index, struct JwVal *target);

ssize_t jw_arr_len(struct Jw *jw, struct JwVal *arr);

int jw_int(struct Jw *jw, struct JwVal *val, int *int_val);

int jw_float(struct Jw *jw, struct JwVal *val, double *f_val);

int jw_str(struct Jw *jw, struct JwVal *val, char **str, size_t *size);

int jw_cleanup(struct Jw *jw, struct JwVal *val);

// Util functions

typedef int (*jw_arr_foreach_fn)(
		struct Jw *jw, struct JwVal *val, int index, void *data);

int jw_arr_foreach(
		struct Jw *jw, struct JwVal *arr, jw_arr_foreach_fn fn, void *data);

int jw_obj_get_str(
		struct Jw *jw, struct JwVal *obj, const char *key, char **str,
		size_t *size);

int
jw_obj_get_int(struct Jw *jw, struct JwVal *obj, const char *key, int *number);

int jw_obj_get_float(
		struct Jw *jw, struct JwVal *obj, const char *key, double *number);

int jw_arr_get_str(
		struct Jw *jw, struct JwVal *arr, size_t index, char **str,
		size_t *size);

int jw_arr_get_int(struct Jw *jw, struct JwVal *arr, size_t index, int *number);

int jw_arr_get_float(
		struct Jw *jw, struct JwVal *arr, size_t index, double *number);

void jw_debug(struct Jw *jw, struct JwVal *val);

int jw_dup(struct Jw *jw, struct JwVal *target, struct JwVal *src);

#endif // JW_H
