#!/bin/bash
# try_match.sh -- Compile + score + diff a candidate C file against a function target
# Usage: bash tools/try_match.sh <func_name> [source.c]
#   If source.c is omitted, uses permuter/<func_name>/base.c
# Exit 0 = perfect match (score 0), 1 = non-match, 2 = compilation failure
set -e

if [ $# -lt 1 ]; then
    echo "Usage: bash tools/try_match.sh <func_name> [source.c]" >&2
    echo "  If source.c is omitted, uses permuter/<func_name>/base.c" >&2
    exit 2
fi

FUNC_NAME="$1"
SOURCE_C="${2:-}"

# Find project root
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$DIR"
while [ ! -f "$ROOT/Makefile" ] && [ "$ROOT" != "/" ]; do
    ROOT="$(dirname "$ROOT")"
done
if [ ! -f "$ROOT/Makefile" ]; then
    echo "ERROR: Cannot find project root" >&2
    exit 2
fi

PERMUTER_DIR="$ROOT/permuter/$FUNC_NAME"

# Default source to base.c
if [ -z "$SOURCE_C" ]; then
    SOURCE_C="$PERMUTER_DIR/base.c"
fi

# Validate paths
if [ ! -f "$PERMUTER_DIR/target.o" ]; then
    echo "ERROR: $PERMUTER_DIR/target.o not found" >&2
    echo "Run: bash tools/permuter_setup.sh $FUNC_NAME <src_file.c>" >&2
    exit 2
fi
if [ ! -f "$PERMUTER_DIR/compile.sh" ]; then
    echo "ERROR: $PERMUTER_DIR/compile.sh not found" >&2
    exit 2
fi
if [ ! -f "$SOURCE_C" ]; then
    echo "ERROR: Source file $SOURCE_C not found" >&2
    exit 2
fi

# Compile
CAND_O=$(mktemp /tmp/bb2_trymatch_XXXXXX.o)
trap "rm -f $CAND_O" EXIT

echo "Compiling $SOURCE_C..."
if ! bash "$PERMUTER_DIR/compile.sh" "$SOURCE_C" -o "$CAND_O" 2>/tmp/bb2_trymatch_err.txt; then
    echo "Compilation FAILED:" >&2
    cat /tmp/bb2_trymatch_err.txt >&2
    rm -f /tmp/bb2_trymatch_err.txt
    exit 2
fi
rm -f /tmp/bb2_trymatch_err.txt

# Score using Python helper
echo ""
python3 "$ROOT/tools/score_match.py" "$PERMUTER_DIR/target.o" "$CAND_O" --debug
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "*** PERFECT MATCH! ***"
fi

exit $EXIT_CODE
