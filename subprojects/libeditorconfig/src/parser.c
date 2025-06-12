#include <ctype.h>
#include <editorconfig.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

enum FieldType { TYPE_STRING, TYPE_INT, TYPE_BOOL };

struct FieldInfo {
	const char *name;
	size_t offset;
	enum FieldType type;
};

static struct FieldInfo fields_info[] = {
#define E(n, t) {#n, offsetof(struct EditorConfigSegment, n), t}
		E(indent_style, TYPE_STRING),
		E(indent_size, TYPE_INT),
		E(tab_width, TYPE_INT),
		E(end_of_line, TYPE_STRING),
		E(charset, TYPE_STRING),
		E(trim_trailing_whitespace, TYPE_BOOL),
		E(insert_final_newline, TYPE_BOOL),
#undef E
};

#define SEGMENT_FIELDS_LENGTH (sizeof(fields_info) / sizeof(fields_info[0]))

static const char *
trim_str(char *str) {
	char *end;
	while (isspace(*str)) {
		str++;
	}

	if (str[0] == '\0') {
		return str;
	}
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end)) {
		end--;
	}
	end[1] = '\0';

	return str;
}

static int
parse_bool(const char *value) {
	return strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0;
}

static int
set_field(
		void *struct_ptr, const char *value,
		const struct FieldInfo *field_info) {
	void *field_ptr = (char *)struct_ptr + field_info->offset;

	switch (field_info->type) {
	case TYPE_STRING:
		free(field_ptr);
		field_ptr = strdup(value);
		if (!field_ptr) {
			return -1;
		}
		break;
	case TYPE_INT:
		*(int *)field_ptr = atoi(value);
		break;
	case TYPE_BOOL:
		*(int *)field_ptr = parse_bool(value);
		break;
	}
	return 0;
}

static int
parse_field(char *line, struct EditorConfig *config) {
	int rv = 0;
	char *delimiter = strchr(line, '=');
	if (!delimiter) {
		return -1;
	}

	delimiter[0] = '\0';
	const char *key = trim_str(line);
	const char *value = trim_str(&delimiter[1]);

	if (config->segment_count > 0) {
		struct EditorConfigSegment *segment =
				&config->segments[config->segment_count - 1];

		for (size_t i = 0; i < SEGMENT_FIELDS_LENGTH; i++) {
			if (strcasecmp(key, fields_info[i].name) != 0) {
				continue;
			}
			rv = set_field(segment, value, &fields_info[i]);
			if (rv < 0) {
				return -1;
			}
			break;
		}
	} else if (strcasecmp(key, "root") == 0) {
		config->root = parse_bool(value);
	}

	return rv;
}

static int
parse_segment(char *line, struct EditorConfig *config) {
	int rv = 0;
	size_t line_len = strlen(line);

	config->segment_count++;
	config->segments = reallocarray(
			config->segments, config->segment_count,
			sizeof(struct EditorConfigSegment));
	struct EditorConfigSegment *new_segment =
			&config->segments[config->segment_count - 1];
	if (!new_segment) {
		rv = -1;
		goto out;
	}
	new_segment->section = strndup(line + 1, line_len - 2);
	if (!new_segment->section) {
		rv = -1;
		goto out;
	}
out:
	free(new_segment->section);
	free(new_segment);
	return rv;
}

int
editorconfig_parse(struct EditorConfig *config, const char *filename) {
	int rv = 0;
	FILE *file = fopen(filename, "r");
	if (!file) {
		return -1;
	}

	char *line = NULL;
	size_t len = 0;

	while (getline(&line, &len, file) != -1) {
		size_t line_len = strcspn(line, "\r\n");
		line[line_len] = 0;

		if (line[0] == '[' && line[line_len - 1] == ']') {
			rv = parse_segment(line, config);
		} else if (line[0] != '\0' && line[0] != '#' && line[0] != ';') {
			rv = parse_field(line, config);
		}

		if (rv < 0) {
			goto out;
		}
	}

out:
	if (rv < 0) {
		editorconfig_cleanup(config);
	}
	free(line);
	fclose(file);
	return rv;
}

void
editorconfig_cleanup(struct EditorConfig *config) {
	for (size_t i = 0; i < config->segment_count; i++) {
		struct EditorConfigSegment *segment = &config->segments[i];
		free(segment->section);
		free(segment->indent_style);
		free(segment->end_of_line);
		free(segment->charset);
	}
	free(config->segments);
	config->segments = NULL;
}
