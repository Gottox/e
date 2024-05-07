#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

extern const TSLanguage *tree_sitter_c(void);

static char *
read_content(const char *path) {
	bool success = false;
	char *content = NULL;
	size_t size = 0;
	FILE *file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "Failed to open file: %s\n", path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	content = (char *)malloc(size + 1);
	if (!content) {
		fprintf(stderr, "Failed to allocate memory for file: %s\n", path);
		goto out;
	}

	if (fread(content, 1, size, file) != size) {
		fprintf(stderr, "Failed to read file: %s\n", path);
		goto out;
	}

	content[size] = '\0';

	success = true;
out:
	if (file) {
		fclose(file);
	}
	if (!success) {
		free(content);
		content = NULL;
	}
	return content;
}

int
main(int argc, char *argv[]) {
	assert(argc == 3);
	const char *source_path = argv[1];
	const char *query_path = argv[2];

	const TSLanguage *lang = tree_sitter_c();
	TSParser *parser = ts_parser_new();
	if (!parser) {
		fprintf(stderr, "Failed to create parser\n");
		return 1;
	}
	ts_parser_set_language(parser, lang);

	TSTree *tree = NULL;
	char *source = read_content(source_path);
	if (!source) {
		goto out;
	}

	tree = ts_parser_parse_string(parser, NULL, source, strlen(source));

	if (!tree) {
		fprintf(stderr, "Failed to parse content\n");
		goto out;
	}

	TSNode root = ts_tree_root_node(tree);

	char *query = read_content(query_path);

	if (!query) {
		fprintf(stderr, "Failed to read query\n");
		goto out;
	}

	uint32_t error_offset;
	TSQueryError error_type;

	TSQuery *ts_query = ts_query_new(
			lang, query, strlen(query), &error_offset, &error_type);
	if (!ts_query) {
		fprintf(stderr, "Failed to parse query %i\n", error_offset);
		goto out;
	}

	TSQueryCursor *cursor = ts_query_cursor_new();
	if (!cursor) {
		fprintf(stderr, "Failed to create query cursor\n");
		goto out;
	}

	ts_query_cursor_exec(cursor, ts_query, root);

	TSQueryMatch match;
	while (ts_query_cursor_next_match(cursor, &match)) {
		printf("Match: pattern_index=%u, captures.size=%u\n",
			   match.pattern_index, match.capture_count);
		for (uint32_t i = 0; i < match.capture_count; i++) {
			TSQueryCapture capture = match.captures[i];
			TSNode node = capture.node;
			char *node_string = ts_node_string(node);
			printf("  capture_index=%u, node=%s\n", capture.index, node_string);
			free(node_string);
			uint32_t start_index = ts_node_start_byte(node);
			uint32_t end_index = ts_node_end_byte(node);
			fwrite(source + start_index, 1, end_index - start_index, stdout);
			puts("");
		}
		puts("-----------------------------");
	}

	ts_query_cursor_delete(cursor);
	ts_parser_delete(parser);
	ts_query_delete(ts_query);
	ts_tree_delete(tree);
	free(query);
	free(source);

out:
	return 0;
}
