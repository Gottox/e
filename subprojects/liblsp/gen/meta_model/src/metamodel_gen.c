#include <jw.h>
#include <jw_quickjs.h>
#include <metamodel_gen.h>
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
	fread(buf, 1, len, f);
	buf[len] = 0;
	fclose(f);
	return buf;
}

int
main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <metaModel.json> <output.h>\n", argv[0]);
		exit(1);
	}
	char *input = argv[1];
	char *json = read_file(input);
	if (!json) {
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
	rv = jw_parse(&jw, &obj, json, strlen(json));
	if (rv != 0) {
		fprintf(stderr, "Failed to parse JSON\n");
		exit(1);
	}

	struct Generator generator = {0};
	rv = generator_init(&generator, &jw, &obj);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize generator\n");
		exit(1);
	}

	rv = generator_load_types(&generator);
	if (rv < 0) {
		fprintf(stderr, "Failed to load types\n");
		exit(1);
	}

	struct Type *type = generator.type_list.head;
	while (type) {
		type->generate_type(type);
		type = type->next;
	}

	generator_cleanup(&generator);
	jw_cleanup(&jw, &obj);
	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);
	free(json);
	return 0;
}
