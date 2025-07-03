#include <e-parser.h>

#define DEF(name, x) extern const TSLanguage *tree_sitter_##name(void);
#include <e-parser-table.h>
#undef DEF

#define DEF(name, x) {#name, tree_sitter_##name()}
#include <e-parser-table.h>
#undef DEF
