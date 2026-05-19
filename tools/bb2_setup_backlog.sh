#!/bin/bash
# bb2_setup_backlog.sh -- prepare permuter dirs for backlog functions.
# Usage: ./bb2_setup_backlog.sh <func> <src_file> [<func> <src_file>...]
#
# For each pair:
#  1. Find the function's asm in asm/funcs/ (resolves alias if function is
#     inside a different-named file)
#  2. Copy/alias to asm/funcs/<func>.s
#  3. Run dc.sh setup <func> <src_file> (creates permuter/<func>/)
#  4. Extract C body from <src_file> into base.c (replacing stub)
#  5. Prebake with target arg-pin chain

set -eu
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/.." && pwd)"
cd "$ROOT"

prepare_one() {
    local FUNC="$1"
    local SRC="$2"

    # Find asm file containing this function
    local ASMFILE
    ASMFILE=$(grep -ln "^glabel ${FUNC}$" asm/funcs/*.s 2>/dev/null | head -1)
    if [ -z "$ASMFILE" ]; then
        # Maybe inside 6CAC.s monolith -- this is a harder case, skip
        echo "  SKIP ${FUNC}: no per-func asm found (would need extraction from 6CAC.s)"
        return 1
    fi

    # If the asm file isn't named after this function, alias it
    if [ "$(basename "$ASMFILE" .s)" != "$FUNC" ]; then
        # Check if file contains ONLY this function -- if so, just copy.
        # If it has multiple, we need to extract the body.
        local FUNCS_IN_FILE
        FUNCS_IN_FILE=$(grep -c "^glabel " "$ASMFILE")
        if [ "$FUNCS_IN_FILE" = "1" ]; then
            cp "$ASMFILE" "asm/funcs/${FUNC}.s"
        else
            # Extract this function from the multi-function file
            awk -v fn="^glabel ${FUNC}$" -v end="^endlabel ${FUNC}$" '
                $0 ~ fn { in_fn=1 }
                in_fn { print }
                $0 ~ end { exit }
            ' "$ASMFILE" > "asm/funcs/${FUNC}.s"
        fi
    fi

    # 1. Run dc.sh setup (creates permuter/<func>/ with target.s, target.o, stub base.c)
    bash tools/dc.sh setup "$FUNC" "$SRC" > /dev/null 2>&1 || {
        echo "  SKIP ${FUNC}: dc.sh setup failed"
        return 1
    }

    # 2. Extract real C body into base.c
    python3 tools/bb2_extract_func_body.py "$FUNC" "$SRC" --out "permuter/${FUNC}/base.c" > /dev/null 2>&1 || {
        echo "  SKIP ${FUNC}: extract failed"
        return 1
    }

    # 3. Test compile (sanity)
    if ! "permuter/${FUNC}/compile.sh" "permuter/${FUNC}/base.c" -o /tmp/bb2_setup_test.o > /dev/null 2>&1; then
        echo "  SKIP ${FUNC}: base.c doesn't compile -- needs manual externs/typedefs"
        return 1
    fi

    # 4. Run prebake (best-effort)
    python3 tools/bb2_permuter_regression.py prebake "$FUNC" \
        --base-path "permuter/${FUNC}/base.c" \
        --target-path "permuter/${FUNC}/target.s" 2>&1 | head -3

    echo "  OK    ${FUNC}: ready"
    return 0
}

# Pair off args
while [ $# -gt 1 ]; do
    FUNC="$1"
    SRC="$2"
    shift 2
    echo "=== ${FUNC} (${SRC}) ==="
    prepare_one "$FUNC" "$SRC" || true
done
