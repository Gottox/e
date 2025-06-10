#include <lsp_generator.h>
#include <jw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <metaModel.json>\n", argv[0]);
        return 1;
    }
    char *json = read_file(argv[1]);
    if (!json) {
        fprintf(stderr, "Failed to read %s\n", argv[1]);
        return 1;
    }
    JSRuntime *rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "Failed to create JS runtime\n");
        free(json);
        return 1;
    }
    JSContext *ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "Failed to create JS context\n");
        JS_FreeRuntime(rt);
        free(json);
        return 1;
    }
    struct Jw jw = {0};
    if (jw_init(&jw, ctx) != 0) {
        fprintf(stderr, "jw_init failed\n");
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        free(json);
        return 1;
    }
    struct JwVal root = {0};
    if (jw_parse(&root, &jw, json, strlen(json)) != 0) {
        fprintf(stderr, "Failed to parse json\n");
        jw_deinit(&jw);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        free(json);
        return 1;
    }
    FILE *out = stdout;

    fprintf(out, "// Generated from %s\n\n", argv[1]);
    fprintf(out, "#include <stdbool.h>\n");
    fprintf(out, "#include <stddef.h>\n\n");

    struct NameList enum_names = {0};
    struct JwVal enums_val = {0};
    if (jw_obj_get(&root, "enumerations", &enums_val) == 0) {
        collect_names(&enums_val, &enum_names);
        jw_cleanup(&enums_val);
    }

    generate_enums(&root, out);
    generate_structs(&root, out, &enum_names);

    freelist(&enum_names);
    jw_cleanup(&root);
    jw_deinit(&jw);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    free(json);
    return 0;
}
