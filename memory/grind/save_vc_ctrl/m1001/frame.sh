#!/bin/bash
# cc1 -dl -dg on the (spliced) src TU; prints .frame lines and pseudos with refs but no hard register.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
STEM=$1; FUNC=$2; OUT=tmp/svc/fr_$FUNC
mkdir -p $OUT
python3 - "$STEM" "$OUT" <<'PY'
import sys
sys.path.insert(0, ".")
from pathlib import Path
from engine import inlineasm
stem, out = sys.argv[1:3]
inlineasm.write_stripped(stem, f"{out}/tu.c", Path(f"src/{stem}.c").read_text(encoding="utf-8"))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $OUT/tu.c > $OUT/tu.i 2>/dev/null
(cd $OUT && "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/gcc-2.7.2/build/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg tu.i -o tu.s)
python3 memory/grind/func_80074E08/r9/tools/cut.py $OUT $FUNC 2>/dev/null || python3 - $OUT $FUNC <<'PY'
import sys
from pathlib import Path
d, func = Path(sys.argv[1]), sys.argv[2]
for p in ("lreg", "greg"):
    t = (d / f"tu.i.{p}").read_text(errors="replace")
    i = t.index(f";; Function {func}\n"); j = t.find("\n;; Function ", i + 10)
    (d / f"{func}.{p}").write_text(t[i:j if j > 0 else None])
PY
grep -A2 "\.ent	$FUNC" $OUT/tu.s | grep frame
python3 - $OUT $FUNC <<'PY'
import re, sys
d, f = sys.argv[1:3]
l = open(f"{d}/{f}.lreg").read(); g = open(f"{d}/{f}.greg").read()
used = re.findall(r"^Register (\d+) used (.*)$", l, re.M)
local = set(re.findall(r";; Register (\d+) in", l))
disp = set(re.findall(r"(\d+) in \d+", g[g.index("Register dispositions"):])) if "Register dispositions" in g else set()
for r, rest in used:
    if r not in local and r not in disp:
        print("unallocated pseudo", r, rest)
PY
