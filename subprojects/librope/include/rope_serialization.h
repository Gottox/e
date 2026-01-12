#ifndef ROPE_SERIALIZATION_H
#define ROPE_SERIALIZATION_H
#include <json.h>

struct RopeNode;
struct RopePool;

struct RopeNode *
rope_node_from_json(struct json_object *obj, struct RopePool *pool);

struct json_object *rope_node_to_json(struct RopeNode *node);

int rope_node_to_graphviz(struct RopeNode *root, const char *file);

#endif
