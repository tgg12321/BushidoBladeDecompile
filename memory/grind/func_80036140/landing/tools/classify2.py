"""Owner Q15 record: every word of an ASPSX-assembled calibration object that differs from the shipped
function, by address and class, plus every gp / sym+N decision for the listed symbols.
usage: python3 tmp/func_80036140/classify2.py <obj> <func> <sym>...
classes: li-expansion (shipped addiu rt,$zero,imm vs ASPSX 2.34 ori rt,$zero,imm), branch-displacement
(same branch, offset shifted by a reordered/added word), scheduling-moved (the same instruction, other
position), scheduling-nop (a hazard nop the reordering adds/removes), UNCLASSIFIED."""
import re, sys, difflib
from collections import Counter
sys.path.insert(0, 'tmp/func_80036140')
import lnk

obj, func, syms = sys.argv[1], sys.argv[2], sys.argv[3:]
t = lnk.text_bytes(open(obj, 'rb').read())
ours_all = [int.from_bytes(t[j:j + 4], 'little') for j in range(0, len(t), 4)]
so = lnk.SYMS[func][1]
ends = sorted(o for s, o in lnk.SYMS.values() if s == '.text' and o > so)
eo = ends[0] if ends else len(t)
ours = ours_all[so // 4:eo // 4]
relocs = {off - so: r for off, r in lnk.RELOCS.items() if so <= off < eo}
lines = [l for l in open(f'asm/funcs/{func}.s') if re.search(r'/\*\s*[0-9A-F]+\s+[0-9A-F]{8}\s+[0-9A-F]{8}\s*\*/', l)]
tw = []
for l in lines:
    m = re.search(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s*\*/\s*(.*)$', l)
    w = m.group(2)
    tw.append((m.group(1), int(w[6:8] + w[4:6] + w[2:4] + w[0:2], 16), m.group(3).strip()))
base_addr = int(tw[0][0], 16)


def mask(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op in (0x0F, 0x09) or op >= 0x20:
        return w & 0xFFFF0000
    return w


def is_li(a, b):
    return a >> 26 == 0x09 and b >> 26 == 0x0D and ((a >> 21) & 31) == 0 and (a & 0x03FFFFFF) == (b & 0x03FFFFFF)


def is_branch(w):
    return (w >> 26) in (1, 4, 5, 6, 7)


T = [mask(w) for _, w, _ in tw]
O = [mask(w) for w in ours]
sm = difflib.SequenceMatcher(None, T, O, autojunk=False)
rows, t_only, o_only = [], [], []
amap = {}
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        for k in range(i2 - i1):
            amap[i1 + k] = j1 + k
        continue
    n = min(i2 - i1, j2 - j1) if tag == 'replace' else 0
    for k in range(n):
        a, b = tw[i1 + k][1], ours[j1 + k]
        amap[i1 + k] = j1 + k
        if is_li(a, b):
            rows.append((tw[i1 + k][0], f'{a:08x} {tw[i1 + k][2]}', f'{b:08x}', 'li-expansion'))
        elif is_branch(a) and (a & 0xFFFF0000) == (b & 0xFFFF0000):
            rows.append((tw[i1 + k][0], f'{a:08x} {tw[i1 + k][2]}', f'{b:08x}', 'branch-displacement'))
        else:
            t_only.append(i1 + k); o_only.append(j1 + k)
    t_only += list(range(i1 + n, i2)); o_only += list(range(j1 + n, j2))
def norm(w):   # ASPSX 2.34's `li` form -> the shipped addiu form (for matching moved instructions)
    return (0x09 << 26) | (w & 0x03FFFFFF) if (w >> 26) == 0x0D and ((w >> 21) & 31) == 0 else w


NO = {j: mask(norm(ours[j])) for j in o_only}
tc, oc = Counter(T[i] for i in t_only), Counter(NO[j] for j in o_only)
for i in t_only:
    a = tw[i][1]
    if a == 0:
        rows.append((tw[i][0], f'{a:08x} {tw[i][2]}', '-', 'scheduling-nop'))
    elif oc[T[i]] > 0:
        oc[T[i]] -= 1
        cls = 'scheduling-moved' + (' + li-expansion' if a >> 26 == 0x09 and ((a >> 21) & 31) == 0 and T[i] not in Counter(O[j] for j in o_only) else '')
        rows.append((tw[i][0], f'{a:08x} {tw[i][2]}', '(same instruction elsewhere)', cls))
    else:
        rows.append((tw[i][0], f'{a:08x} {tw[i][2]}', '-', 'UNCLASSIFIED'))
tc2 = Counter(T[i] for i in t_only)
for j in o_only:
    b = ours[j]
    where = f'ours+0x{j * 4:03x}'
    if b == 0:
        rows.append((where, '-', f'{b:08x}', 'scheduling-nop'))
    elif tc2[NO[j]] > 0:
        tc2[NO[j]] -= 1
        rows.append((where, '(same instruction elsewhere)', f'{b:08x}', 'scheduling-moved' + (' + li-expansion' if NO[j] != O[j] else '')))
    else:
        rows.append((where, '-', f'{b:08x}', 'UNCLASSIFIED'))
print(f'# {func}: {len(tw)} shipped words, {len(ours)} words in {obj}')
print('# every differing word: address | shipped | ASPSX output | class')
for r in rows:
    print(' | '.join(r))
cnt = Counter(r[3] for r in rows)
print('# totals:', dict(cnt))
print('\n# gp / sym+N decisions for', syms)
for i, (addr, w, text) in enumerate(tw):
    m = re.search(r'%(gp_rel|lo)\(((?:' + '|'.join(re.escape(s) for s in syms) + r')\w*|D_800A36B[9AB])\)', text)
    if not m:
        continue
    j = amap.get(i)
    ow = ours[j] if j is not None else None
    if i in t_only:   # a moved instruction: its twin in the pool has the same masked word -> same base register
        twin = [jj for jj in o_only if mask(ours[jj]) == T[i]]
        j = twin[0] if twin else None
        ow = ours[j] if j is not None else None
    ogp = ow is not None and (ow >> 26) >= 0x20 and ((ow >> 21) & 31) == 28
    tgp = m.group(1) == 'gp_rel'
    rel = '+'.join(relocs.get(j * 4, ['?'])) if j is not None else '?'
    print(f'{addr} {text:<50} shipped {"gp" if tgp else "lui/%lo"}  | ASPSX {"gp" if ogp else "non-gp"} ({rel}) {"OK" if tgp == ogp else "MISMATCH"}')
