#!/bin/bash
# usage: dump.sh <tag> <override.c> [ENV=1 ...]
#   cpp the override (quoted includes resolve next to it), run cc1 -da with optional BB2_* env,
#   keep stderr (debug hooks) in tmp/rtl/<tag>/cc1.err and the function's final .s in fn.s
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; SRC=$2; shift 2
OUT=tmp/rtl/$TAG
rm -rf "$OUT"; mkdir -p "$OUT"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$SRC" > "$OUT/in.i" 2>/dev/null
CC1="$PWD/tools/gcc-2.7.2/cc1"
( cd "$OUT" && env "$@" "$CC1" -O2 ${GFLAG:--G0} -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da in.i -o in.s 2> cc1.err || true )
python3 - "$OUT" "${FN:-func_80049718}" <<'PY'
import sys, re
out, fn = sys.argv[1], sys.argv[2]
s = open(out + "/in.s").read().split("\n")
res = []; on = False
for l in s:
    if re.match(r"^%s:" % fn, l): on = True
    if on: res.append(l)
    if on and re.match(r"\s*\.end\s+%s" % fn, l): break
open(out + "/fn.s", "w").write("\n".join(res) + "\n")
print(out, len(res))
PY
