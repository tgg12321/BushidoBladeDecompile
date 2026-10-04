#!/bin/bash
# tc.sh TU... : compile tmp/p2/wk/src/<TU>.c (or src/<TU>.c) with tmp/p2/wk/include overriding include/,
# using the TU's real pipeline from the base make log, and compare per function with the base snapshot.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
mkdir -p tmp/p2/wk/obj
for t in "$@"; do
  line=$(grep -h -m1 -F "src/$t.c |" tmp/p2/*.make.log | head -1)
  [ -n "$line" ] || { echo "no pipeline for $t"; continue; }
  srcf=src/$t.c; [ -f tmp/p2/wk/src/$t.c ] && srcf=tmp/p2/wk/src/$t.c
  d=$(dirname src/$t.c)
  out=tmp/p2/wk/obj/$(echo $t | tr / _).o
  cmd=${line//"-Iinclude -undef"/"-Itmp/p2/wk/include -I$d -Iinclude -undef"}
  cmd=${cmd//" src/$t.c |"/" $srcf |"}
  cmd=${cmd//"-o build/src/$t.o"/"-o $out"}
  eval "$cmd" 2> tmp/p2/wk/err.txt || { echo "COMPILE FAIL $t"; grep -v warning tmp/p2/wk/err.txt | head; continue; }
  python3 - "$t" "$out" <<'PY'
import subprocess, sys, re
t, out = sys.argv[1], sys.argv[2]
def funcs(o):
    s = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-r", "--no-show-raw-insn", o], capture_output=True, text=True).stdout
    d = {}; cur = None
    for line in s.splitlines():
        m = re.match(r"^[0-9a-f]+ <(\S+)>:", line)
        if m: cur = m.group(1); d[cur] = []; continue
        if cur and line.strip(): d[cur].append(re.sub(r"^\s*[0-9a-f]+:\s*", "", line))
    return d
a = funcs(f"tmp/p2/snap/base/obj/{t}.o"); b = funcs(out)
bad = [f for f in sorted(set(a) | set(b)) if a.get(f) != b.get(f)]
print(t, "IDENTICAL" if not bad else "DIFF: " + " ".join(f"{f}({len(a.get(f,[]))}->{len(b.get(f,[]))})" for f in bad))
PY
done
