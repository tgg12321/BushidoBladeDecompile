#!/bin/bash
# fullchain.sh <commit>: test clone /tmp/q56r2/f of MAIN at <commit> (git clone --shared, read-only use of main's
# objects); apply s01..s16 one by one, clean build + SHA1 after each, commit "fNN"; then body-key diffs per step.
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F=/tmp/q56r2/f
rm -rf "$F"
git clone -q --shared --no-checkout "$REPO" "$F" && cd "$F" && git checkout -q -b f "$1" || exit 1
git config user.email f@scratch && git config user.name f-scratch
mkdir -p tmp; ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
printf '/.venv\n/disc\n/tools/gcc-2.7.2\n/tmp\n' >> .git/info/exclude
source .venv/bin/activate
git commit -q --allow-empty -m f00
for n in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16; do
  python3 "$REPO/tmp/q56/adopt/s${n}_apply.py" "$F" > /tmp/q56r2/fapply$n.log 2>&1 || { echo "s$n APPLY FAILED"; tail -6 /tmp/q56r2/fapply$n.log; exit 1; }
  rm -rf build; make -j16 build/bb2.exe > /tmp/q56r2/fbuild$n.log 2>&1
  S=$(sha1sum build/bb2.exe 2>/dev/null | cut -c1-40)
  echo "s$n: $S $([ "$S" = 62efab4f73f992798c43e8c730aa43baa10bb4fa ] && echo ORACLE || echo MISMATCH)"
  [ "$S" = 62efab4f73f992798c43e8c730aa43baa10bb4fa ] || { grep -E "error|Error" /tmp/q56r2/fbuild$n.log | grep -v "missing .end" | head; exit 1; }
  git checkout -q -- metrics/events.jsonl 2>/dev/null
  git add -A . >/dev/null 2>&1; git -c core.hooksPath=/dev/null commit -qm "f$n"
done
echo "== body-key changes per step"
for n in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16; do
  p=$(printf "%02d" $((10#$n - 1)))
  out=$(python3 - "f$p" "f$n" <<'PY'
import re, subprocess, sys
T = "/tmp/q56r2/f"
sys.path.insert(0, T)
from engine import layer2
DEF = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{?\s*$", re.M)
def commit(m):
    return subprocess.run(["git", "-C", T, "log", "--format=%H", f"--grep=^{m}$"], capture_output=True, text=True).stdout.split()[0]
def keys(c):
    out = {}
    fs = [f for f in subprocess.run(["git", "-C", T, "ls-tree", "--name-only", f"{c}:src"], capture_output=True, text=True).stdout.split() if f.endswith(".c")]
    for f in fs:
        t = subprocess.run(["git", "-C", T, "show", f"{c}:src/{f}"], capture_output=True, text=True).stdout
        for n in set(DEF.findall(t)):
            s = layer2.body_source(t, n, read=lambda p: None)
            if s and s[0] == "c":
                out[n] = (layer2._key(s[1]), f)
    return out
a, b = keys(commit(sys.argv[1])), keys(commit(sys.argv[2]))
for n in sorted(set(a) | set(b)):
    if a.get(n, (None,))[0] != b.get(n, (None,))[0]:
        print(f"  {n}: {a.get(n, ('-',))[0]} -> {b.get(n, ('-', ''))[0]} ({(b.get(n) or a.get(n))[1]})")
PY
)
  echo "step $n:"; [ -n "$out" ] && echo "$out"
done
