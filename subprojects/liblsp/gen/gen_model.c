#include <ctype.h>
#include <quickjs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
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

char *
pascalCase(char *str) {
	int j = 0;

	for (int i = 0; str[i]; i++, j++) {
		if (str[i] == '_' || str[i] == '/') {
			i++;
			if (str[i]) {
				str[j] = toupper(str[i]);
			}
		} else {
			str[j] = str[i];
		}
	}
	str[j] = '\0';
	return str;
}

int
main(int argc, char **argv) {
	if (argc < 2) {
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
	JSValue obj = JS_ParseJSON(ctx, buf, strlen(buf), input);

	JSValue requests = JS_GetPropertyStr(ctx, obj, "requests");
	JSValue notifications = JS_GetPropertyStr(ctx, obj, "notifications");
	JSValue structures = JS_GetPropertyStr(ctx, obj, "structures");
	JSValue enumerations = JS_GetPropertyStr(ctx, obj, "enumerations");
	JSValue type_aliases = JS_GetPropertyStr(ctx, obj, "typeAliases");

	JSValue length_val = JS_GetPropertyStr(ctx, requests, "length");
	const size_t length = JS_VALUE_GET_INT(length_val);

	for (int i = 0; i < length; i++) {
		JSValue request = JS_GetPropertyUint32(ctx, requests, i);
		JSValue method_val = JS_GetPropertyStr(ctx, request, "method");
		printf("Method: %s\n", JS_ToCString(ctx, method_val));
	}

	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);
	return 0;
}
