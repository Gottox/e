#include <assert.h>
#include <json.h>
#include <rope.h>

struct RopeNode *
rope_node_from_json(struct json_object *obj, struct RopePool *pool) {
	if (obj == NULL || pool == NULL) {
		return NULL;
	}

	struct RopeNode *node = rope_pool_get(pool);
	if (!node)
		return NULL;

	enum json_type type = json_object_get_type(obj);

	if (type == json_type_array) {
		int length = json_object_array_length(obj);
		assert(length == 2);

		struct json_object *left_json = json_object_array_get_idx(obj, 0);
		struct json_object *right_json = json_object_array_get_idx(obj, 1);

		struct RopeNode *left = rope_node_from_json(left_json, pool);
		struct RopeNode *right = rope_node_from_json(right_json, pool);
		size_t depth = CX_MAX(rope_node_depth(left), rope_node_depth(right)) + 1;
		node->tags = (uint64_t)ROPE_NODE_BRANCH << 62 | (uint64_t)depth;
		
		left->parent = node;
		right->parent = node;
		node->data.branch.children[ROPE_LEFT] = left;
		node->data.branch.children[ROPE_RIGHT] = right;
	} else if (type == json_type_string) {
		int len = json_object_get_string_len(obj);
		const char *str = json_object_get_string(obj);

		rope_node_set_value(node, (const uint8_t *)str, (size_t)len);
	}

	return node;
}

struct json_object *
rope_node_to_json(struct RopeNode *node) {
	if (node == NULL) {
		return NULL;
	}

	enum RopeNodeType type = rope_node_type(node);

	if (type == ROPE_NODE_BRANCH) {
		// Create a JSON array for branches: [left, right]
		struct json_object *jarray = json_object_new_array();

		struct RopeNode *left = rope_node_left(node);
		struct RopeNode *right = rope_node_right(node);

		json_object_array_add(jarray, rope_node_to_json(left));
		json_object_array_add(jarray, rope_node_to_json(right));

		return jarray;
	} else {
		// Handle both ROPE_NODE_LEAF and ROPE_NODE_INLINE_LEAF
		size_t size = 0;
		const uint8_t *val = rope_node_value(node, &size);

		// json_object_new_string_len ensures we respect the byte_size
		// and handle non-null terminated data correctly.
		return json_object_new_string_len((const char *)val, (int)size);
	}
}
