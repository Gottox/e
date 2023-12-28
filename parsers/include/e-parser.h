#include <e-parser-table.h>
#include <tree_sitter/parser.h>

#ifndef E_PARSER_H
#	define E_PARSER_H

struct ETreeSitterLanguage {
	const char *name;
	const TSLanguage *(*language)(void);
};

extern const struct ETreeSitterLanguage e_tree_sitter_parser[];
extern const struct ETreeSitterLanguage e_tree_sitter_parser_len;

#	ifdef __cplusplus
extern "C" {
#	endif

#	ifdef __cplusplus
}
#	endif
#endif /* E_PARSER_TABLE_H */
