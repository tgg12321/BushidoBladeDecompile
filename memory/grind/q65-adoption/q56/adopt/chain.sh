#!/bin/bash
# chain.sh [B]: test clone from step07 (old series tag; read-only use of the adopt clone): apply the NEW s08..s16
# one by one, clean build + SHA1 after each. With B: after s12, maspsx 8-byte statics align 4 (Sony probe).
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash "$REPO/tmp/q56/r2/tclone.sh" step07 >/dev/null || exit 1
cd /tmp/q56r2/t && source .venv/bin/activate
for n in 08 09 10 11 12 13 14 15 16; do
  python3 "$REPO/tmp/q56/adopt/s${n}_apply.py" /tmp/q56r2/t > /tmp/q56r2/apply$n.log 2>&1 || { echo "s$n APPLY FAILED"; tail -5 /tmp/q56r2/apply$n.log; exit 1; }
  if [ "$n" = "12" ] && [ "$1" = "B" ]; then sed -i 's/res.append("\\t.align 3")/res.append("\\t.align 2")/' tools/maspsx/maspsx/__init__.py; grep -c 'align 3' tools/maspsx/maspsx/__init__.py; fi
  echo "s$n: $(bash "$REPO/tmp/q56/r2/tbuild.sh" | head -3 | tr '\n' ' ')"
  git add -A . >/dev/null 2>&1; git -c core.hooksPath=/dev/null commit -qm "t$n" >/dev/null 2>&1
done
