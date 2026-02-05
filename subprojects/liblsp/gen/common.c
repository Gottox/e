#include "common.h"

char *
str_to_snake_case(const char *str) {
	char *result = calloc(strlen(str) * 2 + 1, sizeof(char));
	assert(result != NULL);
	const char *p = str;
	char *q = result;
	for (; *p; p++, q++) {
		if (*p >= 'A' && *p <= 'Z' && p != str) {
			*q = '_';
			q++;
		}
		*q = tolower(*p);
	}
	*q = '\0';
	return result;
}

char *
str_to_upper_snake(const char *str) {
	char *result = str_to_snake_case(str);
	for (char *p = result; *p; p++) {
		*p = toupper(*p);
	}
	return result;
}

char *
str_capitalize(const char *str) {
	if (str == NULL || str[0] == '\0') {
		return strdup("");
	}
	char *result = strdup(str);
	assert(result != NULL);
	result[0] = toupper(result[0]);
	return result;
}

const char *
lsp_name_normalize(const char *name) {
	if (strcmp(name, "LSPErrorCodes") == 0) {
		return "LspErrorCodes";
	} else if (strncmp(name, "LSP", 3) == 0) {
		return name + 3;
	} else {
		return name;
	}
}

char *
to_struct_type(const char *lsp_name) {
	const char *stripped = lsp_name_normalize(lsp_name);
	char *result = NULL;
	int ret = asprintf(&result, "struct Lsp%s", stripped);
	assert(ret >= 0 && result != NULL);
	return result;
}

char *
to_enum_type(const char *lsp_name) {
	const char *stripped = lsp_name_normalize(lsp_name);
	char *result = NULL;
	int ret = asprintf(&result, "enum Lsp%s", stripped);
	assert(ret >= 0 && result != NULL);
	return result;
}

char *
to_func_name(const char *lsp_name) {
	const char *stripped = lsp_name_normalize(lsp_name);
	return str_to_snake_case(stripped);
}

char *
to_constant_name(const char *lsp_name) {
	const char *stripped = lsp_name_normalize(lsp_name);
	return str_to_upper_snake(stripped);
}

const char *
base_to_c_type(const char *base_name) {
	if (strcmp(base_name, "DocumentUri") == 0 ||
		strcmp(base_name, "URI") == 0 || strcmp(base_name, "string") == 0) {
		return "const char *";
	} else if (strcmp(base_name, "uinteger") == 0) {
		return "uint_least64_t";
	} else if (strcmp(base_name, "integer") == 0) {
		return "int_least64_t";
	} else if (strcmp(base_name, "boolean") == 0) {
		return "bool";
	} else if (strcmp(base_name, "decimal") == 0) {
		return "double";
	}
	return NULL;
}

const char *
base_type_json_suffix(const char *name) {
	if (strcmp(name, "DocumentUri") == 0 || strcmp(name, "URI") == 0 ||
		strcmp(name, "string") == 0) {
		return "string";
	} else if (strcmp(name, "uinteger") == 0) {
		return "uint64";
	} else if (strcmp(name, "integer") == 0) {
		return "int64";
	} else if (strcmp(name, "boolean") == 0) {
		return "boolean";
	} else if (strcmp(name, "decimal") == 0) {
		return "double";
	}
	return NULL;
}

const char *
base_type_json_type(const char *name) {
	if (strcmp(name, "DocumentUri") == 0 || strcmp(name, "URI") == 0 ||
		strcmp(name, "string") == 0) {
		return "json_type_string";
	} else if (strcmp(name, "uinteger") == 0 || strcmp(name, "integer") == 0) {
		return "json_type_int";
	} else if (strcmp(name, "boolean") == 0) {
		return "json_type_boolean";
	} else if (strcmp(name, "decimal") == 0) {
		return "json_type_double";
	} else if (strcmp(name, "null") == 0) {
		return "json_type_null";
	}
	return NULL;
}

const char *
strip_dollar_prefix(const char *method) {
	if (method != NULL && method[0] == '$' && method[1] == '/') {
		return method + 2;
	}
	return method;
}

char *
method_to_enum_suffix(const char *method) {
	const char *m = strip_dollar_prefix(method);
	size_t len = strlen(m);
	char *buf = calloc(len * 3 + 1, 1);
	assert(buf != NULL);
	char *dst = buf;

	for (const char *src = m; *src; src++) {
		if (*src == '/') {
			*dst++ = '_';
			*dst++ = '_';
		} else if (*src >= 'A' && *src <= 'Z' && src != m &&
				   *(src - 1) != '/') {
			*dst++ = '_';
			*dst++ = *src;
		} else {
			*dst++ = toupper(*src);
		}
	}
	*dst = '\0';
	return buf;
}

char *
method_to_func_prefix(const char *method) {
	char *buf = method_to_enum_suffix(method);
	for (char *p = buf; *p; p++) {
		*p = tolower(*p);
	}
	return buf;
}
