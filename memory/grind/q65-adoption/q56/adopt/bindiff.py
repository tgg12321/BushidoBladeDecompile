#!/usr/bin/env python3
"""bindiff.py: the scratch build's bb2.bin vs the oracle image, grouped into differing ranges with the
function / data symbol each range falls in (from the scratch build's ELF)."""
import bisect, subprocess
A = "/tmp/q56/adopt tree"
ref = open(A + "/disc/SLUS_006.63", "rb").read()[0x800:]
got = open(A + "/build/bb2.bin", "rb").read()
print("sizes", len(ref), len(got))
syms = []
for l in subprocess.run(f"mipsel-linux-gnu-nm -n '{A}/build/bb2.elf'", shell=True, capture_output=True, text=True).stdout.splitlines():
    p = l.split()
    if len(p) == 3 and p[1] in "TtDdBbRr":
        syms.append((int(p[0], 16), p[2]))
addrs = [a for a, _ in syms]
diffs = [i for i in range(min(len(ref), len(got))) if ref[i] != got[i]]
ranges, cur = [], None
for i in diffs:
    if cur and i - cur[1] <= 8:
        cur[1] = i
    else:
        cur = [i, i]; ranges.append(cur)
print("differing bytes", len(diffs), "ranges", len(ranges))
for s, e in ranges[:60]:
    a = 0x80010000 + s
    k = bisect.bisect_right(addrs, a) - 1
    nm = syms[k][1] if k >= 0 else "?"
    print(f"  {a:08x}..{0x80010000 + e:08x} ({e - s + 1} B) in {nm}+{a - syms[k][0]:#x}  ref={ref[s:s+8].hex()} got={got[s:s+8].hex()}")
