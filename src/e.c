#include <quickjs/quickjs.h>
#include <rope.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
print_tree(struct Rope *rope, char *filename) {
	FILE *fp = fopen(filename, "w");
	if (!fp) {
		fprintf(stderr, "Failed to open file: %s\n", filename);
		return;
	}
	rope_print_tree(rope, fp);
	fclose(fp);
}

void
parse_json_and_print(JSContext *ctx, const char *json_str) {
	JSValue obj = JS_ParseJSON(ctx, json_str, strlen(json_str), "input");
	if (JS_IsException(obj)) {
		JSValue exception = JS_GetException(ctx);
		// JS_PrintError(ctx, stderr, exception);
		JS_FreeValue(ctx, exception);
	} else {
		const char *result_str = JS_ToCString(ctx, obj);
		if (result_str) {
			// printf("Parsed JSON: %s\n", result_str);
			JS_FreeCString(ctx, result_str);
		}
	}
	JS_FreeValue(ctx, obj);
}

int
read_file(struct Rope *r, const char *file_name) {
	size_t read_size = 0;
	uint8_t buffer[512] = {0};
	FILE *fp = fopen(file_name, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open file: %s\n", file_name);
		return 1;
	}

	while ((read_size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
		print_tree(r, "tree.txt");
		int rv = rope_append(r, buffer, read_size);
		if (rv < 0) {
			fprintf(stderr, "Failed to append to rope\n");
			goto out;
		}
	}

	print_tree(r, "tree.txt");

out:
	fclose(fp);
	return 0;
}

int
main(int argc, char *argv[]) {
	JSRuntime *runtime;
	JSContext *context;
	struct Rope rope = {0};

	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	int rv = rope_init(&rope);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize rope\n");
		return 1;
	}
	if (read_file(&rope, argv[1]) < 0) {
		return 1;
	}

	// Set up QuickJS runtime
	runtime = JS_NewRuntime();
	context = JS_NewContext(runtime);

	if (!runtime || !context) {
		fprintf(stderr, "Failed to initialize QuickJS\n");
		return 1;
	}

	// struct RopeNode *node = rope_first(&rope);
	// for(; node; rope_node_next(&node)) {
	//	printf("%lu\n", node->byte_size);
	// }

	// Read a line from stdin using getline
	// char *input_buffer = NULL;
	// size_t input_size = 0;
	// printf("Enter a JSON string: ");
	// ssize_t bytes_read = getline(&input_buffer, &input_size, stdin);

	//// Check for input failure
	// if (bytes_read == -1) {
	//	fprintf(stderr, "Failed to read input\n");
	//	free(input_buffer);
	//	return 1;
	// }

	// if (input_buffer[bytes_read - 1] == '\n') {
	//	input_buffer[bytes_read - 1] = '\0';
	// }

	parse_json_and_print(context, "123");

	// free(input_buffer);
	JS_FreeContext(context);
	JS_FreeRuntime(runtime);
	rope_cleanup(&rope);

	return 0;
}
