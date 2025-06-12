#include <lsp_generator.h>
#include <stdlib.h>
#include <string.h>

static const char *
map_base_type(const char *name) {
	if (strcmp(name, "string") == 0 || strcmp(name, "DocumentUri") == 0 ||
		strcmp(name, "URI") == 0)
		return "char *";
	if (strcmp(name, "integer") == 0)
		return "int";
	if (strcmp(name, "uinteger") == 0)
		return "unsigned int";
	if (strcmp(name, "decimal") == 0 || strcmp(name, "number") == 0)
		return "double";
	if (strcmp(name, "boolean") == 0)
		return "bool";
	return "void *";
}

char *
type_basename(struct JwVal *type) {
	char *kind = NULL;
	size_t kl = 0;
	if (jw_obj_get_str(type, "kind", &kind, &kl) != 0)
		return strdup("Anon");

	char *name = NULL;
	if (strcmp(kind, "reference") == 0 || strcmp(kind, "base") == 0) {
		if (jw_obj_get_str(type, "name", &name, &kl) == 0) {
			char tmp[256];
			to_upper_camel(name, tmp, sizeof(tmp));
			char *res = strdup(tmp);
			free(name);
			free(kind);
			return res;
		}
	} else if (strcmp(kind, "array") == 0) {
		struct JwVal elem = {0};
		if (jw_obj_get(type, "element", &elem) == 0) {
			char *inner = type_basename(&elem);
			size_t sz = strlen(inner) + 6; // "Array" + nul
			char *res = malloc(sz);
			snprintf(res, sz, "%sArray", inner);
			free(inner);
			jw_cleanup(&elem);
			free(kind);
			return res;
		}
	} else if (strcmp(kind, "map") == 0) {
		struct JwVal key = {0}, value = {0};
		if (jw_obj_get(type, "key", &key) == 0 &&
			jw_obj_get(type, "value", &value) == 0) {
			char *kbn = type_basename(&key);
			char *vbn = type_basename(&value);
			size_t sz = strlen(kbn) + strlen(vbn) + 6; // "Map" + "To" + nul
			char *res = malloc(sz);
			snprintf(res, sz, "Map%sTo%s", kbn, vbn);
			free(kbn);
			free(vbn);
			jw_cleanup(&key);
			jw_cleanup(&value);
			free(kind);
			return res;
		}
	} else if (strcmp(kind, "tuple") == 0) {
		struct JwVal items = {0};
		if (jw_obj_get(type, "items", &items) == 0) {
			ssize_t n = jw_arr_len(&items);
			char namebuf[512] = {0};
			for (ssize_t i = 0; i < n; ++i) {
				struct JwVal it = {0};
				jw_arr_get(&items, i, &it);
				char *bn = type_basename(&it);
				strcat(namebuf, bn);
				free(bn);
				jw_cleanup(&it);
			}
			size_t sz = strlen(namebuf) + 4; // "Tup" + nul
			char *res = malloc(sz);
			snprintf(res, sz, "Tup%s", namebuf);
			jw_cleanup(&items);
			free(kind);
			return res;
		}
	} else if (strcmp(kind, "literal") == 0) {
		struct JwVal val = {0};
		if (jw_obj_get(type, "value", &val) == 0) {
			struct JwVal props = {0};
			if (jw_obj_get(&val, "properties", &props) == 0) {
				ssize_t pc = jw_arr_len(&props);
				if (pc > 0) {
					char namebuf[512] = {0};
					for (ssize_t i = 0; i < pc; ++i) {
						struct JwVal p = {0};
						jw_arr_get(&props, i, &p);
						char *pn = NULL;
						if (jw_obj_get_str(&p, "name", &pn, &kl) == 0) {
							char tmp[256];
							to_upper_camel(pn, tmp, sizeof(tmp));
							strcat(namebuf, tmp);
							bool opt = false;
							if (jw_obj_get_bool(&p, "optional", &opt) == 0 &&
								opt)
								strcat(namebuf, "Opt");
							free(pn);
						}
						jw_cleanup(&p);
					}
					size_t sz = strlen(namebuf) + 4; // "Lit" + nul
					char *res = malloc(sz);
					snprintf(res, sz, "Lit%s", namebuf);
					jw_cleanup(&props);
					jw_cleanup(&val);
					free(kind);
					return res;
				} else {
					jw_cleanup(&props);
					jw_cleanup(&val);
					free(kind);
					return strdup("Object");
				}
			}
			jw_cleanup(&val);
		}
	} else if (strcmp(kind, "stringLiteral") == 0) {
		free(kind);
		return strdup("String");
	}

	free(kind);
	return strdup("Anon");
}

void
collect_deps(
		struct JwVal *type, struct NameList *enums, struct NameList *deps) {
	struct TypeCollector ctx = {
			.enums = enums,
			.deps = deps,
			.or_enums = NULL,
			.tuples = NULL,
			.literals = NULL,
	};
	collect_types(type, &ctx);
}

char *
resolve_type(struct JwVal *type, struct NameList *enums) {
	char *kind = NULL;
	size_t len = 0;
	if (jw_obj_get_str(type, "kind", &kind, &len) < 0)
		return strdup("void *");
	char *result = NULL;
	if (strcmp(kind, "base") == 0) {
		char *name = NULL;
		size_t nl = 0;
		if (jw_obj_get_str(type, "name", &name, &nl) == 0)
			result = strdup(map_base_type(name));
		free(name);
	} else if (strcmp(kind, "reference") == 0) {
		char *name = NULL;
		size_t nl = 0;
		if (jw_obj_get_str(type, "name", &name, &nl) == 0) {
			if (name_in_list(enums, name)) {
				result = malloc(nl + 12);
				sprintf(result, "enum Lsp%s", name);
			} else {
				result = malloc(nl + 15);
				sprintf(result, "struct Lsp%s *", name);
			}
		}
		free(name);
	} else if (strcmp(kind, "array") == 0) {
		struct JwVal elem = {0};
		if (jw_obj_get(type, "element", &elem) == 0) {
			char *inner = resolve_type(&elem, enums);
			size_t sz = strlen(inner) + 3;
			result = malloc(sz);
			snprintf(result, sz, "%s *", inner);
			free(inner);
			jw_cleanup(&elem);
		}
	} else if (strcmp(kind, "map") == 0) {
		struct JwVal key = {0};
		struct JwVal value = {0};
		if (jw_obj_get(type, "key", &key) == 0 &&
			jw_obj_get(type, "value", &value) == 0) {
			char *kt = resolve_type(&key, enums);
			char *vt = resolve_type(&value, enums);
			size_t sz = strlen(kt) + strlen(vt) + 50;
			result = malloc(sz);
			snprintf(
					result, sz, "struct { size_t count; %s *keys; %s *items; }",
					kt, vt);
			free(kt);
			free(vt);
			jw_cleanup(&key);
			jw_cleanup(&value);
		}
	} else if (strcmp(kind, "tuple") == 0) {
		struct JwVal items = {0};
		if (jw_obj_get(type, "items", &items) == 0) {
			ssize_t n = jw_arr_len(&items);
			char name[512] = {0};
			for (ssize_t i = 0; i < n; ++i) {
				struct JwVal it = {0};
				jw_arr_get(&items, i, &it);
				char *bn = type_basename(&it);
				strcat(name, bn);
				free(bn);
				jw_cleanup(&it);
			}
			size_t sz = strlen(name) + 16;
			result = malloc(sz);
			snprintf(result, sz, "struct LspTup%s", name);
			jw_cleanup(&items);
		}
	} else if (strcmp(kind, "literal") == 0) {
		struct JwVal val = {0};
		if (jw_obj_get(type, "value", &val) == 0) {
			struct JwVal props = {0};
			if (jw_obj_get(&val, "properties", &props) == 0) {
				ssize_t pc = jw_arr_len(&props);
				if (pc > 0) {
					char name[512] = {0};
					for (ssize_t i = 0; i < pc; ++i) {
						struct JwVal p = {0};
						jw_arr_get(&props, i, &p);
						char *pn = NULL;
						size_t pl = 0;
						if (jw_obj_get_str(&p, "name", &pn, &pl) == 0) {
							char tmp[256];
							to_upper_camel(pn, tmp, sizeof(tmp));
							strcat(name, tmp);
							bool opt = false;
							if (jw_obj_get_bool(&p, "optional", &opt) == 0 &&
								opt) {
								strcat(name, "Opt");
							}
							free(pn);
						}
						jw_cleanup(&p);
					}
					size_t sz = strlen(name) + 16;
					result = malloc(sz);
					snprintf(result, sz, "struct LspLit%s", name);
				} else {
					result = strdup("void *");
				}
				jw_cleanup(&props);
			} else {
				result = strdup("char *");
			}
			jw_cleanup(&val);
		} else {
			result = strdup("char *");
		}
	} else if (strcmp(kind, "stringLiteral") == 0) {
		result = strdup("char *");
	} else {
		result = strdup("void *");
	}
	free(kind);
	if (!result)
		result = strdup("void *");
	return result;
}
