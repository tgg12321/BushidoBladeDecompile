"""implchk.py: the old file's per-function implicit callees vs the parts'. A pair (F, X) implicit in a part
but not reported for F in the old file is fine only when the old file had no explicit declaration of X
before F (X was implicit there too, first reported in an earlier function). The reverse direction must be
empty: a call implicit in the old file must stay implicit."""
import subprocess, sys
sys.argv = [sys.argv[0]]
sys.path.insert(0, "tmp/r4f")
import split4f as S
old = subprocess.run(["git", "show", "HEAD:src/main/psxsdk/libspu/spu.c"], capture_output=True).stdout
open("tmp/r4f/old_main.c", "wb").write(old)
def pairs(files):
    out = subprocess.run(["python3", "tmp/r4e/implicit.py", *files], capture_output=True, text=True).stdout
    return {tuple(l.split("\t")) for l in out.splitlines() if l.strip()}
po = pairs(["tmp/r4f/old_main.c"])
ids = open("tmp/r4f/ids.txt").read().split()
pn = pairs([f"src/{t}.c" for t in ids])
bad = []
for f, x in sorted(pn - po):
    fl = next(i["start"] for i in S.ITEMS if i["kind"] == "func" and i["name"] == f)
    ok = not S.explicit_before(x, fl)
    print("new-site", f, x, "ok (implicit in the old file too)" if ok else "PROTOTYPE LOST")
    bad += [] if ok else [(f, x)]
for f, x in sorted(po - pn):
    print("lost implicit", f, x); bad.append((f, x))
print(len(po), "old pairs;", len(pn), "new pairs;", "IMPLCHK", "OK" if not bad else "FAIL")
