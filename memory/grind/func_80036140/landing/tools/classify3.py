"""Q15 (0134264b7) strict classification of an archived-toolchain object vs the shipped function:
every differing word, by address, as (i) archived-cc1psx scheduling — paired 1:1 with a DISTINCT shipped
word of the SAME shipped basic block at a different position, same opcode/registers/masked immediate —
or (ii) ASPSX li expansion — shipped addiu rt,$zero,imm vs archived ori rt,$zero,imm, imm 0..0x7FFF —
else UNCLASSIFIED. Masking: relocation immediates only (lui/%lo/loads/stores/j/jal), branch offsets NOT.
usage: python3 tmp/func_80036140/classify3.py <obj> <func>"""
import re, sys, difflib
sys.path.insert(0, 'tmp/func_80036140')
import lnk

obj, func = sys.argv[1], sys.argv[2]
t = lnk.text_bytes(open(obj, 'rb').read())
allw = [int.from_bytes(t[j:j + 4], 'little') for j in range(0, len(t), 4)]
so = lnk.SYMS[func][1]
ends = sorted(o for s, o in lnk.SYMS.values() if s == '.text' and o > so)
A = allw[so // 4:(ends[0] if ends else len(t)) // 4]

S, SA, ST, blk = [], [], [], []
b, pend_end = 0, False
for l in open(f'asm/funcs/{func}.s'):
    if re.match(r'^\s*(jlabel\s+\S+|\.L\w+:)', l):
        b += 1; continue
    m = re.search(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s*\*/\s*(.*)$', l)
    if not m:
        continue
    if pend_end:
        b += 1; pend_end = False
    w = m.group(2)
    S.append(int(w[6:8] + w[4:6] + w[2:4] + w[0:2], 16)); SA.append(m.group(1)); ST.append(m.group(3).strip()); blk.append(b)
    op = m.group(3).split()[0]
    if op in ('b', 'j', 'jr', 'jal', 'jalr', 'beq', 'bne', 'beqz', 'bnez', 'blez', 'bgtz', 'bltz', 'bgez', 'bal') or op.startswith('b') and op not in ('break',):
        pend_end = 'delay'
    if pend_end == 'delay':
        pend_end = 'next'   # the delay slot belongs to this block
    elif pend_end == 'next':
        pass
# normalise: the flag 'next' means the block ends after the next word (the delay slot)
# (recomputed below for clarity)
S2, blk2, b, cut = [], [], 0, None
blk = []
b = 0
lines = [l for l in open(f'asm/funcs/{func}.s')]
k = -1
end_after = None
for l in lines:
    if re.match(r'^\s*(jlabel\s+\S+|\.L\w+:)', l):
        b += 1; continue
    m = re.search(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s*\*/\s*(\S+)', l)
    if not m:
        continue
    k += 1
    blk.append(b)
    if end_after == k:
        b += 1; end_after = None
    op = m.group(3)
    if op in ('j', 'jr', 'jal', 'jalr', 'beq', 'bne', 'beqz', 'bnez', 'blez', 'bgtz', 'bltz', 'bgez', 'b'):
        end_after = k + 1


def mask(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op in (0x0F, 0x09) or op >= 0x20:
        return w & 0xFFFF0000 if op != 0x09 or ((w >> 21) & 31) != 0 else w   # keep li immediates exact
    return w


def li_pair(s, a):
    return (s >> 26) == 0x09 and ((s >> 21) & 31) == 0 and (a >> 26) == 0x0D and ((a >> 21) & 31) == 0 \
        and ((s >> 16) & 31) == ((a >> 16) & 31) and (s & 0xFFFF) == (a & 0xFFFF) and (s & 0xFFFF) <= 0x7FFF


def norm(a):   # archived ori-li -> addiu form, for alignment only
    if (a >> 26) == 0x0D and ((a >> 21) & 31) == 0:
        return (0x09 << 26) | (a & 0x03FFFFFF)
    return a


MS, MA = [mask(w) for w in S], [mask(norm(w)) for w in A]
sm = difflib.SequenceMatcher(None, MS, MA, autojunk=False)
amap = {}
s_only, a_only = [], []
rows = []
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        for d in range(i2 - i1):
            amap[j1 + d] = i1 + d
            if mask(S[i1 + d]) != mask(A[j1 + d]):
                if li_pair(S[i1 + d], A[j1 + d]):
                    rows.append((SA[i1 + d], ST[i1 + d], f'{A[j1 + d]:08x}', 'li-expansion (same position)'))
                else:
                    rows.append((SA[i1 + d], ST[i1 + d], f'{A[j1 + d]:08x}', 'UNCLASSIFIED'))
        continue
    s_only += range(i1, i2); a_only += range(j1, j2)


def ablock(j):   # shipped block of an archived-only word: its nearest aligned neighbours
    prev = max((amap[x] for x in amap if x < j), default=0)
    nxt = min((amap[x] for x in amap if x > j), default=len(S) - 1)
    return blk[prev] if blk[prev] == blk[nxt] else (blk[prev], blk[nxt])


used = set()
for j in a_only:
    bj = ablock(j)
    cands = [i for i in s_only if i not in used and (blk[i] == bj if not isinstance(bj, tuple) else blk[i] in bj)]
    hit = next((i for i in cands if MS[i] == mask(A[j]) and S[i] != 0 or (S[i] == A[j] == 0 and MS[i] == mask(A[j]))), None)
    if hit is not None and S[hit] == A[j] or (hit is not None and mask(S[hit]) == mask(A[j])):
        used.add(hit)
        rows.append((f'arch+0x{j * 4:03x}', f'pairs {SA[hit]} {ST[hit]}', f'{A[j]:08x}', f'scheduling (block {blk[hit]})'))
        continue
    hit = next((i for i in cands if li_pair(S[i], A[j])), None)
    if hit is not None:
        used.add(hit)
        rows.append((f'arch+0x{j * 4:03x}', f'pairs {SA[hit]} {ST[hit]}', f'{A[j]:08x}', f'li-expansion, other position (block {blk[hit]})'))
        continue
    rows.append((f'arch+0x{j * 4:03x}', '-', f'{A[j]:08x}', f'UNCLASSIFIED (block {bj})'))
for i in s_only:
    if i not in used:
        rows.append((SA[i], ST[i], '-', 'UNCLASSIFIED (no archived partner)'))
print(f'# {func}: shipped {len(S)} words, archived {len(A)} words ({obj}); strict Q15 classification')
for r in rows:
    print(' | '.join(r))
from collections import Counter
c = Counter(r[3].split(' (')[0] for r in rows)
print('# totals:', dict(c))
