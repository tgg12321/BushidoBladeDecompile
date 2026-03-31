#!/bin/bash
# Dump RTL debug output for a C file
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
INPUT="$1"
TMP="/tmp/bb2_rtl_debug"
mkdir -p "$TMP"
BASENAME="$(basename "$INPUT" .c)"
cp "$INPUT" "$TMP/$BASENAME.c"
cd "$TMP"
mipsel-linux-gnu-cpp \
  "-I$ROOT/include" -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
  "$TMP/$BASENAME.c" \
  > "$TMP/$BASENAME.i"
"$ROOT/tools/gcc-2.7.2/build/cc1" -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -da "$TMP/$BASENAME.i" 2>&1 | head -3
echo "=== RTL dump files ==="
ls "$TMP/$BASENAME.i."* 2>/dev/null || echo "No RTL dump files found in $TMP"
ls "$TMP/"*.greg "$TMP/"*.lreg "$TMP/"*.jump "$TMP/"*.rtl 2>/dev/null || true
echo "=== Searching for frame/stack info ==="
grep -i "frame\|stack.slot\|virtual-stack" "$TMP/$BASENAME.i."* 2>/dev/null | head -30 || echo "No frame matches in dump files"
