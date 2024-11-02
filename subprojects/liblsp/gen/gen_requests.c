#include "identifier_util.h"
#include <jw.h>
#include <jw_quickjs.h>
#include <quickjs.h>
#include <stdlib.h>

static int
gen_request(struct Jw *jw, struct JwVal *request, int index, void *user) {
	(void)index;
	(void)user;
	char *method = NULL;
	size_t method_size;
	int rv;
	rv = jw_obj_get_str(jw, request, "method", &method, &method_size);
	if (rv != 0) {
		fprintf(stderr, "Failed to get method\n");
		return -1;
	}
	printf("Method: %s\n", method);
	char *snake = snake_case(method);
	printf("Snake: %s\n", snake);
	free(snake);
	pascal_case(method);
	printf("Pascal: %s\n", method);
	free(method);
	return 0;
}

int
gen_requests(struct Jw *jw, struct JwVal *requests) {
	jw_arr_foreach(jw, requests, gen_request, NULL);
	return 0;
}
