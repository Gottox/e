#include <e-parser.h>

#define DEF(name, upper_name) extern const TSLanguage *tree_sitter_##name(void);
#include <e-parser-table.def.h>
#undef DEF

const struct ETreeSitterLanguage e_tree_sitter_languages[] = {
#define DEF(name, upper_name) {#name, tree_sitter_##name},
#include <e-parser-table.def.h>
#undef DEF
};

const size_t e_tree_sitter_languages_len =
		sizeof(e_tree_sitter_languages) / sizeof(e_tree_sitter_languages[0]);
