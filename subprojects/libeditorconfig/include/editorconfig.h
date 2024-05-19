#ifndef EDITORCONFIG_PARSER_H
#define EDITORCONFIG_PARSER_H

#include <stdbool.h>
#include <stddef.h>

struct EditorConfigSegment {
	char *section;
	char *indent_style;
	int indent_size;
	int tab_width;
	char *end_of_line;
	char *charset;
	bool trim_trailing_whitespace;
	bool insert_final_newline;
};

struct EditorConfig {
	bool root;
	struct EditorConfigSegment *segments;
	size_t segment_count;
};

int editorconfig_parse(struct EditorConfig *config, const char *filename);
void editorconfig_cleanup(struct EditorConfig *config);

#endif /* EDITORCONFIG_PARSER_H */
