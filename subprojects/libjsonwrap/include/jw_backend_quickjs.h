#ifndef JW_QUICKJS_H
#define JW_QUICKJS_H

#include <quickjs.h>
#include <stdbool.h>

struct Jw {
	JSContext *context;
};

struct JwVal {
	JSValue value;
	JSContext *context;
};

#endif // JW_QUICKJS_H
