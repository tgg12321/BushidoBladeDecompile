"""Compare decoded ASPSX output (stdin, from pipe.sh) against a target asm/funcs file, masking
relocated immediates.  usage: pipe.sh X.c | python3 cmp.py asm/funcs/<f>.s"""
import sys, re
tgt = [int(m.group(1)[6:8] + m.group(1)[4:6] + m.group(1)[2:4] + m.group(1)[0:2], 16)
       for m in (re.search(r'/\*\s*[0-9A-F]+\s+[0-9A-F]{8}\s+([0-9A-F]{8})\s*\*/', l) for l in open(sys.argv[1])) if m]
ours = [int(l.split()[1], 16) for l in sys.stdin if re.match(r'^[0-9a-f]{4} [0-9a-f]{8} ', l)]
def mask(w):
    op = w >> 26
    if op in (2, 3): return w & 0xFC000000
    if op in (0x0F, 0x09) or op >= 0x20: return w & 0xFFFF0000
    return w
n = max(len(tgt), len(ours)); bad = 0
for i in range(n):
    t = tgt[i] if i < len(tgt) else None; o = ours[i] if i < len(ours) else None
    ok = t is not None and o is not None and mask(t) == mask(o)
    bad += not ok
    print(f'{i*4:04x} tgt={t and f"{t:08x}"} ours={o and f"{o:08x}"} {"" if ok else "<<"}')
print(f'{len(tgt)} target words, {len(ours)} ours, {bad} differ (reloc immediates masked)')
