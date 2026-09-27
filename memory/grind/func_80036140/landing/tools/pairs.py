"""Q15 (i) full pairing for the reordered blocks: in each shipped basic block that the archived output
reorders, pair every non-nop word 1:1 with the archived word of the same masked value (li normalised,
first-unused in order) and list every pair whose position inside the block (nops set aside) differs.
usage: python3 tmp/func_80036140/pairs.py <obj> <func> <block,...>"""
import re, sys
sys.path.insert(0, 'tmp/func_80036140')
import lnk

obj, func, want = sys.argv[1], sys.argv[2], [int(x) for x in sys.argv[3].split(',')]
t = lnk.text_bytes(open(obj, 'rb').read())
so = lnk.SYMS[func][1]
ends = sorted(o for s, o in lnk.SYMS.values() if s == '.text' and o > so)
A = [int.from_bytes(t[j:j + 4], 'little') for j in range(so, ends[0] if ends else len(t), 4)]


def mask(w):
    op = w >> 26
    if op == 0x0D and ((w >> 21) & 31) == 0:
        w = (0x09 << 26) | (w & 0x03FFFFFF)
        op = 0x09
    if op in (2, 3):
        return w & 0xFC000000
    if op in (0x0F,) or op >= 0x20 or (op == 0x09 and ((w >> 21) & 31) != 0):
        return w & 0xFFFF0000
    if op in (1, 4, 5, 6, 7):
        return w & 0xFFFF0000       # branch: offset compared separately (Q18 iii-b)
    return w


S, SA, ST, blk = [], [], [], []
b, k, end_after = 0, -1, None
for l in open(f'asm/funcs/{func}.s'):
    if re.match(r'^\s*(jlabel\s+\S+|\.L\w+:)', l):
        b += 1
        continue
    m = re.search(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s*\*/\s*(.*)$', l)
    if not m:
        continue
    k += 1
    w = m.group(2)
    S.append(int(w[6:8] + w[4:6] + w[2:4] + w[0:2], 16)); SA.append(m.group(1)); ST.append(m.group(3).strip()); blk.append(b)
    if end_after == k:
        b += 1; end_after = None
    if m.group(3).split()[0] in ('j', 'jr', 'jal', 'jalr', 'beq', 'bne', 'beqz', 'bnez', 'blez', 'bgtz', 'bltz', 'bgez', 'b'):
        end_after = k + 1
# archived blocks: same count of blocks in the same order; walk the archived words with the shipped
# block sizes plus the nops ASPSX added (a block's archived extent ends where its last non-nop word ends)
pos = 0
for bb in range(max(blk) + 1):
    idx = [i for i in range(len(S)) if blk[i] == bb]
    nonnop_s = [i for i in idx if S[i] != 0]
    # archived extent: take words until the same number of non-nop words has been consumed
    ext, n = [], 0
    while n < len(nonnop_s) and pos < len(A):
        ext.append(pos)
        if A[pos] != 0:
            n += 1
        pos += 1
    while pos < len(A) and A[pos] == 0 and bb < max(blk) and len(ext) < len(idx) + 4 and \
            not (pos < len(A) and False):
        break
    # trailing nops belonging to this block (delay-slot nop etc.)
    while pos < len(A) and A[pos] == 0 and len([x for x in ext]) < len(idx) + (4 if bb in want else 0) and \
            sum(1 for x in idx if S[x] == 0) > sum(1 for x in ext if A[x] == 0):
        ext.append(pos); pos += 1
    if bb not in want:
        continue
    nonnop_a = [j for j in ext if A[j] != 0]
    used = set()
    print(f'## block {bb}: shipped {SA[idx[0]]}..{SA[idx[-1]]} ({len(idx)} words), archived +0x{ext[0] * 4:03x}..+0x{ext[-1] * 4:03x} ({len(ext)} words)')
    for r, i in enumerate(nonnop_s):
        j = next((j for j in nonnop_a if j not in used and mask(A[j]) == mask(S[i])), None)
        if j is None:
            print(f'   {SA[i]} {ST[i]:<44} -> NO PARTNER'); continue
        used.add(j)
        ra = nonnop_a.index(j)
        tag = 'moved' if ra != r else 'same position'
        print(f'   {SA[i]} {ST[i]:<44} <-> archived +0x{j * 4:03x} {A[j]:08x}  ({tag}: shipped #{r}, archived #{ra} of the non-nop words)')
    rest = [j for j in nonnop_a if j not in used]
    if rest:
        print('   archived words without a shipped partner:', ', '.join(f'+0x{j * 4:03x}' for j in rest))
