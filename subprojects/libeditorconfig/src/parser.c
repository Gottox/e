#include <assert.h>
#include <ctype.h>
#include <editorconfig.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

enum FieldType { TYPE_STR, TYPE_INT, TYPE_BOOL };

struct FieldInfo {
	const char *name;
	size_t offset;
	enum FieldType type;
};

static struct FieldInfo fields_info[] = {
#define DEF(n, t) {#n, offsetof(struct EditorConfigSegment, n), TYPE_##t},
#include "fields.def.h"
#undef DEF
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
	case TYPE_STR:
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
	if (delimiter == NULL) {
		rv = -1;
		goto out;
	}

	delimiter[0] = '\0';
	const char *key = trim_str(line);
	const char *value = trim_str(&delimiter[1]);

	// Every key except "root" must be in a segment.
	if (config->segment_count > 0) {
		struct EditorConfigSegment *segment =
				&config->segments[config->segment_count - 1];

		for (size_t i = 0; i < SEGMENT_FIELDS_LENGTH; i++) {
			if (strcasecmp(key, fields_info[i].name) != 0) {
				continue;
			}

			rv = set_field(segment, value, &fields_info[i]);
			if (rv < 0) {
				goto out;
			}
			break;
		}
	} else if (strcasecmp(key, "root") == 0) {
		config->root = parse_bool(value);
	} else {
		rv = -1;
		goto out;
	}

out:
	return rv;
}

static int
parse_segment(char *line, size_t len, struct EditorConfig *config) {
	int rv = 0;
	struct EditorConfigSegment *new_segment = NULL;

	assert(len >= 2);

	config->segment_count++;
	config->segments = reallocarray(
			config->segments, config->segment_count,
			sizeof(struct EditorConfigSegment));
	if (!config->segments) {
		rv = -1;
		goto out;
	}

	new_segment = &config->segments[config->segment_count - 1];
	new_segment->section = strndup(line + 1, len - 2);
	if (!new_segment->section) {
		rv = -1;
		goto out;
	}

out:
	if (new_segment != NULL) {
		free(new_segment->section);
		free(new_segment);
	}
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
	char *trim = NULL;
	size_t len = 0;

	while (getline(&line, &len, file) != -1) {
		trim = line;
		while (isspace(*trim)) {
			trim++;
		}

		len = strcspn(trim, "\r\n");
		trim[len] = '\0';

		if (trim[0] == '[' && trim[len - 1] == ']') {
			rv = parse_segment(trim, len, config);
		} else if (trim[0] != '\0' && trim[0] != '#' && trim[0] != ';') {
			rv = parse_field(trim, config);
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

static void
free_data_segment(struct EditorConfigSegment *segment) {
#define DEF(n, t) \
	t(segment->n); \
	segment->n = 0;
#define STR free
#define INT (void)
#define BOOL (void)
#include "fields.def.h"
#undef STR
#undef INT
#undef BOOL
#undef DEF
}

void
editorconfig_cleanup(struct EditorConfig *config) {
	for (size_t i = 0; i < config->segment_count; i++) {
		struct EditorConfigSegment *segment = &config->segments[i];
		free(segment->section);
		free_data_segment(segment);
	}
	free(config->segments);
	config->segments = NULL;
}
