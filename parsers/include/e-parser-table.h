#ifndef E_PARSER_TABLE_H
#define E_PARSER_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum ETreeSitterLanguageId {
#define DEF(name, upper_name) E_TREE_SITTER_##upper_name,
#include <e-parser-table.def.h>
#undef DEF
};
#ifdef __cplusplus
}
#endif
#endif /* E_PARSER_TABLE_H */
