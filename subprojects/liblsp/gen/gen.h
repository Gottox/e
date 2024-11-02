#include <jw.h>

#define LSP_PREFIX_SNAKE "lsp_"
#define LSP_PREFIX_PASCAL "Lsp"

///////////////////////////////
// identifier_util.c

void pascal_case(char *str);

__attribute__((warn_unused_result)) char *snake_case(const char *str);

///////////////////////////////
// gen_requests.c

int gen_requests(struct Jw *jw, struct JwVal *requests);

///////////////////////////////
// gen_structures.c

int gen_structures(
		struct Jw *jw, struct JwVal *structures, struct JwVal *type_aliases,
		struct JwVal *enums);
