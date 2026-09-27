"""Q18 record: for each extra nop in the archived output, the archived instruction context and the cc1psx
`#nop` line of the .s it comes from; and the shifted branch's offset delta vs the extra nops between it and
its target.  usage: python3 tmp/func_80036140/nopmap.py <obj> <cc1psx .s> <func>"""
import re, subprocess, sys
sys.path.insert(0, 'tmp/func_80036140')
import lnk

obj, sfile, func = sys.argv[1:4]
t = lnk.text_bytes(open(obj, 'rb').read())
so = lnk.SYMS[func][1]
ends = sorted(o for s, o in lnk.SYMS.values() if s == '.text' and o > so)
eo = ends[0] if ends else len(t)
open('/tmp/nopmap.bin', 'wb').write(t[so:eo])
dis = subprocess.run(['mipsel-linux-gnu-objdump', '-D', '-z', '-b', 'binary', '-m', 'mips:3000', '-EL', '/tmp/nopmap.bin'],
                     capture_output=True, text=True).stdout
ins = {}
for l in dis.splitlines():
    m = re.match(r'^\s+([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)$', l)
    if m:
        ins[int(m.group(1), 16)] = (m.group(2), m.group(3).strip())
# the function's lines in the cc1psx .s, with line numbers
src = open(sfile).read().split('\n')
start = next(i for i, l in enumerate(src) if re.match(r'^' + re.escape(func) + r':', l))
end = next(i for i in range(start + 1, len(src)) if re.match(r'^\s*\.end\s+' + re.escape(func), src[i]))
body = [(i + 1, src[i].strip()) for i in range(start, end) if src[i].strip() and not src[i].strip().startswith('.')]
extra = [int(x, 16) for x in sys.argv[4].split(',')]   # archived byte offsets of the unpaired nops
for off in extra:
    prev = [ins[o] for o in (off - 8, off - 4) if o in ins]
    nxt = ins.get(off + 4)
    print(f'archived +0x{off:03x}: {ins[off][1]}   after: {" ; ".join(p[1] for p in prev)}   before: {nxt[1] if nxt else "-"}')
    # the cc1psx .s: the `#nop` directly following the instruction ASPSX emitted before this nop
    pm = prev[-1][1].split()[0]
    hits = [(n, body[k - 1][1]) for k, (n, l) in enumerate(body) if l == '#nop' and k > 0 and body[k - 1][1].split()[0] == pm]
    print('    cc1psx .s candidates (#nop right after a `' + pm + '`): ' + ', '.join(f'line {n}' for n, _ in hits))
br = [o for o, (w, txt) in ins.items() if txt.startswith(('blez', 'bgtz', 'beq', 'bne', 'bltz', 'bgez'))]
b0 = int(sys.argv[5], 16)
w, txt = ins[b0]
imm = int(w, 16) & 0xFFFF
tgt = b0 + 4 + 4 * imm
n = sum(1 for o in extra if b0 < o < tgt)
print(f'branch at archived +0x{b0:03x}: {txt} (word {w}), offset 0x{imm:x}; shipped offset 0x{int(sys.argv[6], 16):x}; '
      f'delta {imm - int(sys.argv[6], 16)} words; extra nops strictly between branch and target: {n}')
