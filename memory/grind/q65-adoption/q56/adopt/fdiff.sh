#!/bin/bash
# fdiff.sh <stem>: test clone - compile src/<stem>.c at commit t13 and at the working tree with the tree's own
# Makefile pipeline flags; list functions whose disassembly differs.
cd /tmp/q56r2/t || exit 1
source .venv/bin/activate
S=$1
mkdir -p /tmp/q56r2/fd
git show HEAD~1:src/$S.c > /tmp/q56r2/fd/old.c
cp src/$S.c /tmp/q56r2/fd/new.c
CMD=$(grep -F "src/$S.c |" /tmp/q56r2/tbuild.log | head -1)
for v in old new; do
  c="${CMD//src\/$S.c/\/tmp\/q56r2\/fd\/$v.c}"
  c="${c//build\/src\/$S.o/\/tmp\/q56r2\/fd\/$v.o}"
  bash -c "$c" 2>/dev/null
  mipsel-linux-gnu-objdump -d --no-show-raw-insn /tmp/q56r2/fd/$v.o | sed 's/^ *[0-9a-f]*:\t//' > /tmp/q56r2/fd/$v.dis
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
a, b = fns("/tmp/q56r2/fd/old.dis"), fns("/tmp/q56r2/fd/new.dis")
for f in sorted(set(a) | set(b)):
    if a.get(f) != b.get(f):
        print(f, len(a.get(f, [])), "->", len(b.get(f, [])))
        import difflib
        for x in list(difflib.unified_diff(a.get(f, []), b.get(f, []), n=0, lineterm=""))[2:14]:
            print("   ", x)
PY
