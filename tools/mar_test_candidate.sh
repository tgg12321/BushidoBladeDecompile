#!/bin/bash
# Apply the candidate, run cc1 dispositions + engine sandbox, restore.
set -e
cd "$(dirname "$0")/.."
cp src/system.c tmp/system.c.bak
python3 tmp/mar_apply_candidate.py
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips \
  -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
  src/system.c > tmp/rtl/marD.i 2>/dev/null
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
  -mips1 -mno-abicalls -fno-builtin -w -da tmp/rtl/marD.i -o /dev/null \
  2>/dev/null || true
python3 - <<'PYEOF'
import re
from pathlib import Path
g = Path("tmp/rtl/marD.i.greg").read_text()
i = g.index(";; Function marionation_Exec")
j = g.find(";; Function", i + 10)
gs = g[i:j if j > 0 else len(g)]
m = re.search(r";; Register dispositions:\s*\n((?:[^\n;]*\n)+)", gs)
disp = dict(re.findall(r"(\d+) in (\d+)", m.group(1)))
l = Path("tmp/rtl/marD.i.lreg").read_text()
li_ = l.index(";; Function marionation_Exec")
ls = l[li_:li_ + 60000]
RN = {16: "s0", 17: "s1", 18: "s2", 19: "s3", 20: "s4", 21: "s5",
      22: "s6", 23: "s7", 30: "fp"}
for ps, r in sorted(disp.items(), key=lambda kv: int(kv[1])):
    r = int(r)
    if r < 16 or r > 30:
        continue
    stat = re.search(rf"Register {ps} ([^\n]*)", ls)
    print(f"pseudo {ps} -> {RN.get(r, r)}  | {stat.group(1)[:80] if stat else '?'}")
PYEOF
source .venv/bin/activate
python3 -m engine.cli sandbox marionation_Exec --disable all 2>&1 \
  | grep -E '"score"|_insns'
cp tmp/system.c.bak src/system.c
echo "src restored"
