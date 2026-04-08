#!/bin/bash
# Set up a decomp-permuter run directory for a BB2 function.
# Usage: ./permuter_setup.sh <func_name> [source_file.c]
# Example: ./permuter_setup.sh func_80048A7C src/text1b.c
set -e

FUNC_NAME="$1"
SOURCE_FILE="${2:-}"

if [ -z "$FUNC_NAME" ]; then
    echo "Usage: $0 <func_name> [source_file.c]"
    echo "Example: $0 func_80048A7C src/text1b.c"
    exit 1
fi

DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$DIR"
while [ ! -f "$ROOT/Makefile" ] && [ "$ROOT" != "/" ]; do
    ROOT="$(dirname "$ROOT")"
done
if [ ! -f "$ROOT/Makefile" ]; then
    echo "ERROR: Cannot find project root (no Makefile found)" >&2
    exit 1
fi
PERMUTER_DIR="$ROOT/tools/decomp-permuter"
WORK_DIR="$ROOT/permuter/$FUNC_NAME"

# Check that the function asm exists
FUNC_ASM="$ROOT/asm/funcs/$FUNC_NAME.s"
if [ ! -f "$FUNC_ASM" ]; then
    echo "ERROR: Cannot find $FUNC_ASM"
    exit 1
fi

echo "Setting up permuter for $FUNC_NAME..."

# Create work directory
mkdir -p "$WORK_DIR"

# --- Create target.s from the function's assembly ---
# The permuter needs MIPS asm with a specific format.
# We convert from splat format (glabel/endlabel) to plain labels.
echo "  Creating target.s..."
{
    echo ".set noat"
    echo ".set noreorder"
    echo ""
    echo ".text"
    echo ""
    # Convert glabel to regular label, strip endlabel, strip comments with addresses
    sed -e 's/^glabel \(.*\)/\1:/' \
        -e '/^endlabel/d' \
        -e 's|/\*.*\*/  ||' \
        "$FUNC_ASM"
} > "$WORK_DIR/target.s"

# Fix GTE/COP2 instructions and splat directives (jlabel, alabel) before assembling
echo "  Fixing GTE/splat instructions in target.s..."
python3 "$ROOT/tools/fix_gte_asm.py" "$WORK_DIR/target.s" 2>&1 | grep -v "^$" || true

# Assemble target.s to target.o
echo "  Assembling target.o..."
ASM_ERR=$(mipsel-linux-gnu-as -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 \
    "$WORK_DIR/target.s" -o "$WORK_DIR/target.o" 2>&1) || {
    echo "WARNING: Could not assemble target.s:" >&2
    echo "$ASM_ERR" >&2
    echo "  You may need to fix target.s manually."
}

# Validate target.o has non-empty .text section
if [ -f "$WORK_DIR/target.o" ]; then
    TEXT_SIZE=$(mipsel-linux-gnu-objdump -h "$WORK_DIR/target.o" 2>/dev/null \
        | grep '\.text' | awk '{print $3}')
    if [ "$TEXT_SIZE" = "00000000" ] || [ -z "$TEXT_SIZE" ]; then
        echo "ERROR: target.o has empty .text section!" >&2
        echo "  Assembly silently dropped instructions. Check target.s for" >&2
        echo "  unrecognized opcodes (GTE, jlabel, alabel)." >&2
    fi
fi

# --- Create base.c ---
echo "  Creating base.c..."
if [ -n "$SOURCE_FILE" ] && [ -f "$ROOT/$SOURCE_FILE" ]; then
    # Extract context from the source file - preprocess with PERMUTER defined
    # to strip INCLUDE_ASM macros, then keep only the target function
    echo "  Preprocessing $SOURCE_FILE..."
    cd "$ROOT"
    mipsel-linux-gnu-cpp \
        -I"$ROOT/include" -undef -Wall -lang-c -fno-builtin \
        -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
        -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
        -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
        "$ROOT/$SOURCE_FILE" 2>/dev/null | \
        python3 "$PERMUTER_DIR/strip_other_fns.py" - "$FUNC_NAME" > "$WORK_DIR/base.c" 2>/dev/null || true

    # Check if we got a valid base.c
    if [ ! -s "$WORK_DIR/base.c" ] || ! grep -q "$FUNC_NAME" "$WORK_DIR/base.c" 2>/dev/null; then
        echo "  WARNING: Could not extract $FUNC_NAME from $SOURCE_FILE."
        # Try m2c as fallback
        _try_m2c=1
    fi
else
    # No source file specified — try m2c directly
    _try_m2c=1
fi

# --- m2c fallback: generate base.c from assembly ---
if [ "${_try_m2c:-0}" = "1" ]; then
    M2C="$ROOT/tools/m2c/m2c.py"
    STRUCT_FIX="$ROOT/tools/struct_copy_fix.py"
    if [ -f "$M2C" ]; then
        echo "  Running m2c on $FUNC_NAME..."
        M2C_OUT=$(python3 "$M2C" --valid-syntax -t mipsel-gcc-c "$FUNC_ASM" 2>/dev/null) || true
        if [ -n "$M2C_OUT" ] && echo "$M2C_OUT" | grep -q "$FUNC_NAME"; then
            # Apply struct copy fix if the tool exists
            if [ -f "$STRUCT_FIX" ]; then
                FIXED=$(echo "$M2C_OUT" | python3 "$STRUCT_FIX" /dev/stdin --asm "$FUNC_ASM" 2>/dev/null) || true
                if [ -n "$FIXED" ]; then
                    # Report what was fixed
                    FIXES=$(echo "$M2C_OUT" | python3 "$STRUCT_FIX" /dev/stdin --asm "$FUNC_ASM" --dry-run 2>/dev/null) || true
                    if [ -n "$FIXES" ] && ! echo "$FIXES" | grep -q "No struct copy"; then
                        echo "  struct_copy_fix: $FIXES"
                    fi
                    M2C_OUT="$FIXED"
                fi
            fi
            echo "$M2C_OUT" > "$WORK_DIR/base.c"
            echo "  Created base.c from m2c output."
        else
            echo "  m2c failed; creating stub base.c instead."
            _create_stub=1
        fi
    else
        echo "  m2c not found; creating stub base.c instead."
        _create_stub=1
    fi

    if [ "${_create_stub:-0}" = "1" ]; then
        cat > "$WORK_DIR/base.c" << BASEC
// TODO: Add proper types, externs, and function body
// Function: $FUNC_NAME

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

void $FUNC_NAME(void) {
    // TODO: decompile this function
}
BASEC
    fi
fi

# --- Copy compile.sh ---
cp "$ROOT/tools/permuter_compile.sh" "$WORK_DIR/compile.sh"

# --- Create settings.toml ---
cat > "$WORK_DIR/settings.toml" << TOML
compiler = "gcc"
TOML

echo ""
echo "=== Permuter setup complete ==="
echo "Directory: $WORK_DIR"
echo ""
echo "Files created:"
ls -la "$WORK_DIR/"
echo ""
echo "Next steps:"
echo "  1. Edit $WORK_DIR/base.c with your best decompilation attempt"
echo "  2. Run: cd $ROOT && python3 tools/decomp-permuter/permuter.py permuter/$FUNC_NAME -j"
echo "  3. Or for manual permutations, add PERM_* macros to base.c"
echo ""
echo "Quick test:"
echo "  cd $ROOT && $WORK_DIR/compile.sh $WORK_DIR/base.c -o /tmp/test.o && echo 'Compile OK'"
