#include <lsp_generator.h>
#include <jw.h>
#include <quickjs.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
parse_json(JSContext *ctx, struct JwVal *val, const char *json) {
    struct Jw jw = {0};
    ASSERT_EQ(jw_init(&jw, ctx), 0);
    ASSERT_EQ(jw_parse(val, &jw, json, strlen(json)), 0);
    jw_deinit(&jw);
}

static void
test_type_basename_reference(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"some_type\"}");
    char *bn = type_basename(&val);
    ASSERT_STREQ("SomeType", bn);
    free(bn);
    jw_cleanup(&val);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_collect_deps_reference(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct NameList enums = {0};
    struct NameList deps = {0};
    add_name(&enums, "EnumType");

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"EnumType\"}");
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 0);

    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"MyStruct\"}");
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 1);
    ASSERT_STREQ(deps.names[0], "MyStruct");

    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"MyStruct\"}");
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 1);

    freelist(&deps);
    freelist(&enums);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_type_basename_array(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"array\",\"element\":{\"kind\":\"reference\",\"name\":\"my_type\"}}" );
    char *bn = type_basename(&val);
    ASSERT_STREQ("MyTypeArray", bn);
    free(bn);
    jw_cleanup(&val);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_type_basename_map(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"map\",\"key\":{\"kind\":\"reference\",\"name\":\"my_key\"},\"value\":{\"kind\":\"reference\",\"name\":\"my_value\"}}" );
    char *bn = type_basename(&val);
    ASSERT_STREQ("MapMyKeyToMyValue", bn);
    free(bn);
    jw_cleanup(&val);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_collect_deps_array(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct NameList enums = {0};
    struct NameList deps = {0};
    add_name(&enums, "EnumType");

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"array\",\"element\":{\"kind\":\"reference\",\"name\":\"EnumType\"}}" );
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 0);

    parse_json(ctx, &val, "{\"kind\":\"array\",\"element\":{\"kind\":\"reference\",\"name\":\"MyStruct\"}}" );
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 1);
    ASSERT_STREQ(deps.names[0], "MyStruct");

    parse_json(ctx, &val, "{\"kind\":\"array\",\"element\":{\"kind\":\"reference\",\"name\":\"MyStruct\"}}" );
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 1);

    freelist(&deps);
    freelist(&enums);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_collect_deps_map(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct NameList enums = {0};
    struct NameList deps = {0};
    add_name(&enums, "EnumType");

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"map\",\"key\":{\"kind\":\"reference\",\"name\":\"my_key\"},\"value\":{\"kind\":\"reference\",\"name\":\"EnumType\"}}" );
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 1);
    ASSERT_STREQ(deps.names[0], "my_key");

    parse_json(ctx, &val, "{\"kind\":\"map\",\"key\":{\"kind\":\"reference\",\"name\":\"my_key\"},\"value\":{\"kind\":\"reference\",\"name\":\"MyStruct\"}}" );
    collect_deps(&val, &enums, &deps);
    jw_cleanup(&val);
    ASSERT_EQ(deps.count, 2);
    ASSERT_STREQ(deps.names[0], "my_key");
    ASSERT_STREQ(deps.names[1], "MyStruct");

    freelist(&deps);
    freelist(&enums);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

DECLARE_TESTS
TEST(test_type_basename_reference)
TEST(test_type_basename_array)
TEST(test_type_basename_map)
TEST(test_collect_deps_reference)
TEST(test_collect_deps_array)
TEST(test_collect_deps_map)
END_TESTS
