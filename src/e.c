#include <quickjs/quickjs.h>
#include <rope.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <tree.h>
#include <ttyui.h>

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
		int rv = rope_append(r, buffer, read_size);
		if (rv < 0) {
			fprintf(stderr, "Failed to append to rope\n");
			goto out;
		}
	}

out:
	fclose(fp);
	return 0;
}

int
handle_tty(struct TtyUi *ui, struct TtyUiEvent *ev, void *user_data) {
	(void)ui;
	(void)user_data;
	int rv = 0;
	printf("Event: %d ", ev->type);
	switch (ev->type) {
	case TTYUI_EVENT_KEY:
		printf("Key: %s (%x)\n\r", ev->key.seq, ev->key.seq[0]);
		if (strcmp(ev->key.seq, "q") == 0) {
			rv = -1;
			goto out;
		}
		break;
	case TTYUI_EVENT_CURSOR:
		printf("Cursor: %02x (%c)\r\n", ev->cursor.direction,
			   ev->cursor.direction);
		break;
	case TTYUI_EVENT_MOUSE:
		printf("Mouse: X%d Y%d B%d P%d\r\n", ev->mouse.x, ev->mouse.y,
			   ev->mouse.button, ev->mouse.pressed);
		break;
	case TTYUI_EVENT_FOCUS:
		printf("Focus: %d\r\n", ev->focus.focus);
		break;
	case TTYUI_EVENT_RESIZE:
	case TTYUI_EVENT_EOF:
		fputs("Resize or EOF\r\n", stdout);
		break;
	}

out:
	return rv;
}

int
main(int argc, char *argv[]) {
	JSRuntime *runtime;
	JSContext *context;
	int rv = 0;
	struct Rope rope = {0};
	struct TtyUi ui = {0};

	prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	rv = ttyui_init(&ui, 0, handle_tty, NULL);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize tty ui\n");
		return 1;
	}
	rv = rope_init(&rope);
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

	while (ttyui_process(&ui) >= 0) {
	}
	// while (1) {
	//	struct RopeNode *node = rope_first(&rope);
	//	for (; node; rope_node_next(&node)) {
	//		// printf("%lu\n", node->byte_size);
	//		size_t size = 0;
	//		const uint8_t *value = rope_node_value(node, &size);
	//		fwrite(value, 1, size, stdout);
	//	}
	//	fputs("\n-------\n", stdout);
	//	e_tree_view_ascii(&e_tree_visitor_rope_impl, rope.root, stdout);
	//	chr = getchar();
	//	if (chr == EOF) {
	//		break;
	//	}

	//	uint8_t chr2 = (uint8_t)chr;
	//	rope_append(&rope, &chr2, 1);
	//}

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

	// out:
	//  free(input_buffer);
	JS_FreeContext(context);
	JS_FreeRuntime(runtime);
	rope_cleanup(&rope);
	ttyui_cleanup(&ui);

	return 0;
}
