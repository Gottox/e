/**
 * @file jw.h
 * @brief Header file for JSON Wrapper functions using QuickJS.
 */

#ifndef JW_H
#define JW_H

#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>

#include <jw_backend.h>

int jw_init(struct Jw *jw, void *context);

int jw_deinit(struct Jw *jw);

int jw_parse(struct JwVal *val, struct Jw *jw, const char *json, size_t size);

int jw_serialize(struct JwVal *val, char **json, size_t *size);

int jw_obj_get(struct JwVal *object, const char *key, struct JwVal *target);

int jw_arr_get(
		struct JwVal *arr, size_t index, struct JwVal *target);

ssize_t jw_arr_len(struct JwVal *arr);

int jw_int(struct JwVal *val, int *int_val);

int jw_float(struct JwVal *val, double *f_val);

int jw_bool(struct JwVal *val, bool *b);

int jw_str(struct JwVal *val, char **str, size_t *size);

int jw_cleanup(struct JwVal *val);

// Util functions

typedef int (*jw_arr_foreach_fn)(
		struct JwVal *val, int index, void *data);

int jw_arr_foreach(
		struct JwVal *arr, jw_arr_foreach_fn fn, void *data);

int jw_obj_get_str(
		struct JwVal *obj, const char *key, char **str,
		size_t *size);

int
jw_obj_get_int(struct JwVal *obj, const char *key, int *number);

int jw_obj_get_float(
                struct JwVal *obj, const char *key, double *number);
int jw_obj_get_bool(struct JwVal *obj, const char *key, bool *boolean);

int jw_arr_get_str(
		struct JwVal *arr, size_t index, char **str,
		size_t *size);

int jw_arr_get_int(struct JwVal *arr, size_t index, int *number);

int jw_arr_get_float(
                struct JwVal *arr, size_t index, double *number);
int jw_arr_get_bool(struct JwVal *arr, size_t index, bool *boolean);

void jw_debug(struct JwVal *val);

int jw_dup(struct JwVal *target, struct JwVal *src);

bool jw_is_str(struct JwVal *val);

bool jw_is_int(struct JwVal *val);

bool jw_is_float(struct JwVal *val);

bool jw_is_obj(struct JwVal *val);

bool jw_is_arr(struct JwVal *val);

bool jw_is_null(struct JwVal *val);

bool jw_is_bool(struct JwVal *val);

#endif // JW_H
