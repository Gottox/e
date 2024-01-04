#ifndef E_TREE_H
#define E_TREE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CxBuffer;

struct ETreeVisitorImpl {
	void *(*down)(void *node, void *user_data);
	void *(*up)(void *node, void *user_data);
	void *(*next_sibling)(void *node, void *user_data);
	int (*label)(void *node, struct CxBuffer *buffer, void *user_data);
};

extern const struct ETreeVisitorImpl e_tree_visitor_rope_impl;

int e_tree_view_ascii(
		const struct ETreeVisitorImpl *visitor, void *root_node, FILE *out);

#ifdef __cplusplus
}
#endif
#endif /* E_TREE_H */
