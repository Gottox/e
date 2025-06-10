#include <lsp_generator.h>
#include <stdlib.h>
#include <string.h>

int
add_name(struct NameList *l, const char *name) {
    if (!l || !name)
        return -1;
    for (size_t i = 0; i < l->count; ++i)
        if (strcmp(l->names[i], name) == 0)
            return 0;
    char **tmp = realloc(l->names, (l->count + 1) * sizeof(char *));
    if (!tmp)
        return -1;
    l->names = tmp;
    l->names[l->count] = strdup(name);
    if (!l->names[l->count])
        return -1;
    l->count++;
    return 0;
}

void
freelist(struct NameList *l) {
    for (size_t i = 0; i < l->count; ++i)
        free(l->names[i]);
    free(l->names);
    l->names = NULL;
    l->count = 0;
}

int
collect_names(struct JwVal *arr, struct NameList *out) {
    ssize_t n = jw_arr_len(arr);
    out->names = calloc(n, sizeof(char *));
    if (!out->names)
        return -1;
    out->count = n;
    for (ssize_t i = 0; i < n; ++i) {
        struct JwVal v = {0};
        jw_arr_get(arr, i, &v);
        char *name = NULL;
        size_t nl = 0;
        if (jw_obj_get_str(&v, "name", &name, &nl) == 0)
            out->names[i] = name;
        else
            out->names[i] = strdup("");
        jw_cleanup(&v);
    }
    return 0;
}

int
name_in_list(struct NameList *l, const char *n) {
    for (size_t i = 0; i < l->count; ++i)
        if (strcmp(l->names[i], n) == 0)
            return 1;
    return 0;
}

int
add_or_enum(struct OrEnumList *l, const char *t1, const char *t2) {
    if (!l || !t1 || !t2)
        return -1;
    for (size_t i = 0; i < l->count; ++i)
        if (strcmp(l->items[i].t1, t1) == 0 && strcmp(l->items[i].t2, t2) == 0)
            return 0;
    struct OrEnum *tmp = realloc(l->items, (l->count + 1) * sizeof(*tmp));
    if (!tmp)
        return -1;
    l->items = tmp;
    l->items[l->count].t1 = strdup(t1);
    l->items[l->count].t2 = strdup(t2);
    if (!l->items[l->count].t1 || !l->items[l->count].t2)
        return -1;
    l->count++;
    return 0;
}

void
free_or_enum_list(struct OrEnumList *l) {
    for (size_t i = 0; i < l->count; ++i) {
        free(l->items[i].t1);
        free(l->items[i].t2);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
}

int
add_tuple(struct TupleList *l, struct JwVal *tuple, struct NameList *enums) {
    if (!l || !tuple)
        return -1;
    struct JwVal items = {0};
    if (jw_obj_get(tuple, "items", &items) != 0)
        return -1;
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
    for (size_t i = 0; i < l->count; ++i) {
        if (strcmp(l->items[i].name, namebuf) == 0) {
            jw_cleanup(&items);
            return 0;
        }
    }
    struct TupleDef *tmp = realloc(l->items, (l->count + 1) * sizeof(*tmp));
    if (!tmp) {
        jw_cleanup(&items);
        return -1;
    }
    l->items = tmp;
    size_t idx = l->count++;
    l->items[idx].name = strdup(namebuf);
    l->items[idx].field_count = n;
    l->items[idx].field_types = calloc(n, sizeof(char *));
    if (!l->items[idx].field_types) {
        jw_cleanup(&items);
        return -1;
    }
    for (ssize_t i = 0; i < n; ++i) {
        struct JwVal it = {0};
        jw_arr_get(&items, i, &it);
        l->items[idx].field_types[i] = resolve_type(&it, enums);
        jw_cleanup(&it);
    }
    jw_cleanup(&items);
    return 0;
}

void
free_tuple_list(struct TupleList *l) {
    for (size_t i = 0; i < l->count; ++i) {
        for (size_t j = 0; j < l->items[i].field_count; ++j)
            free(l->items[i].field_types[j]);
        free(l->items[i].field_types);
        free(l->items[i].name);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
}

int
add_literal(struct LiteralList *l, struct JwVal *literal, struct NameList *enums) {
    if (!l || !literal)
        return -1;
    struct JwVal val = {0};
    if (jw_obj_get(literal, "value", &val) != 0)
        return -1;
    struct JwVal props = {0};
    if (jw_obj_get(&val, "properties", &props) != 0) {
        jw_cleanup(&val);
        return -1;
    }
    ssize_t n = jw_arr_len(&props);
    if (n == 0) {
        jw_cleanup(&props);
        jw_cleanup(&val);
        return 0;
    }
    char namebuf[512] = {0};
    for (ssize_t i = 0; i < n; ++i) {
        struct JwVal p = {0};
        jw_arr_get(&props, i, &p);
        char *pn = NULL;
        size_t pl = 0;
        if (jw_obj_get_str(&p, "name", &pn, &pl) == 0) {
            char tmp[256];
            to_upper_camel(pn, tmp, sizeof(tmp));
            strcat(namebuf, tmp);
            bool opt = false;
            if (jw_obj_get_bool(&p, "optional", &opt) == 0 && opt)
                strcat(namebuf, "Opt");
            free(pn);
        }
        jw_cleanup(&p);
    }
    for (size_t i = 0; i < l->count; ++i) {
        if (strcmp(l->items[i].name, namebuf) == 0) {
            jw_cleanup(&props);
            jw_cleanup(&val);
            return 0;
        }
    }
    struct LiteralDef *tmp = realloc(l->items, (l->count + 1) * sizeof(*tmp));
    if (!tmp) {
        jw_cleanup(&props);
        jw_cleanup(&val);
        return -1;
    }
    l->items = tmp;
    size_t idx = l->count++;
    l->items[idx].name = strdup(namebuf);
    l->items[idx].field_count = n;
    l->items[idx].field_names = calloc(n, sizeof(char *));
    l->items[idx].field_types = calloc(n, sizeof(char *));
    if (!l->items[idx].field_names || !l->items[idx].field_types) {
        jw_cleanup(&props);
        jw_cleanup(&val);
        return -1;
    }
    for (ssize_t i = 0; i < n; ++i) {
        struct JwVal p = {0};
        jw_arr_get(&props, i, &p);
        char *pn = NULL;
        size_t pl = 0;
        if (jw_obj_get_str(&p, "name", &pn, &pl) == 0) {
            char sn[256];
            to_snake_case(pn, sn, sizeof(sn));
            l->items[idx].field_names[i] = strdup(sn);
            free(pn);
        } else {
            l->items[idx].field_names[i] = strdup("field");
        }
        struct JwVal pt = {0};
        if (jw_obj_get(&p, "type", &pt) == 0) {
            l->items[idx].field_types[i] = resolve_type(&pt, enums);
            jw_cleanup(&pt);
        } else {
            l->items[idx].field_types[i] = strdup("void *");
        }
        jw_cleanup(&p);
    }
    jw_cleanup(&props);
    jw_cleanup(&val);
    return 0;
}

void
free_literal_list(struct LiteralList *l) {
    for (size_t i = 0; i < l->count; ++i) {
        for (size_t j = 0; j < l->items[i].field_count; ++j) {
            free(l->items[i].field_names[j]);
            free(l->items[i].field_types[j]);
        }
        free(l->items[i].field_names);
        free(l->items[i].field_types);
        free(l->items[i].name);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
}

void
collect_types(struct JwVal *type, struct TypeCollector *ctx) {
    char *kind = NULL;
    size_t kl = 0;
    if (!type || !ctx || jw_obj_get_str(type, "kind", &kind, &kl) != 0)
        return;

    if (strcmp(kind, "reference") == 0) {
        if (ctx->deps) {
            char *name = NULL;
            if (jw_obj_get_str(type, "name", &name, &kl) == 0 &&
                !name_in_list(ctx->enums, name))
                add_name(ctx->deps, name);
            free(name);
        }
    } else if (strcmp(kind, "array") == 0) {
        struct JwVal elem = {0};
        if (jw_obj_get(type, "element", &elem) == 0) {
            collect_types(&elem, ctx);
            jw_cleanup(&elem);
        }
    } else if (strcmp(kind, "map") == 0) {
        struct JwVal key = {0}, value = {0};
        if (jw_obj_get(type, "key", &key) == 0 &&
            jw_obj_get(type, "value", &value) == 0) {
            collect_types(&key, ctx);
            collect_types(&value, ctx);
            jw_cleanup(&key);
            jw_cleanup(&value);
        }
    } else if (strcmp(kind, "or") == 0 || strcmp(kind, "and") == 0 ||
               strcmp(kind, "tuple") == 0) {
        struct JwVal items = {0};
        if (jw_obj_get(type, "items", &items) == 0) {
            ssize_t n = jw_arr_len(&items);
            if (ctx->or_enums && strcmp(kind, "or") == 0 && n == 2) {
                struct JwVal i1 = {0}, i2 = {0};
                jw_arr_get(&items, 0, &i1);
                jw_arr_get(&items, 1, &i2);
                if (!is_null_type(&i1) && !is_null_type(&i2)) {
                    char *n1 = type_basename(&i1);
                    char *n2 = type_basename(&i2);
                    add_or_enum(ctx->or_enums, n1, n2);
                    free(n1);
                    free(n2);
                }
                jw_cleanup(&i1);
                jw_cleanup(&i2);
            }
            for (ssize_t i = 0; i < n; ++i) {
                struct JwVal it = {0};
                jw_arr_get(&items, i, &it);
                collect_types(&it, ctx);
                jw_cleanup(&it);
            }
            jw_cleanup(&items);
        }
        if (strcmp(kind, "tuple") == 0 && ctx->tuples)
            add_tuple(ctx->tuples, type, ctx->enums);
    } else if (strcmp(kind, "literal") == 0) {
        struct JwVal val = {0};
        if (jw_obj_get(type, "value", &val) == 0) {
            struct JwVal props = {0};
            if (jw_obj_get(&val, "properties", &props) == 0) {
                ssize_t pc = jw_arr_len(&props);
                for (ssize_t i = 0; i < pc; ++i) {
                    struct JwVal p = {0};
                    jw_arr_get(&props, i, &p);
                    struct JwVal pt = {0};
                    if (jw_obj_get(&p, "type", &pt) == 0) {
                        collect_types(&pt, ctx);
                        jw_cleanup(&pt);
                    }
                    jw_cleanup(&p);
                }
                if (ctx->literals)
                    add_literal(ctx->literals, type, ctx->enums);
                jw_cleanup(&props);
            }
            jw_cleanup(&val);
        }
    }

    free(kind);
}
