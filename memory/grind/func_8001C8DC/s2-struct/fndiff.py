"""fndiff.py <func> [<func> ...]: disassemble each function from the scratch tree's EXE (address from its
bb2.map) and from the original EXE (address from asm/funcs/<func>.s glabel), and diff the instruction text
with absolute branch/jump targets and %hi/%lo-bearing immediates left as-is (a pure shift shows up as
target-only diffs; a real codegen difference shows as a different opcode/register sequence or length).
Run in WSL from the repo root."""
import re, subprocess, sys, difflib

TREE = 'tmp/c8dc2/tree/build'
syms = {}
for l in open(f'{TREE}/bb2.map', errors='replace'):
    m = re.match(r'\s+0x([0-9a-f]{8})\s+(\S+)\s*$', l)
    if m:
        syms[m.group(2)] = int(m.group(1), 16)
order = sorted(syms.items(), key=lambda kv: kv[1])

def next_addr(a):
    for _, v in order:
        if v > a:
            return v
    return a + 0x1000

def dis(path, start, n):
    off = start - 0x80010000 + 0x800
    data = open(path, 'rb').read()[off:off + n]
    open('/tmp/fndiff.bin', 'wb').write(data)
    out = subprocess.run(['mipsel-linux-gnu-objdump', '-D', '-b', 'binary', '-m', 'mips',
                          f'--adjust-vma={start:#x}', '/tmp/fndiff.bin'], capture_output=True, text=True).stdout
    lines = []
    for l in out.splitlines():
        parts = l.split('\t')
        if len(parts) >= 3 and parts[0].strip().endswith(':'):
            lines.append('\t'.join(p.strip() for p in parts[2:]))
    return lines

for f in sys.argv[1:]:
    ours = syms[f]
    size_ours = next_addr(ours) - ours
    g = open(f'asm/funcs/{f}.s').read()
    ninsn = len(re.findall(r'^\s*/\* [0-9A-F]+ [0-9A-F]{8} [0-9A-F]{8} \*/', g, re.M))
    m = re.search(r'/\* [0-9A-F]+ ([0-9A-F]{8}) ', g)
    tgt = int(m.group(1), 16)
    a = dis(f'{TREE}/bb2.exe', ours, size_ours)
    b = dis('disc/SLUS_006.63', tgt, ninsn * 4)
    print(f'== {f}: ours {ours:#x} {len(a)} insns (to next map symbol), target {tgt:#x} {len(b)} insns')
    strip = lambda xs: [re.sub(r'0x[0-9a-f]+', 'X', x) for x in xs]
    d = list(difflib.unified_diff(strip(b[:len(b)]), strip(a[:len(b) + 4]), 'target', 'ours', n=1, lineterm=''))
    print('\n'.join(d[:60]) if d else '(identical modulo addresses/immediates)')
