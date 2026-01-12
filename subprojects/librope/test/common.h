#include <rope.h>
#include <rope_serialization.h>
#include <string.h>
#include <testlib.h>

static inline char *
to_str(struct RopeNode *node) {
	json_object *json = rope_node_to_json(node);
	const char *str = json_object_to_json_string(json);
	char *result = cx_memdup(str, strlen(str) + 1);
	json_object_put(json);
	return result;
}

static inline void
assert_json(const char *expected, struct RopeNode *actual) {
	char *actual_str = to_str(actual);

	json_object *expected_json = json_tokener_parse(expected);
	ASSERT_STREQ(json_object_to_json_string(expected_json), actual_str);
	json_object_put(expected_json);
	free(actual_str);
}

static inline struct RopeNode *
from_str(struct RopePool *pool, const char *str) {
	json_object *json = json_tokener_parse(str);
	struct RopeNode *node = rope_node_from_json(json, pool);
	json_object_put(json);
	return node;
}
