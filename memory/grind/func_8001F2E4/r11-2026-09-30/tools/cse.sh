#!/bin/bash
# cse.sh <dumpdir> : re-run the instrumented cc1 on <dumpdir>/tu.i with -ds -dc -df (cse/combine/flow dumps) and cut func_8001F2E4
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT" && cd "$1" || { echo "cd failed: $1"; exit 1; }
"$ROOT/tools/gcc-2.7.2/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -ds -dc -df tu.i -o cse_run.s || echo "cc1 failed"
for f in tu.i.rtl tu.i.cse tu.i.flow tu.i.combine; do
  python3 - "$f" <<'PY'
import sys, re
p = sys.argv[1]
s = open(p).read()
m = re.search(r'\n;; Function func_8001F2E4\n', s)
if m:
    e = s.find('\n;; Function ', m.end())
    open(p + '.fn', 'w').write(s[m.start():e if e > 0 else len(s)])
PY
done
