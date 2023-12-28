#!/bin/sh
set -e

NPM_DIR="${1}"
MARKER="${2}"

: "${NPM:=npm}"

cd "${NPM_DIR}"
${NPM} install
cd -

touch "${MARKER}"
