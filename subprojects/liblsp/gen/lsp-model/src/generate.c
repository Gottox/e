#include <lsp_generator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StructInfo {
    char *name;
    struct NameList deps;
    ssize_t index;
};

static void
generate_enum(const char *ename, struct JwVal *vals, FILE *out) {
    fprintf(out, "enum Lsp%s {\n", ename);
    ssize_t vcount = jw_arr_len(vals);
    char enum_sn[256];
    to_upper_snake(ename, enum_sn, sizeof(enum_sn));
    for (ssize_t j = 0; j < vcount; ++j) {
        struct JwVal v = {0};
        jw_arr_get(vals, j, &v);
        char *vname = NULL;
        size_t vl = 0;
        if (jw_obj_get_str(&v, "name", &vname, &vl) == 0) {
            char val_sn[256];
            to_upper_snake(vname, val_sn, sizeof(val_sn));
            fprintf(out, "    LSP_%s_%s%s", enum_sn, val_sn,
                    j + 1 == vcount ? "" : ",\n");
            free(vname);
        }
        jw_cleanup(&v);
    }
    fprintf(out, "\n};\n\n");
}

void
generate_enums(struct JwVal *root, FILE *out) {
    struct JwVal enums = {0};
    if (jw_obj_get(root, "enumerations", &enums) != 0)
        return;
    ssize_t count = jw_arr_len(&enums);
    for (ssize_t i = 0; i < count; ++i) {
        struct JwVal e = {0};
        jw_arr_get(&enums, i, &e);
        char *ename = NULL;
        size_t el = 0;
        if (jw_obj_get_str(&e, "name", &ename, &el) != 0) {
            jw_cleanup(&e);
            continue;
        }
        struct JwVal vals = {0};
        if (jw_obj_get(&e, "values", &vals) == 0) {
            generate_enum(ename, &vals, out);
            jw_cleanup(&vals);
        }
        free(ename);
        jw_cleanup(&e);
    }
    jw_cleanup(&enums);
}

static void
generate_struct_body(struct JwVal *structs, struct StructInfo *info, FILE *out,
                     struct NameList *enums) {
    struct JwVal s = {0};
    jw_arr_get(structs, info->index, &s);
    fprintf(out, "struct Lsp%s {\n", info->name);

    struct JwVal exts = {0};
    if (jw_obj_get(&s, "extends", &exts) == 0) {
        ssize_t ec = jw_arr_len(&exts);
        if (ec > 0)
            fprintf(out, "    //extends\n");
        for (ssize_t j = 0; j < ec; ++j) {
            struct JwVal ex = {0};
            jw_arr_get(&exts, j, &ex);
            char *name = NULL;
            size_t nl = 0;
            if (jw_obj_get_str(&ex, "name", &name, &nl) == 0) {
                char snake[256];
                to_snake_case(name, snake, sizeof(snake));
                fprintf(out, "    struct Lsp%s %s;\n", name, snake);
                free(name);
            }
            jw_cleanup(&ex);
        }
        if (ec > 0)
            fprintf(out, "\n");
        jw_cleanup(&exts);
    }

    struct JwVal mix = {0};
    if (jw_obj_get(&s, "mixins", &mix) == 0) {
        ssize_t mc = jw_arr_len(&mix);
        if (mc > 0)
            fprintf(out, "    //mixins\n");
        for (ssize_t j = 0; j < mc; ++j) {
            struct JwVal mx = {0};
            jw_arr_get(&mix, j, &mx);
            char *name = NULL;
            size_t nl = 0;
            if (jw_obj_get_str(&mx, "name", &name, &nl) == 0) {
                char snake[256];
                to_snake_case(name, snake, sizeof(snake));
                fprintf(out, "    struct Lsp%s %s;\n", name, snake);
                free(name);
            }
            jw_cleanup(&mx);
        }
        if (mc > 0)
            fprintf(out, "\n");
        jw_cleanup(&mix);
    }

    struct JwVal props = {0};
    if (jw_obj_get(&s, "properties", &props) == 0) {
        ssize_t pcount = jw_arr_len(&props);
        for (ssize_t j = 0; j < pcount; ++j) {
            struct JwVal p = {0};
            jw_arr_get(&props, j, &p);
            char *pname = NULL;
            size_t pl = 0;
            if (jw_obj_get_str(&p, "name", &pname, &pl) == 0) {
                char pname_sn[256];
                to_snake_case(pname, pname_sn, sizeof(pname_sn));
                struct JwVal ptype = {0};
                if (jw_obj_get(&p, "type", &ptype) == 0) {
                    char *kind = NULL;
                    size_t kl = 0;
                    if (jw_obj_get_str(&ptype, "kind", &kind, &kl) == 0 &&
                        strcmp(kind, "array") == 0) {
                        struct JwVal elem = {0};
                        if (jw_obj_get(&ptype, "element", &elem) == 0) {
                            char *inner = resolve_type(&elem, enums);
                            fprintf(out, "    %s *%s;\n", inner, pname_sn);
                            fprintf(out, "    size_t %s_count;\n", pname_sn);
                            free(inner);
                            jw_cleanup(&elem);
                        }
                    } else if (strcmp(kind, "or") == 0) {
                        struct JwVal items = {0};
                        if (jw_obj_get(&ptype, "items", &items) == 0 &&
                            jw_arr_len(&items) == 2) {
                            struct JwVal it1 = {0}, it2 = {0};
                            jw_arr_get(&items, 0, &it1);
                            jw_arr_get(&items, 1, &it2);
                            int null1 = is_null_type(&it1);
                            int null2 = is_null_type(&it2);
                            if ((null1 && !null2) || (!null1 && null2)) {
                                struct JwVal *real = null1 ? &it2 : &it1;
                                char *ctype = resolve_type(real, enums);
                                if (ctype[strlen(ctype) - 1] == '*')
                                    fprintf(out, "    %s %s;\n", ctype, pname_sn);
                                else
                                    fprintf(out, "    %s *%s;\n", ctype, pname_sn);
                                free(ctype);
                                jw_cleanup(&it1);
                                jw_cleanup(&it2);
                            } else {
                                char *t1 = resolve_type(&it1, enums);
                                char *t2 = resolve_type(&it2, enums);
                                char *n1 = type_basename(&it1);
                                char *n2 = type_basename(&it2);
                                char name1_sn[256], name2_sn[256];
                                to_snake_case(n1, name1_sn, sizeof(name1_sn));
                                to_snake_case(n2, name2_sn, sizeof(name2_sn));
                                fprintf(out, "    enum Lsp%sOr%s %s_enum;\n", n1, n2, pname_sn);
                                fprintf(out, "    union { %s %s; %s %s; } %s;\n", t1, name1_sn, t2, name2_sn, pname_sn);
                                free(t1);
                                free(t2);
                                free(n1);
                                free(n2);
                                jw_cleanup(&it1);
                                jw_cleanup(&it2);
                            }
                        }
                        jw_cleanup(&items);
                    } else {
                        char *ctype = resolve_type(&ptype, enums);
                        fprintf(out, "    %s %s;\n", ctype, pname_sn);
                        free(ctype);
                    }
                    free(kind);
                    jw_cleanup(&ptype);
                }
            }
            free(pname);
            jw_cleanup(&p);
        }
        jw_cleanup(&props);
    }
    fprintf(out, "};\n\n");
    jw_cleanup(&s);
}

void
generate_structs(struct JwVal *root, FILE *out, struct NameList *enums) {
    struct JwVal structs = {0};
    if (jw_obj_get(root, "structures", &structs) != 0)
        return;
    ssize_t count = jw_arr_len(&structs);

    struct NameList struct_names = {0};
    collect_names(&structs, &struct_names);

    struct StructInfo *infos = calloc(count, sizeof(*infos));
    struct OrEnumList or_enums = {0};
    struct TupleList tuple_defs = {0};
    struct LiteralList literal_defs = {0};
    struct TypeCollector collector = {
        .enums = enums,
        .deps = NULL,
        .or_enums = &or_enums,
        .tuples = &tuple_defs,
        .literals = &literal_defs,
    };
    for (ssize_t i = 0; i < count; ++i) {
        struct JwVal s = {0};
        jw_arr_get(&structs, i, &s);
        char *sname = NULL;
        size_t sl = 0;
        if (jw_obj_get_str(&s, "name", &sname, &sl) != 0) {
            jw_cleanup(&s);
            continue;
        }
        infos[i].name = sname;
        infos[i].index = i;

        struct JwVal props = {0};
        if (jw_obj_get(&s, "properties", &props) == 0) {
            ssize_t pcount = jw_arr_len(&props);
            for (ssize_t j = 0; j < pcount; ++j) {
                struct JwVal p = {0};
                jw_arr_get(&props, j, &p);
                struct JwVal pt = {0};
                if (jw_obj_get(&p, "type", &pt) == 0) {
                    collector.deps = &infos[i].deps;
                    collect_types(&pt, &collector);
                    collector.deps = NULL;
                    jw_cleanup(&pt);
                }
                jw_cleanup(&p);
            }
            jw_cleanup(&props);
        }

        struct JwVal exts = {0};
        if (jw_obj_get(&s, "extends", &exts) == 0) {
            ssize_t ec = jw_arr_len(&exts);
            for (ssize_t j = 0; j < ec; ++j) {
                struct JwVal ex = {0};
                jw_arr_get(&exts, j, &ex);
                collector.deps = &infos[i].deps;
                collect_types(&ex, &collector);
                collector.deps = NULL;
                jw_cleanup(&ex);
            }
            jw_cleanup(&exts);
        }

        struct JwVal mixins = {0};
        if (jw_obj_get(&s, "mixins", &mixins) == 0) {
            ssize_t mc = jw_arr_len(&mixins);
            for (ssize_t j = 0; j < mc; ++j) {
                struct JwVal mx = {0};
                jw_arr_get(&mixins, j, &mx);
                collector.deps = &infos[i].deps;
                collect_types(&mx, &collector);
                collector.deps = NULL;
                jw_cleanup(&mx);
            }
            jw_cleanup(&mixins);
        }
        jw_cleanup(&s);
    }

    size_t *order = calloc(count, sizeof(size_t));
    int *processed = calloc(count, sizeof(int));
    size_t produced = 0;
    while (produced < (size_t)count) {
        int progress = 0;
        for (ssize_t i = 0; i < count; ++i) {
            if (processed[i])
                continue;
            int ready = 1;
            for (size_t d = 0; d < infos[i].deps.count; ++d) {
                const char *dep = infos[i].deps.names[d];
                for (ssize_t k = 0; k < count; ++k) {
                    if (strcmp(infos[k].name, dep) == 0 && !processed[k]) {
                        ready = 0;
                        break;
                    }
                }
                if (!ready)
                    break;
            }
            if (ready) {
                order[produced++] = i;
                processed[i] = 1;
                progress = 1;
            }
        }
        if (!progress)
            break;
    }

    size_t cycle_start = produced;
    for (ssize_t i = 0; i < count; ++i) {
        if (!processed[i]) {
            order[produced++] = i;
            processed[i] = 1;
        }
    }

    if (cycle_start < (size_t)count) {
        for (size_t idx = cycle_start; idx < (size_t)count; ++idx)
            fprintf(out, "struct Lsp%s;\n", infos[order[idx]].name);
        fprintf(out, "\n");
    }

    for (size_t i = 0; i < or_enums.count; ++i) {
        char up1[256];
        char up2[256];
        to_upper_snake(or_enums.items[i].t1, up1, sizeof(up1));
        to_upper_snake(or_enums.items[i].t2, up2, sizeof(up2));
        fprintf(out, "enum Lsp%sOr%s {\n", or_enums.items[i].t1, or_enums.items[i].t2);
        fprintf(out, "    LSP_%s_OR_%s_%s,\n", up1, up2, up1);
        fprintf(out, "    LSP_%s_OR_%s_%s\n", up1, up2, up2);
        fprintf(out, "};\n\n");
    }

    for (size_t i = 0; i < tuple_defs.count; ++i) {
        fprintf(out, "struct LspTup%s {\n", tuple_defs.items[i].name);
        for (size_t j = 0; j < tuple_defs.items[i].field_count; ++j)
            fprintf(out, "    %s f%zu;\n", tuple_defs.items[i].field_types[j], j + 1);
        fprintf(out, "};\n\n");
    }

    for (size_t i = 0; i < literal_defs.count; ++i) {
        fprintf(out, "struct LspLit%s {\n", literal_defs.items[i].name);
        for (size_t j = 0; j < literal_defs.items[i].field_count; ++j)
            fprintf(out, "    %s %s;\n", literal_defs.items[i].field_types[j],
                    literal_defs.items[i].field_names[j]);
        fprintf(out, "};\n\n");
    }

    for (size_t idx = 0; idx < (size_t)count; ++idx)
        generate_struct_body(&structs, &infos[order[idx]], out, enums);

    for (ssize_t i = 0; i < count; ++i) {
        freelist(&infos[i].deps);
        free(infos[i].name);
    }
    free_or_enum_list(&or_enums);
    free_tuple_list(&tuple_defs);
    free_literal_list(&literal_defs);
    free(infos);
    free(order);
    free(processed);
    freelist(&struct_names);
    jw_cleanup(&structs);
}
