#!/bin/sh

OUTDIR="${1?Missing output directory}"
shift

### Generate header

# Generate enums
exec > "${OUTDIR}/e-parser-table.h"
cat <<EOF
#ifndef E_PARSER_TABLE_H
#define E_PARSER_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum ETreeSitterLanguageId {
EOF
for grammar_name; do
	printf '	E_TREE_SITTER_%s,\n' "$( echo "${grammar_name}" | tr '[:lower:]' '[:upper:]')"
done | sort
cat <<EOF
};
#ifdef __cplusplus
}
#endif
#endif /* E_PARSER_TABLE_H */
EOF

### Generate c file
exec > "${OUTDIR}/e-parser-table.c"
cat <<EOF
#include <e-parser.h>
EOF

# Generate extern declarations
for grammar_name; do
	echo "extern const TSLanguage *tree_sitter_${grammar_name}(void);"
done | sort

# Generate table
cat <<EOF
const struct ETreeSitterLanguage e_tree_sitter_languages[] = {
EOF
for grammar_name; do
	printf '	{ "%s", %s },\n' "${grammar_name}" "tree_sitter_${grammar_name}"
done | sort

cat <<EOF
};
const size_t e_tree_sitter_languages_len = $#;
EOF
