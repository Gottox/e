#!/bin/sh
set -e

GRAMMAR_NAME="${1?Missing grammar name}"
GRAMMAR_DIR="$(realpath "${2?Missing grammar directory}")"
OUTDIR="${3?Missing output directory}"

: "${TREE_SITTER:=tree-sitter}"

(
	cd "$GRAMMAR_DIR"
	"$TREE_SITTER" generate
)

exec > "$OUTDIR/${GRAMMAR_NAME}-parser.c"
printf '#include "%s/src/parser.c"\n' "${GRAMMAR_DIR}"

exec > "$OUTDIR/${GRAMMAR_NAME}-scanner.c"
if [ -f "${GRAMMAR_DIR}/src/scanner.c" ]; then
	printf '#include "%s/src/scanner.c"\n' "${GRAMMAR_DIR}"
else
	echo 'void E_OPTIMIZE_ME_OUT(void) {}'
fi

