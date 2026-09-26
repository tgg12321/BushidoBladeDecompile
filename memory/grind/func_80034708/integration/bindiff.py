"""Where does a scratch exe differ from the original? Maps differing words to the nearest symbol."""
import sys, re, bisect
exe = open(sys.argv[1], 'rb').read()
orig = open('disc/SLUS_006.63', 'rb').read()
mp = open(sys.argv[2]).read()
syms = sorted((int(a, 16), n) for a, n in re.findall(r'^\s+0x([0-9a-f]{8})\s+(\w+)\s*$', mp, re.M) if int(a, 16) >= 0x80010000)
addrs = [a for a, _ in syms]
diffs = [i for i in range(0x800, min(len(exe), len(orig)), 4) if exe[i:i + 4] != orig[i:i + 4]]
print(len(diffs), 'differing words')
seen = {}
for off in diffs:
    va = off - 0x800 + 0x80010000
    k = bisect.bisect_right(addrs, va) - 1
    name = syms[k][1] if k >= 0 else '?'
    seen.setdefault(name, []).append(hex(va))
for n, v in list(seen.items())[:20]:
    print(n, len(v), v[:6])
