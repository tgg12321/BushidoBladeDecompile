#!/bin/bash
# RTL dumps (-dl -dg -df) of a candidate body substituted into its TU, sandbox-stripped.
# usage: bash tmp/gte/dumps.sh <func> <candidate.c> <outdir>
set -e
source .venv/bin/activate
FUNC=$1; CAND=$2; OUT=$3
mkdir -p "$OUT"
python3 - "$FUNC" "$CAND" "$OUT" <<'PY'
import sys
sys.path.insert(0, ".")
from pathlib import Path
from engine import inlineasm, sandbox
func, cand, out = sys.argv[1:4]
stem = sandbox.func_file(func)
base = Path(f"src/{stem}.c").read_text(encoding="utf-8")
text = inlineasm.substitute_body(base, func, Path(cand).read_text(encoding="utf-8"))
inlineasm.write_stripped(stem, f"{out}/tu.c", text)
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$OUT/tu.c" > "$OUT/tu.i" 2>/dev/null
(cd "$OUT" && ../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg -df tu.i -o tu.s)
ls "$OUT"
