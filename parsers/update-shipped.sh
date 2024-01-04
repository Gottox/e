#!/bin/sh

set -e

BUILD_DIR=${1?Missing build directory}
BUILD_DIR=$(realpath "$BUILD_DIR")
SUBDIR=shipped

cd $(dirname "$0")

cp_files() {
	local src="$1"
	local dest="$2"

	local files=$(gcc -MM "$src" \
		| xargs echo \
		| xargs ls 2> /dev/null \
		| grep -vFx "$src")
	for file in $files; do 
		new_path="$dest/${file#$BUILD_DIR/parsers/generate/*/}"
		new_dir=${new_path%/*}
		printf "        '%s',\n" "${new_path#$SUBDIR/}"
		mkdir -p "$new_dir"
		cp "$file" "$new_path"
	done
}
mkdir -p $SUBDIR
rm -rf $SUBDIR/*
exec >> "$SUBDIR/meson.build"
echo "tree_sitter_parsers_source = ["
for parser in $BUILD_DIR/parsers/generate/*-parser.c; do
	parser_name="${parser%-parser.c}"
	scanner="${parser_name}-scanner.c"
	parser_name="${parser_name##*/}"
	echo "$parser_name" >&2
	echo "    # $parser_name"
	mkdir -p "$SUBDIR/$parser_name"
	echo "    files("
	cp_files "$parser" "$SUBDIR/$parser_name"
	cp_files "$scanner" "$SUBDIR/$parser_name"
	echo "    ),"
done
echo "]"
