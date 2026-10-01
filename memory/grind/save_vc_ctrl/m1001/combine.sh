#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/svc/fr_$1"
"/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/gcc-2.7.2/build/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -df -dc tu.i -o /dev/null
python3 - $1 <<'PY'
import sys
f = sys.argv[1]
for p in ("flow", "combine"):
    t = open(f"tu.i.{p}", errors="replace").read()
    i = t.index(f";; Function {f}\n"); j = t.find("\n;; Function ", i + 10)
    open(f"{f}.{p}", "w").write(t[i:j if j > 0 else None])
PY
