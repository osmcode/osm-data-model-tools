#!/bin/bash
#-----------------------------------------------------------------------------
#
#  test/line-or-polygon.sh SOURCE_DIR
#
#-----------------------------------------------------------------------------

set -euo pipefail

SRCDIR="$1"

mkdir -p line-or-polygon

for input in "$SRCDIR"/test/line-or-polygon/*.opl; do
    output="line-or-polygon/"$(basename -s .opl "$input")
    ../src/odmt-line-or-polygon -e "$SRCDIR/filter-patterns" "$input" >"$output.result"
    expected="$SRCDIR/test/$output.expected"
    echo diff -u "$expected" "$output.result"
done

#-----------------------------------------------------------------------------
