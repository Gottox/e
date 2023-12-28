#!/bin/sh

set -e

: "${GIT:=git}"

REPO="${1?Missing repository URL}"
REV="${2?Missing revision}"
OUT="${3?Missing output directory}"

[ -d "$OUT/.git" ] || $GIT init "$OUT"
if git -C "$OUT" remote | grep -qFx origin; then
	$GIT -C "$OUT" remote set-url origin "$REPO"
else
	$GIT -C "$OUT" remote add origin "$REPO"
fi
$GIT -C "$OUT" fetch --depth 1 origin "$REV"
$GIT -C "$OUT" -c advice.detachedHead=false checkout FETCH_HEAD
