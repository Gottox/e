#include <getopt.h>
#include <highlight.h>
#include <stdio.h>
#include <string.h>

const TSLanguage *tree_sitter_c(void);

static const char *const capture_names[] = {
		"attribute",
		"boolean",
		"carriage-return",
		"comment",
		"comment.documentation",
		"constant",
		"constant.builtin",
		"constructor",
		"constructor.builtin",
		"embedded",
		"error",
		"escape",
		"function",
		"function.builtin",
		"keyword",
		"markup",
		"markup.bold",
		"markup.heading",
		"markup.italic",
		"markup.link",
		"markup.link.url",
		"markup.list",
		"markup.list.checked",
		"markup.list.numbered",
		"markup.list.unchecked",
		"markup.list.unnumbered",
		"markup.quote",
		"markup.raw",
		"markup.raw.block",
		"markup.raw.inline",
		"markup.strikethrough",
		"module",
		"number",
		"operator",
		"property",
		"property.builtin",
		"punctuation",
		"punctuation.bracket",
		"punctuation.delimiter",
		"punctuation.special",
		"string",
		"string.escape",
		"string.regexp",
		"string.special",
		"string.special.symbol",
		"tag",
		"type",
		"type.builtin",
		"variable",
		"variable.builtin",
		"variable.member",
		"variable.parameter",
};
static const int capture_names_count =
		sizeof(capture_names) / sizeof(capture_names[0]);

int
usage(char *arg0) {
	printf("Usage: %s <query> <src>\n", arg0);
	return 1;
}

static char *
get_content(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Failed to open file %s\n", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *content = malloc(size + 1);
	if (!content) {
		fprintf(stderr, "Failed to allocate memory\n");
		fclose(f);
		return NULL;
	}

	long read = fread(content, 1, size, f);
	if (read != size) {
		fprintf(stderr, "Failed to read file %s\n", path);
		fclose(f);
		free(content);
		return NULL;
	}

	content[size] = '\0';
	fclose(f);
	return content;
}

static char *source = NULL;

static void
highlight_cb(const struct HighlightEvent *event) {
	switch (event->type) {
	case HIGHLIGHT_START:
		printf("\033[38;5;%um", event->start.capture_id);
		break;
	case HIGHLIGHT_TEXT:
		fwrite(&source[event->text.start], 1,
			   event->text.end - event->text.start, stdout);
		break;
	case HIGHLIGHT_END:
		fputs("\033[0m", stdout);
		break;
	}
}

static void
dump_cb(const struct HighlightEvent *event) {
	switch (event->type) {
	case HIGHLIGHT_START:
		printf("start: %s\n", capture_names[event->start.capture_id]);
		break;
	case HIGHLIGHT_TEXT:
		printf("text: %i - %i\n", event->text.start, event->text.end);
		break;
	case HIGHLIGHT_END:
		printf("end\n");
		break;
	}
}

int
main(int argc, char *argv[]) {
	const TSLanguage *lang = tree_sitter_c();
	struct Highlight hl = {0};
	struct HighlightIterator iter = {0};
	TSParser *parser = NULL;
	TSTree *tree = NULL;
	int opt;
	void (*cb)(const struct HighlightEvent *) = highlight_cb;

	while ((opt = getopt(argc, argv, "hd")) != -1) {
		switch (opt) {
		case 'd':
			cb = dump_cb;
			break;
		default:
			return usage(argv[0]);
		}
	}

	char *arg0 = argv[0];
	argc -= optind;
	argv += optind;

	if (argc != 2) {
		return usage(arg0);
	}

	char *query_src = get_content(argv[0]);
	if (!query_src) {
		fprintf(stderr, "Failed to read query\n");
		return 1;
	}
	size_t query_len = strlen(query_src);

	source = get_content(argv[1]);
	if (!source) {
		fprintf(stderr, "Failed to read source\n");
		free(query_src);
		return 1;
	}
	size_t source_len = strlen(source);

	struct HighlightConfig config = {0};
	highlight_config_init(&config, lang);
	highlight_config_highlight(&config, query_src, query_len);
	highlight_config_captures(&config, capture_names, capture_names_count);

	int rv = highlight_init(&hl, &config);
	if (rv < 0) {
		fprintf(stderr, "Failed to initialize highlighter\n");
		goto out;
	}

	parser = ts_parser_new();
	if (!parser) {
		fprintf(stderr, "Failed to initialize parser\n");
		goto out;
	}
	ts_parser_set_language(parser, lang);

	tree = ts_parser_parse_string(parser, NULL, source, source_len);

	rv = highlight_iterator_init(&iter, &hl, tree);

	struct HighlightEvent event;
	while (highlight_iterator_next(&iter, &event, &rv)) {
		cb(&event);
	}
out:
	highlight_iterator_cleanup(&iter);
	highlight_cleanup(&hl);
	ts_tree_delete(tree);
	ts_parser_delete(parser);
	free(query_src);
	free(source);
	return 0;
}
