"""-G8 prong (iv) move record: each post-split file = a head block + ONE contiguous, byte-identical slice
of the pre-split file; the slices, in order, cover the pre-split file except the reported dropped lines.
usage: python3 tmp/func_80036140/movecheck2.py <pre-file> <post-file>...   (in source order)"""
import sys

pre = open(sys.argv[1]).read().rstrip('\n').split('\n')
pos, ok = 0, True
for f in sys.argv[2:]:
    post = open(f).read().rstrip('\n').split('\n')
    found = None
    for k in range(len(post)):          # smallest head block whose remainder is a slice of pre at/after pos
        tail = post[k:]
        for s in range(pos, len(pre) - len(tail) + 1):
            if pre[s:s + len(tail)] == tail:
                found = (k, s)
                break
        if found:
            break
    if not found:
        print(f'{f}: NO contiguous slice found'); ok = False; continue
    k, s = found
    if s > pos:
        print(f'  dropped pre lines {pos + 1}..{s}:')
        for l in pre[pos:s]:
            print(f'      - {l}')
    e = s + len(post) - k
    print(f'{f}: head block {k} lines, then pre lines {s + 1}..{e} verbatim ({e - s} lines)')
    pos = e
if pos < len(pre):
    print(f'  dropped pre lines {pos + 1}..{len(pre)}'); ok = False
print('RESULT:', 'every moved line byte-identical, nothing else dropped than listed' if ok else 'MISMATCH')
