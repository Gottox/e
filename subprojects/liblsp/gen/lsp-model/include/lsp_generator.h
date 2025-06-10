#ifndef LSP_GEN_GENERATOR_H
#define LSP_GEN_GENERATOR_H

#include <jw.h>
#include <stdio.h>
#include <stddef.h>

// Utility helpers
char *read_file(const char *path);
void to_upper_snake(const char *in, char *out, size_t max);
void to_snake_case(const char *in, char *out, size_t max);
void to_upper_camel(const char *in, char *out, size_t max);
int is_null_type(struct JwVal *type);

struct NameList {
    char **names;
    size_t count;
};

struct OrEnum {
    char *t1;
    char *t2;
};

struct OrEnumList {
    struct OrEnum *items;
    size_t count;
};

struct TupleDef {
    char *name;
    char **field_types;
    size_t field_count;
};

struct TupleList {
    struct TupleDef *items;
    size_t count;
};

struct LiteralDef {
    char *name;
    char **field_names;
    char **field_types;
    size_t field_count;
};

struct LiteralList {
    struct LiteralDef *items;
    size_t count;
};

struct TypeCollector {
    struct NameList *enums;
    struct NameList *deps;
    struct OrEnumList *or_enums;
    struct TupleList *tuples;
    struct LiteralList *literals;
};

int add_name(struct NameList *l, const char *name);
void freelist(struct NameList *l);
int collect_names(struct JwVal *arr, struct NameList *out);
int name_in_list(struct NameList *l, const char *n);

int add_or_enum(struct OrEnumList *l, const char *t1, const char *t2);
void free_or_enum_list(struct OrEnumList *l);

int add_tuple(struct TupleList *l, struct JwVal *tuple, struct NameList *enums);
void free_tuple_list(struct TupleList *l);

int add_literal(struct LiteralList *l, struct JwVal *literal,
                struct NameList *enums);
void free_literal_list(struct LiteralList *l);

void collect_types(struct JwVal *type, struct TypeCollector *ctx);

char *type_basename(struct JwVal *type);
void collect_deps(struct JwVal *type, struct NameList *enums, struct NameList *deps);
char *resolve_type(struct JwVal *type, struct NameList *enums);

void generate_enums(struct JwVal *root, FILE *out);
void generate_structs(struct JwVal *root, FILE *out, struct NameList *enums);

#endif
