#!/bin/bash
# dc.sh — Decomp helper for Claude Code sessions
# Eliminates fragile WSL one-liners. All commands run from project root.
#
# Usage (from WSL):
#   bash tools/dc.sh compile <func_dir>          — compile + objdump
#   bash tools/dc.sh score <func_dir>             — permuter score only
#   bash tools/dc.sh debug <func_dir>             — permuter --debug (full diff)
#   bash tools/dc.sh build                        — incremental make, tail output
#   bash tools/dc.sh replace <src_file> <func_name> <c_file>  — replace INCLUDE_ASM with C code (LF-safe)
#   bash tools/dc.sh setup <func_name> <src_file> — permuter setup
#   bash tools/dc.sh analysis <func_name>         — run asm_analysis.py
#   bash tools/dc.sh dump-text <func_name> [src]  — dump numbered TEXT indices from pipeline
#   bash tools/dc.sh validate-regfix [--func F]   — validate regfix.txt rules (static)
#   bash tools/dc.sh validate-regfix --live [--func F] — validate rules against pipeline
#   bash tools/dc.sh gen-regfix <func_name> [src] — auto-generate regfix rules from diff
#   bash tools/dc.sh verify <func_name>           — binary-level verify function against original
#   bash tools/dc.sh verify --all                  — verify all C functions
#
set -eo pipefail

# Find project root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/.."
cd "$ROOT"

# Activate venv if present
if [ -f ".venv/bin/activate" ]; then
    source .venv/bin/activate
fi

CMD="$1"
shift || { echo "Usage: bash tools/dc.sh <command> [args...]"; exit 1; }

case "$CMD" in
    compile)
        # Compile a permuter base.c and dump disassembly
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh compile <func_dir>"; exit 1; }
        OUT="/tmp/dc_test_$$.o"
        trap 'rm -f "$OUT"' EXIT
        bash "$FUNC_DIR/compile.sh" "$FUNC_DIR/base.c" -o "$OUT" 2>&1
        mipsel-linux-gnu-objdump -d "$OUT"
        ;;

    score)
        # Get permuter score (number only)
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh score <func_dir>"; exit 1; }
        python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1 | grep "score"
        ;;

    debug)
        # Full permuter debug output (diff + score)
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh debug <func_dir>"; exit 1; }
        python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1
        ;;

    build)
        # Incremental build + SHA1 check
        make 2>&1 | tail -5
        ;;

    replace)
        # Replace INCLUDE_ASM stub with C code from a file (LF-safe)
        SRC_FILE="$1"
        FUNC_NAME="$2"
        C_FILE="$3"
        [ -z "$C_FILE" ] && { echo "Usage: dc.sh replace <src_file> <func_name> <c_file>"; exit 1; }
        [ ! -f "$SRC_FILE" ] && { echo "ERROR: $SRC_FILE not found"; exit 1; }
        [ ! -f "$C_FILE" ] && { echo "ERROR: $C_FILE not found"; exit 1; }
        # Loudly identify which repo we are operating on. Past agents have hit
        # the main repo instead of their worktree because dc.sh cd's to its own
        # SCRIPT_DIR/.. — print the absolute resolved path so the caller can
        # spot the wrong target before damage is done.
        ABS_SRC="$(cd "$(dirname "$SRC_FILE")" && pwd)/$(basename "$SRC_FILE")"
        BRANCH="$(git symbolic-ref --short HEAD 2>/dev/null || echo '<detached>')"
        echo "REPLACE: $ABS_SRC (branch: $BRANCH)"
        if [ "$BRANCH" = "main" ]; then
            echo "WARNING: operating on main branch — if you intended a worktree, abort now."
        fi
        # Read replacement C code (strip any CR)
        REPLACEMENT="$(tr -d '\r' < "$C_FILE")"
        # Do the replacement using python (safe with any content)
        python3 -c "
import sys
src = '$SRC_FILE'
func = '$FUNC_NAME'
with open(src, 'r') as f:
    content = f.read()
stub = 'INCLUDE_ASM(\"asm/funcs\", ' + func + ');'
if stub not in content:
    print(f'ERROR: stub {stub} not found in {src}', file=sys.stderr)
    sys.exit(1)
with open('$C_FILE', 'r') as f:
    repl = f.read().rstrip()
content = content.replace(stub, repl)
with open(src, 'w', newline='\n') as f:
    f.write(content)
print(f'Replaced {func} in {src}')
"
        ;;

    setup)
        # Set up permuter directory for a function
        FUNC_NAME="$1"
        SRC_FILE="$2"
        [ -z "$SRC_FILE" ] && { echo "Usage: dc.sh setup <func_name> <src_file>"; exit 1; }
        bash tools/permuter_setup.sh "$FUNC_NAME" "$SRC_FILE"
        ;;

    analysis)
        # Run asm_analysis on a function
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh analysis <func_name>"; exit 1; }
        python3 tools/asm_analysis.py "asm/funcs/${FUNC_NAME}.s" 2>&1
        ;;

    dump-text)
        # Dump numbered TEXT indices from build pipeline
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh dump-text <func_name> [src_file]"; exit 1; }
        SRC_FILE="${2:-}"
        if [ -n "$SRC_FILE" ]; then
            python3 tools/dump_text_indices.py "$FUNC_NAME" "$SRC_FILE"
        else
            python3 tools/dump_text_indices.py "$FUNC_NAME"
        fi
        ;;

    validate-regfix)
        # Validate regfix.txt rules
        python3 tools/validate_regfix.py "$@" 2>&1
        ;;

    gen-regfix)
        # Auto-generate regfix rules from pipeline vs target diff
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh gen-regfix <func_name> [src_file]"; exit 1; }
        SRC_FILE="${2:-}"
        if [ -n "$SRC_FILE" ]; then
            python3 tools/gen_regfix.py "$FUNC_NAME" "$SRC_FILE"
        else
            python3 tools/gen_regfix.py "$FUNC_NAME"
        fi
        ;;

    verify)
        # Binary-level verification of function against original
        ARG1="${1:---help}"
        if [ "$ARG1" = "--help" ]; then
            echo "Usage: dc.sh verify <func_name>   — verify one function"
            echo "       dc.sh verify --all          — verify all C functions"
            exit 1
        fi
        python3 tools/regfix_verify.py "$ARG1" 2>&1
        ;;

    *)
        echo "Unknown command: $CMD"
        echo "Commands: compile, score, debug, build, replace, setup, analysis, dump-text, validate-regfix, gen-regfix, verify"
        exit 1
        ;;
esac
