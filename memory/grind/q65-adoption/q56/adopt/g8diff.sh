#!/bin/bash
# g8diff.sh <stem>: test clone - compile src/<stem>.c with cc1 -G0 and -G8 (same pipeline otherwise); list the
# functions whose disassembly differs.
cd /tmp/q56r2/t || exit 1
source .venv/bin/activate
S=$1
mkdir -p /tmp/q56r2/g8
CMD=$(grep -F "src/$S.c |" /tmp/q56r2/tbuild.log | head -1)
for g in 0 8; do
  c="${CMD//build\/src\/$S.o/\/tmp\/q56r2\/g8\/g$g.o}"
  c="${c/cc1 -O2 -G0/cc1 -O2 -G$g}"
  bash -c "$c" 2>/dev/null
  mipsel-linux-gnu-objdump -dr --no-show-raw-insn /tmp/q56r2/g8/g$g.o | sed 's/^ *[0-9a-f]*:\t//' > /tmp/q56r2/g8/g$g.dis
done
python3 - <<'PY'
import re
def fns(p):
    d, cur = {}, None
    for l in open(p):
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", l)
        if m: cur = m.group(1); d[cur] = []; continue
        if cur and l.strip(): d[cur].append(re.sub(r"[0-9a-f]+ <[^>]+>", "T", l.strip()))
    return d
a, b = fns("/tmp/q56r2/g8/g0.dis"), fns("/tmp/q56r2/g8/g8.dis")
n = [f for f in sorted(set(a) | set(b)) if a.get(f) != b.get(f) and not f.startswith(".L")]
print(len(n), "functions differ:", n[:40])
PY
