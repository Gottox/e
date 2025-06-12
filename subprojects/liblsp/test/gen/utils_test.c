#include <lsp_generator.h>
#include <jw.h>
#include <quickjs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <testlib.h>

static void
test_string_helpers(void) {
    char buf[64];

    to_upper_snake("hello world-test", buf, sizeof(buf));
    ASSERT_STREQ("HELLO_WORLD_TEST", buf);

    to_snake_case("HelloWorld Test", buf, sizeof(buf));
    ASSERT_STREQ("hello_world_test", buf);

    to_upper_camel("hello_world test-case", buf, sizeof(buf));
    ASSERT_STREQ("HelloWorldTestCase", buf);
}

static void
test_read_file(void) {
    char tmpl[] = "/tmp/lsp_gen_testXXXXXX";
    int fd = mkstemp(tmpl);
    ASSERT_GT(fd, -1);
    const char *msg = "content";
    write(fd, msg, strlen(msg));
    close(fd);

    char *buf = read_file(tmpl);
    ASSERT_STREQ(msg, buf);
    free(buf);
    unlink(tmpl);
}

static void
parse_json(JSContext *ctx, struct JwVal *val, const char *json) {
    struct Jw jw = {0};
    ASSERT_EQ(jw_init(&jw, ctx), 0);
    ASSERT_EQ(jw_parse(val, &jw, json, strlen(json)), 0);
    jw_deinit(&jw);
}

static void
test_is_null_type(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"base\",\"name\":\"null\"}");
    ASSERT_TRUE(is_null_type(&val));
    jw_cleanup(&val);

    parse_json(ctx, &val, "{\"kind\":\"base\",\"name\":\"string\"}");
    ASSERT_FALSE(is_null_type(&val));
    jw_cleanup(&val);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}

static void
test_resolve_type(void) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);
    struct NameList enums = {0};
    add_name(&enums, "MyEnum");

    struct JwVal val = {0};
    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"MyEnum\"}");
    char *t = resolve_type(&val, &enums);
    ASSERT_STREQ("enum LspMyEnum", t);
    free(t);
    jw_cleanup(&val);

    parse_json(ctx, &val, "{\"kind\":\"reference\",\"name\":\"MyStruct\"}");
    t = resolve_type(&val, &enums);
    ASSERT_STREQ("struct LspMyStruct *", t);
    free(t);
    jw_cleanup(&val);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    freelist(&enums);
}

DECLARE_TESTS
TEST(test_string_helpers)
TEST(test_read_file)
TEST(test_is_null_type)
TEST(test_resolve_type)
END_TESTS
