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

# Validate target.o has non-empty .text section (catches silently dropped GTE instructions)
TEXT_SIZE=$(mipsel-linux-gnu-objdump -h "$PERMUTER_DIR/target.o" 2>/dev/null | grep '\.text' | awk '{print $3}')
if [ "$TEXT_SIZE" = "00000000" ] || [ -z "$TEXT_SIZE" ]; then
    echo "ERROR: $PERMUTER_DIR/target.o has empty .text section!" >&2
    echo "  The target was assembled incorrectly (GTE/COP2 instructions likely dropped)." >&2
    echo "  Fix: python3 tools/fix_gte_asm.py --fix-targets --reassemble" >&2
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

# Auto-fix CRLF in source before compiling (no separate step needed)
sed -i "s/\r$//" "$SOURCE_C"

# Warn if source uses M2C_ERROR (drops GTE/cop instructions, score is unreliable)
if grep -q 'M2C_ERROR' "$SOURCE_C"; then
    echo "WARNING: $SOURCE_C contains M2C_ERROR() — GTE instructions are dropped."
    echo "         Score will be UNRELIABLE (false positive likely)."
    echo ""
fi

# Session log setup
LOG="$PERMUTER_DIR/session_log.txt"
TS=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
ATTEMPT=$(grep -c "^[0-9].*|SCORE|" "$LOG" 2>/dev/null) || ATTEMPT=0
ATTEMPT=$((ATTEMPT + 1))

# Compile
CAND_O=$(mktemp /tmp/bb2_trymatch_XXXXXX.o)
trap "rm -f $CAND_O" EXIT

echo "Compiling $SOURCE_C..."
if ! bash "$PERMUTER_DIR/compile.sh" "$SOURCE_C" -o "$CAND_O" 2>/tmp/bb2_trymatch_err.txt; then
    echo "Compilation FAILED:" >&2
    cat /tmp/bb2_trymatch_err.txt >&2
    echo "${TS}|COMPILE_FAIL|attempt=${ATTEMPT}" >> "$LOG"
    rm -f /tmp/bb2_trymatch_err.txt
    exit 2
fi
rm -f /tmp/bb2_trymatch_err.txt

# Score using Python helper (disable set -e here — non-zero means non-match, not error)
echo ""
set +e
python3 "$ROOT/tools/score_match.py" "$PERMUTER_DIR/target.o" "$CAND_O" --debug > /tmp/bb2_score_out.txt 2>&1
EXIT_CODE=$?
set -e
cat /tmp/bb2_score_out.txt

# Extract numeric score and log
SCORE_VAL=$(grep -oP 'Score:\s*\K[0-9]+' /tmp/bb2_score_out.txt | tail -1) || true
echo "${TS}|SCORE|attempt=${ATTEMPT} score=${SCORE_VAL:-unknown}" >> "$LOG"
rm -f /tmp/bb2_score_out.txt

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "*** PERFECT MATCH! ***"
    echo "${TS}|MATCHED|attempt=${ATTEMPT}" >> "$LOG"
fi

exit $EXIT_CODE
