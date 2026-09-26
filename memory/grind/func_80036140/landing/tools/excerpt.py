"""Excerpt, for banking, the RTL insns around given address uses in one function, followed through
the passes by insn uid (uids persist across cc1's dumps).
usage: python3 tmp/func_80036140/excerpt.py <dumpdir> <stem> <func> <out> <window> <pattern>...
  Every .rtl insn matching a pattern selects itself and the next <window> insns; the same uids are
  then printed from .rtl, .cse, .cse2, .combine, .lreg, .greg."""
import re, sys
from pathlib import Path

d, stem, func, out, win = Path(sys.argv[1]), sys.argv[2], sys.argv[3], Path(sys.argv[4]), int(sys.argv[5])
pats = [re.compile(p, re.S) for p in sys.argv[6:]]


def insns_of(ext):
    p = d / f'{stem}.i.{ext}'
    if not p.exists():
        return None
    t = p.read_text()
    m = re.search(r'^;; Function ' + re.escape(func) + r'\b.*?(?=^;; Function |\Z)', t, re.S | re.M)
    if not m:
        return None
    out = []
    for chunk in re.split(r'\n\n+(?=\()', m.group(0)):
        u = re.match(r'\((?:insn|jump_insn|call_insn|note|code_label|barrier)\s+(\d+)', chunk)
        if u:
            out.append((int(u.group(1)), chunk.strip()))
    return out


rtl = insns_of('rtl')
sel = set()
for i, (uid, txt) in enumerate(rtl):
    if any(p.search(txt) for p in pats):
        sel.update(u for u, _ in rtl[i:i + win + 1])
res = [f'# excerpt of {d.name}/{stem}.i.* for {func}: insns matching {sys.argv[6:]} + {win} following, by uid',
       (d / 'CMD').read_text().strip(), f'# selected uids: {sorted(sel)}']
for ext in ('rtl', 'cse', 'cse2', 'combine', 'lreg', 'greg'):
    ins = insns_of(ext)
    if ins is None:
        continue
    res.append(f'\n======================== .{ext}')
    res.extend(txt for uid, txt in ins if uid in sel)
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text('\n\n'.join(res) + '\n')
print(out, len(sel), 'uids')
