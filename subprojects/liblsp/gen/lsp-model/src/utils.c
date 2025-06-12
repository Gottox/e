#include <ctype.h>
#include <lsp_generator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
read_file(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = calloc(sz + 1, 1);
	if (!buf) {
		fclose(f);
		return NULL;
	}
	fread(buf, 1, sz, f);
	fclose(f);
	buf[sz] = '\0';
	return buf;
}

void
to_upper_snake(const char *in, char *out, size_t max) {
	size_t j = 0;
	for (size_t i = 0; in[i] && j + 1 < max; ++i) {
		char c = in[i];
		if (isalnum((unsigned char)c))
			out[j++] = toupper((unsigned char)c);
		else
			out[j++] = '_';
	}
	out[j] = '\0';
}

void
to_snake_case(const char *in, char *out, size_t max) {
	size_t j = 0;
	int prev_lower = 0;
	for (size_t i = 0; in[i] && j + 1 < max; ++i) {
		unsigned char c = in[i];
		if (isalnum(c)) {
			if (isupper(c) && prev_lower && j + 1 < max)
				out[j++] = '_';
			out[j++] = tolower(c);
			prev_lower = islower(c) || isdigit(c);
		} else {
			if (j > 0 && out[j - 1] != '_' && j + 1 < max)
				out[j++] = '_';
			prev_lower = 0;
		}
	}
	out[j] = '\0';
}

void
to_upper_camel(const char *in, char *out, size_t max) {
	size_t j = 0;
	int cap = 1;
	for (size_t i = 0; in[i] && j + 1 < max; ++i) {
		unsigned char c = in[i];
		if (!isalnum(c)) {
			cap = 1;
			continue;
		}
		if (cap) {
			out[j++] = toupper(c);
			cap = 0;
		} else {
			out[j++] = c;
		}
	}
	out[j] = '\0';
}

int
is_null_type(struct JwVal *type) {
	char *kind = NULL;
	size_t kl = 0;
	if (!type || jw_obj_get_str(type, "kind", &kind, &kl) != 0)
		return 0;
	int res = 0;
	if (strcmp(kind, "base") == 0) {
		char *name = NULL;
		if (jw_obj_get_str(type, "name", &name, &kl) == 0 &&
			strcmp(name, "null") == 0)
			res = 1;
		free(name);
	}
	free(kind);
	return res;
}
