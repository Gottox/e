#include <rope.h>
#include <sys/wait.h>
#include <tree_sitter/api.h>

struct EDocument {
	struct Rope content;
	struct TSTree *tree;
	const TSLanguage *(*parser)(void);
};

int e_document_init(struct EDocument *doc);

int e_document_cleanup(struct EDocument *doc);
