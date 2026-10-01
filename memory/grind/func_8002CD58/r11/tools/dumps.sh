#!/bin/bash
# RTL dumps (-dl -dg -df) of a func_8002CD58 candidate substituted into its TU, sandbox-stripped.
# usage: bash memory/grind/func_8002CD58/r11/tools/dumps.sh <candidate.c> <outdir> [findreg-pseudo ...]
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
FUNC=func_8002CD58; CAND=$1; OUT=$2; shift 2
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
CC1="$PWD/tools/gcc-2.7.2/build/cc1"; CC1I="$PWD/tools/gcc-2.7.2/cc1"
FL="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
(cd "$OUT" && "$CC1" $FL -dl -dg -df tu.i -o tu.s)
for p in "$@"; do
  (cd "$OUT" && BB2_FINDREG_DEBUG=$p "$CC1I" $FL tu.i -o tu_instr.s 2> findreg_$p.txt || true)
done
echo "cmd: $CC1 $FL -dl -dg -df tu.i -o tu.s" > "$OUT/cmd.txt"
ls "$OUT"
