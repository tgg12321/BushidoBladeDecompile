"""Whole-binary census of small-data accesses in the ORIGINAL code (asm/funcs/*.s = splat of the
shipped EXE).  For every load/store/la whose address falls in the gp window, record
(function, address, mode, width) with mode in {gp, lui, la}.
Outputs:
  - region bounds (sdata = image part, sbss = crt0-cleared part starting 0x800A3308)
  - OFFSET SIGNATURES: an address X accessed via lui (not gp) inside a function that accesses some
    X-k (1<=k<=7) via gp  => "base gp, base+k lui" (the .comm+offset signature)
  - GP-AT-X where the same function also gp-accesses X-k for small k and X is in a matched-C
    `sym+k` list (supplied separately) -- printed by the second script.
usage (WSL, repo root): python3 tmp/research36140/census.py > tmp/research36140/census.txt"""
import re, glob, json
from collections import defaultdict
from pathlib import Path

GP = 0x800A30CC
BSS_START = 0x800A3308  # __SN_ENTRY_POINT clears [0x800A3308, 0x801078E0)

addr = {}
for ln in open('tmp/research36140/nm.txt'):
    parts = ln.split()
    if len(parts) == 3 and not parts[2].endswith('.NON_MATCHING'):
        addr.setdefault(parts[2], int(parts[0], 16))
for f in ('undefined_syms_auto.txt', 'named_syms.txt', 'symbol_addrs.txt'):
    for ln in open(f, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*([A-Za-z_.$][\w.$]*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;', ln)
        if m:
            addr.setdefault(m.group(1), int(m.group(2), 16))

def sym_addr(s):
    m = re.match(r'([A-Za-z_.$][\w.$]*)(?:\s*([+-])\s*(0x[0-9A-Fa-f]+|\d+))?$', s.strip())
    if not m:
        return None
    base = addr.get(m.group(1))
    if base is None:
        mm = re.match(r'D_([0-9A-F]{8})$', m.group(1))
        if not mm:
            return None
        base = int(mm.group(1), 16)
    off = int(m.group(3), 0) if m.group(3) else 0
    return base + (off if m.group(2) != '-' else -off)

W = {'lb': 1, 'lbu': 1, 'sb': 1, 'lh': 2, 'lhu': 2, 'sh': 2, 'lw': 4, 'sw': 4, 'lwl': 4, 'lwr': 4,
     'swl': 4, 'swr': 4}
ins = re.compile(r'\*/\s+(\w+)\s+(.*)$')
acc = []  # (func, file, addr, mode, width, op, vaddr)
unres = set()
for path in sorted(glob.glob('asm/funcs/*.s')):
    func = Path(path).stem
    prev = []  # (op, rest) of earlier instructions in this function
    for ln in open(path, encoding='utf-8', errors='replace'):
        m = ins.search(ln)
        if not m:
            continue
        op, rest = m.group(1), m.group(2)
        prev.append((op, rest))
        va = re.search(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})', ln)
        va = int(va.group(1), 16) if va else 0
        g = re.search(r'%gp_rel\(([^)]+)\)\(\$gp\)', rest)
        l = re.search(r'%lo\(([^)]+)\)', rest)
        if g:
            a = sym_addr(g.group(1)); mode = 'gp'
        elif l and (op in W or op == 'addiu'):
            a = sym_addr(l.group(1)); mode = 'la' if op == 'addiu' else 'lui'
            if mode == 'lui':
                br = re.search(r'\((\$\w+)\)\s*$', rest).group(1)
                mode = 'idx'
                for pop, prest in reversed(prev[:-1][-8:]):
                    dst = prest.split(',')[0].strip()
                    if pop == 'lui' and dst == br:
                        mode = 'lui' if '%hi(' + l.group(1) + ')' in prest else 'idx'
                        break
                    if dst == br and pop not in W or (pop in ('lw','lh','lhu','lb','lbu') and dst == br):
                        break
        else:
            continue
        if a is None:
            unres.add((g or l).group(1)); continue
        if not (GP - 0x8000 <= a <= GP + 0x7FFF):
            continue
        acc.append((func, a, mode, W.get(op, 0), op, va))

gp_addrs = sorted({a for f, a, m, *_ in acc if m == 'gp'})
lo, hi = gp_addrs[0], gp_addrs[-1]
print(f"unresolved symbols: {len(unres)}", sorted(unres)[:200])
print(f'gp-accessed address range: {lo:#x}..{hi:#x}  (sdata part < {BSS_START:#x}, sbss part >=)')
print(f'total gp-window accesses: {len(acc)}; gp={sum(1 for x in acc if x[2]=="gp")} '
      f'lui={sum(1 for x in acc if x[2]=="lui")} la={sum(1 for x in acc if x[2]=="la")}')
gp_funcs = {f for f, a, m, *_ in acc if m == 'gp'}
print(f'functions with >=1 gp access: {len(gp_funcs)}')

# per function: gp addresses
fgp = defaultdict(set)
for f, a, m, *_ in acc:
    if m == 'gp':
        fgp[f].add(a)
all_gp = set(gp_addrs)

# 1. lui/la accesses inside [lo,hi] from gp-enabled functions
print('\n== lui/la accesses into the small-data region [lo,hi] from gp-using functions ==')
rows = []
for f, a, m, w, op, va in acc:
    if m != 'lui' or f not in gp_funcs or not (lo <= a <= hi):
        continue
    near_same = sorted(a - b for b in fgp[f] if 0 < a - b <= 7)
    near_any = sorted(a - b for b in all_gp if 0 < a - b <= 7)
    base_gp_same = a in fgp[f]
    base_gp_any = a in all_gp
    rows.append((a, f, m, op, va, near_same, near_any, base_gp_same, base_gp_any))
for r in sorted(rows):
    a, f, m, op, va, ns, na, bs, ba = r
    cls = ('OFFSET-SIG(same fn: base-k gp)' if ns and not bs else
           'X itself gp in same fn' if bs else
           'X gp elsewhere' if ba else
           'near gp elsewhere' if na else 'no gp near')
    print(f'{a:#010x} {f:28} {op:5} {m:3} @{va:#x}  k(same)={ns} k(any)={na[:3]}  -> {cls}')

json.dump([list(x) for x in acc], open('tmp/research36140/acc.json', 'w'))
