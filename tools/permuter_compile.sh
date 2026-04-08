#!/bin/bash
# Compile script for decomp-permuter (BB2)
# Usage: ./compile.sh input.c -o output.o
set -eo pipefail

INPUT="$1"
shift; shift
OUTPUT="$1"

# Find project root by walking up from script location
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$DIR"
while [ ! -f "$ROOT/Makefile" ] && [ "$ROOT" != "/" ]; do
    ROOT="$(dirname "$ROOT")"
done
if [ ! -f "$ROOT/Makefile" ]; then
    echo "ERROR: Cannot find project root (no Makefile found)" >&2
    exit 1
fi

CC1="$ROOT/tools/gcc-2.7.2/build/cc1"
PROLOGUE_FIX="$ROOT/tools/prologue_fix.py"
MASPSX="$ROOT/tools/maspsx/maspsx.py"

CC_FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

cd "$ROOT"

# Copy input to /tmp to avoid spaces in path (maspsx .file directive bug)
# Use PID-based name to avoid collisions with parallel runs
TMP_INPUT="/tmp/bb2_permuter_input_$$.c"
trap 'rm -f "$TMP_INPUT"' EXIT
cp "$INPUT" "$TMP_INPUT"

mipsel-linux-gnu-cpp \
  "-I$ROOT/include" -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
  "$TMP_INPUT" \
  | "$CC1" $CC_FLAGS \
  | python3 "$PROLOGUE_FIX" \
  | python3 "$MASPSX" --expand-div --aspsx-version=2.34 \
      "--sdata-syms=$ROOT/sdata_syms.txt" \
      "--sdata-funcs=$ROOT/sdata_funcs.txt" \
      "--sdata-exclude=$ROOT/sdata_exclude.txt" \
  | mipsel-linux-gnu-as "-I$ROOT/include" -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUTPUT"

# Validate output was produced
if [ ! -s "$OUTPUT" ]; then
    echo "ERROR: Compilation produced empty output file: $OUTPUT" >&2
    exit 1
fi
