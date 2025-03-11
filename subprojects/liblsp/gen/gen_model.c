#include "identifier_util.h"
#include <gen.h>
#include <jw.h>
#include <jw_quickjs.h>
#include <quickjs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
read_file(char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Could not open file %s\n", filename);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = calloc(len + 1, 1);
	if (!buf) {
		perror("calloc");
		exit(1);
	}
	fread(buf, 1, len, f);
	buf[len] = 0;
	fclose(f);
	return buf;
}

int
main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <metaModel.json> <output.h>\n", argv[0]);
		exit(1);
	}
	char *input = argv[1];
	char *output = argv[2];
	char *buf = read_file(input);
	if (!buf) {
		perror(input);
		exit(1);
	}

	JSRuntime *rt = JS_NewRuntime();
	JSContext *ctx = JS_NewContext(rt);
	struct Jw jw = {0};
	int rv = jw_init(&jw, ctx);
	if (rv != 0) {
		fprintf(stderr, "Failed to initialize jw\n");
		exit(1);
	}

	struct JwVal obj = {0};
	rv = jw_parse(&jw, &obj, buf, strlen(buf));
	if (rv != 0) {
		fprintf(stderr, "Failed to parse JSON\n");
		exit(1);
	}

	struct JwVal type_aliases = {0};
	rv = jw_obj_get(&jw, &obj, "typeAliases", &type_aliases);
	struct JwVal structures = {0};
	rv = jw_obj_get(&jw, &obj, "structures", &structures);
	struct JwVal enums = {0};
	rv = jw_obj_get(&jw, &obj, "enums", &type_aliases);
	gen_structures(&jw, &structures, &type_aliases, &enums);

	// struct JwVal requests = {0};
	// rv = jw_obj_get(&jw, &obj, "requests", &requests);
	// gen_requests(&jw, &requests);

	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);
	return 0;
}
