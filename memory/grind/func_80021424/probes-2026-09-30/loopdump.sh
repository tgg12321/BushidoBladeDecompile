#!/bin/bash
# loopdump.sh <variant>... : cc1 -dL (loop dump) of tmp/prc/<variant>/code6cac_tu2.c; prints func_80022F34's loop
# notes and strength-reduction lines.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
CPP=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CPP_FLAGS, b.CPP_DEFS)")
CC=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CC_FLAGS)")
for v in "$@"; do
  d=tmp/prc/$v
  mipsel-linux-gnu-cpp $CPP -I$d $d/code6cac_tu2.c > $d/tu.i 2>/dev/null
  (cd $d && ../../../tools/gcc-2.7.2/cc1 $CC -dL tu.i -o /dev/null 2>/dev/null)
  python3 - "$d/tu.i.loop" <<'EOF'
import re, sys
s = open(sys.argv[1]).read()
m = re.search(r"\n;; Function func_80022F34\n", s)
e = s.find("\n;; Function ", m.end())
f = s[m.start():e]
print("  loop notes:", len(re.findall(r"NOTE_INSN_LOOP_BEG", f)),
      "| 'giv' lines:", len(re.findall(r"giv", f, re.I)),
      "| 'reduced' lines:", len(re.findall(r"reduc", f, re.I)))
for l in f.splitlines():
    if re.search(r"Loop from|giv|biv|reduc", l):
        print("   ", l[:150])
EOF
  echo "== $v (cmd: cc1 $CC -dL tu.i)"
done
