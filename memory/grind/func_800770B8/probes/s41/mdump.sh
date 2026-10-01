#!/bin/bash
# mdump.sh <name> <cand.c>: on the private main clone (/tmp/l770m/tree), splice the candidate into a copy of
# src/text1b_tu2.c and run the build's cc1 with -da (all RTL dumps); keep func_800770B8's section of each
# dump as /tmp/l770m/d/<name>/f.<pass> and its asm as f.asm. The exact command lines go to cmd.txt.
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
N="$1"; C="$2"; case "$C" in /*) ;; *) C="$R/$C";; esac
cd /tmp/l770m/tree || exit 1
D=/tmp/l770m/d/$N; rm -rf "$D"; mkdir -p "$D"
python3 - "$C" "$D/x.c" <<'PY'
import sys
src = open("src/text1b_tu2.c").read()
line = 'INCLUDE_ASM("asm/funcs", func_800770B8);'
assert src.count(line) == 1
open(sys.argv[2], "w").write(src.replace(line, open(sys.argv[1]).read()))
PY
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC1="tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
{ echo "# in a clone of main with include/game.h's SelWork f1C/f20 unions (tmp/func_800770B8/unions.py)";
  echo "$CPP x.c > x.i   # x.c = src/text1b_tu2.c with the INCLUDE_ASM line replaced by $(basename "$C")";
  echo "$CC1 -da -dumpbase x x.i -o x.s"; } > "$D/cmd.txt"
$CPP "$D/x.c" > "$D/x.i"
(cd "$D" && /tmp/l770m/tree/$CC1 -da -dumpbase x x.i -o x.s)
for f in "$D"/x.*; do
  ext="${f##*.}"; case "$ext" in c|i|s) continue;; esac
  awk '/^;; Function /{p=($3=="func_800770B8")} p' "$f" > "$D/f.$ext"
done
awk '/^func_800770B8:/{p=1} p{print} p&&/\.end[ \t]+func_800770B8/{exit}' "$D/x.s" > "$D/f.asm"
rm -f "$D"/x.*[a-z0-9] 2>/dev/null; ls "$D" | tr '\n' ' '; echo
