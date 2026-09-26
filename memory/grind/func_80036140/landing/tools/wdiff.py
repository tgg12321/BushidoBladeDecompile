"""Show word-level diff of an ASPSX object's function vs the shipped asm/funcs bytes.
usage: python3 tmp/func_80036140/wdiff.py <obj> <func> [max]"""
import re, sys, difflib
sys.path.insert(0, 'tmp/func_80036140')
import lnk
obj, f = sys.argv[1], sys.argv[2]
mx = int(sys.argv[3]) if len(sys.argv) > 3 else 40
t = lnk.text_bytes(open(obj, 'rb').read())
ours = [int.from_bytes(t[j:j + 4], 'little') for j in range(0, len(t), 4)]
so = lnk.SYMS[f][1]
ends = sorted(o for s, o in lnk.SYMS.values() if s == '.text' and o > so)
seg = ours[so // 4:(ends[0] // 4 if ends else len(ours))]
lines = [l for l in open(f'asm/funcs/{f}.s') if re.search(r'/\*\s*[0-9A-F]+\s+[0-9A-F]{8}\s+[0-9A-F]{8}\s*\*/', l)]
words = [int((m := re.search(r'/\*\s*[0-9A-F]+\s+[0-9A-F]{8}\s+([0-9A-F]{8})', l)).group(1)[6:8] + m.group(1)[4:6] + m.group(1)[2:4] + m.group(1)[0:2], 16) for l in lines]


def mask(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op in (0x0F, 0x09) or op >= 0x20:
        return w & 0xFFFF0000
    return w


sm = difflib.SequenceMatcher(None, [mask(x) for x in words], [mask(y) for y in seg], autojunk=False)
if '--classify' in sys.argv:
    li = other = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            continue
        if tag == 'replace' and i2 - i1 == j2 - j1:
            for x, y in zip(words[i1:i2], seg[j1:j2]):
                # addiu rt,$zero,imm (target) vs ori rt,$zero,imm (ASPSX 2.34 `li` expansion), same rt/imm
                if x >> 26 == 0x09 and y >> 26 == 0x0D and (x & 0x03FFFFFF) == (y & 0x03FFFFFF) and (x >> 21) & 31 == 0:
                    li += 1
                else:
                    other += 1
        else:
            other += max(i2 - i1, j2 - j1)
    print(f'{f}: {len(words)} target words; differing: li-expansion (addiu vs ori, same reg+imm) = {li}, other = {other}')
    sys.exit(0)
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        continue
    print(f'--- {tag} target[{i1}:{i2}] ours[{j1}:{j2}]')
    for i in range(i1, i2):
        print('  T', lines[i].strip()[:110])
    for j in range(j1, j2):
        print(f'  O {seg[j]:08x}')
    n += 1
    if n >= mx:
        break
