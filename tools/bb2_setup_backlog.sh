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
    # Tolerate CRLF in asm files (some have \r\n endings against project
    # convention). Use \b or end-of-line-or-cr regex.
    ASMFILE=$(grep -ln "^glabel ${FUNC}[[:space:]]*\$" asm/funcs/*.s 2>/dev/null | head -1)
    if [ -z "$ASMFILE" ]; then
        # Try the 6CAC.s monolith -- many functions live there
        if grep -q "^glabel ${FUNC}[[:space:]]*$" asm/6CAC.s 2>/dev/null; then
            echo "  EXTRACT ${FUNC}: from asm/6CAC.s monolith"
            # Extract glabel...endlabel block. 6CAC.s uses jlabel/glabel
            # internally; we just grab from glabel <fn> to the next
            # glabel/endlabel boundary.
            awk -v fn="^glabel ${FUNC}\$" -v end="^endlabel ${FUNC}\$" -v nextfn="^glabel " '
                $0 ~ fn { in_fn=1 }
                in_fn { print }
                in_fn && $0 ~ end { exit }
                in_fn && $0 ~ nextfn && !/^glabel '"${FUNC}"'$/ { exit }
            ' asm/6CAC.s > "asm/funcs/${FUNC}.s.tmp"
            # Trim the trailing next-glabel line if it got captured (when no endlabel)
            if [ "$(tail -1 "asm/funcs/${FUNC}.s.tmp")" != "endlabel ${FUNC}" ]; then
                # Add endlabel for cleanliness; permuter_setup.sh tolerates this
                head -n -1 "asm/funcs/${FUNC}.s.tmp" > "asm/funcs/${FUNC}.s"
            else
                mv "asm/funcs/${FUNC}.s.tmp" "asm/funcs/${FUNC}.s"
            fi
            rm -f "asm/funcs/${FUNC}.s.tmp"
        else
            echo "  SKIP ${FUNC}: no asm found in asm/funcs/ or asm/6CAC.s"
            return 1
        fi
    fi

    # If the asm file isn't named after this function, alias it.
    # Always normalize CRLF -> LF when writing (some asm files have CRLF
    # from earlier processing; the downstream sed in permuter_setup.sh
    # mangles `glabel X\r` into `X\r:` which breaks assembly).
    if [ -n "$ASMFILE" ] && [ "$(basename "$ASMFILE" .s)" != "$FUNC" ]; then
        local FUNCS_IN_FILE
        FUNCS_IN_FILE=$(grep -c "^glabel " "$ASMFILE")
        if [ "$FUNCS_IN_FILE" = "1" ]; then
            tr -d '\r' < "$ASMFILE" > "asm/funcs/${FUNC}.s"
        else
            awk -v fn="^glabel ${FUNC}[[:space:]]*$" -v end="^endlabel ${FUNC}[[:space:]]*$" '
                $0 ~ fn { in_fn=1 }
                in_fn { print }
                $0 ~ end { exit }
            ' "$ASMFILE" | tr -d '\r' > "asm/funcs/${FUNC}.s"
        fi
    elif [ -n "$ASMFILE" ]; then
        # Same-name asm file. Normalize CRLF in place if needed.
        if grep -q $'\r' "$ASMFILE"; then
            tr -d '\r' < "$ASMFILE" > "${ASMFILE}.tmp" && mv "${ASMFILE}.tmp" "$ASMFILE"
        fi
    fi

    # 1. Run dc.sh setup (creates permuter/<func>/ with target.s, target.o, stub base.c)
    bash tools/dc.sh setup "$FUNC" "$SRC" > /dev/null 2>&1 || {
        echo "  SKIP ${FUNC}: dc.sh setup failed"
        return 1
    }

    # 2. Extract real C body into base.c
    local EXTRACT_ERR
    EXTRACT_ERR=$(python3 tools/bb2_extract_func_body.py "$FUNC" "$SRC" --out "permuter/${FUNC}/base.c" 2>&1)
    if [ $? -ne 0 ]; then
        echo "  SKIP ${FUNC}: extract failed: ${EXTRACT_ERR}"
        return 1
    fi

    # 3. Test compile (sanity)
    local COMPILE_ERR
    COMPILE_ERR=$("permuter/${FUNC}/compile.sh" "permuter/${FUNC}/base.c" -o /tmp/bb2_setup_test.o 2>&1)
    if [ $? -ne 0 ]; then
        # Surface the first error message so agents know what's missing.
        # Filter out the maspsx 'no input from stdin' noise (cascade from
        # the actual compile error upstream).
        local FIRST_ERR
        FIRST_ERR=$(echo "$COMPILE_ERR" | grep -E "error|undeclared|undefined|parse error" | head -1)
        echo "  SKIP ${FUNC}: compile failed: ${FIRST_ERR:-(see compile.sh output)}"
        echo "         hint: missing extern/typedef in permuter/${FUNC}/base.c; add manually"
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
