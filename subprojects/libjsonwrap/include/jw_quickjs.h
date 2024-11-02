#ifndef JW_QUICKJS_H
#define JW_QUICKJS_H

#include <quickjs.h>
#include <stdbool.h>

struct Jw {
	JSContext *context;
};

struct JwVal {
	JSValue value;
	void *marker;
};

bool jw_is_str(struct Jw *jw, struct JwVal *val);

bool jw_is_int(struct Jw *jw, struct JwVal *val);

bool jw_is_float(struct Jw *jw, struct JwVal *val);

bool jw_is_obj(struct Jw *jw, struct JwVal *val);

bool jw_is_arr(struct Jw *jw, struct JwVal *val);

bool jw_is_null(struct Jw *jw, struct JwVal *val);

bool jw_is_bool(struct Jw *jw, struct JwVal *val);

#endif // JW_QUICKJS_H
