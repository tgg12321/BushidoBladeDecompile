"""-G8 screening (compiler-flags-canonical.md § Screening scope): for a TU of a scratch tree, list every
extern of <= 8 bytes it references that is not in sdata_syms.txt, then build the TU through the FULL
per-file pipeline (the tree's Makefile recipe) once as -G8 and once as -G0 and compare every instruction
that accesses each such extern (offset in its function, bytes, relocation type/symbol/addend).
usage (WSL, repo root): python3 tmp/func_80036140/screen.py <tree> <stem> <outfile>"""
import re, subprocess, sys
from pathlib import Path

tree, stem, outf = sys.argv[1:4]
T = Path('tmp/func_80036140') / tree


def recipe(g):
    cmd = subprocess.run(['make', '-C', str(T), '-n', '-B', f'build/src/{stem}.o'], capture_output=True, text=True).stdout
    line = [l for l in cmd.splitlines() if 'maspsx' in l and f'-o build/src/{stem}.o' in l][-1]
    assert (' -G8 ' in line) or (' -G0 ' in line)
    line = re.sub(r' -G[08] -funsigned-char', f' -{g} -funsigned-char', line)
    return line.replace(f'-o build/src/{stem}.o', f'-o build/src/{stem}.{g}.o')


def objdump(o):
    return subprocess.run(['mipsel-linux-gnu-objdump', '-dr', str(T / o)],
                          capture_output=True, text=True).stdout


lines = {}
for g in ('G8', 'G0'):
    r = recipe(g)
    subprocess.run(['bash', '-o', 'pipefail', '-c', r], cwd=T, check=True, capture_output=True)
    lines[g] = r
# cc1 -G8's own record of every extern it references and its size
cc1 = re.sub(r'\| python3 tools/prologue_fix.py.*$', '', lines['G8'].split(f' -o build')[0])
cc1 = lines['G8'].split('| PROLOGUE_CONFIG')[0].split('| python3 tools/prologue_fix.py')[0]
asm = subprocess.run(['bash', '-o', 'pipefail', '-c', cc1], cwd=T, capture_output=True, text=True).stdout
ext = {m.group(1): int(m.group(2)) for m in re.finditer(r'^\s*\.extern\s+(\w+),\s*(\d+)', asm, re.M)}
sdata = {l.strip() for l in (T / 'sdata_syms.txt').read_text().splitlines() if l.strip() and not l.startswith('#')}
small = sorted(s for s, n in ext.items() if n <= 8 and s not in sdata)


def accesses(dump, sym):
    out, func, base = [], None, 0
    rows = dump.splitlines()
    for i, l in enumerate(rows):
        m = re.match(r'^([0-9a-f]+) <(\w+)>:', l)
        if m:
            func, base = m.group(2), int(m.group(1), 16)
            continue
        m = re.match(r'^\s+([0-9a-f]+):\s+R_MIPS_(\w+)\s+(\S+)', l)
        if m and re.fullmatch(re.escape(sym) + r'(\+0x[0-9a-f]+)?', m.group(3)):
            off = int(m.group(1), 16)
            insn = next((r for r in reversed(rows[:i]) if re.match(r'^\s+%x:' % off, r)), '?')
            out.append(f'{func}+0x{off - base:x}  {insn.split(":", 1)[1].strip()}  R_MIPS_{m.group(2)} {m.group(3)}')
    return out


d8, d0 = objdump(f'build/src/{stem}.G8.o'), objdump(f'build/src/{stem}.G0.o')
rep = [f'# -G8 screening, {stem}.c (tree {tree})', f'# -G8 recipe: {lines["G8"]}', f'# -G0 recipe: {lines["G0"]}',
       f'# externs cc1 -G8 reports (.extern name,size): {len(ext)}; <= 8 bytes and not in sdata_syms.txt: {small}']
allsame = True
for s in small:
    a8, a0 = accesses(d8, s), accesses(d0, s)
    key = lambda xs: [x.split("  ", 1)[1] for x in xs]   # instruction + relocation; the offset may shift
    same = key(a8) == key(a0)
    allsame &= same
    rep.append(f'\n## {s} (size {ext[s]}): {len(a8)} accesses at -G8, {len(a0)} at -G0 -> {"IDENTICAL" if same else "DIFFERENT"}')
    rep += [f'  G8 {x}' for x in a8] + [f'  G0 {x}' for x in a0]
rep.append(f'\nRESULT: {"every screened access identical" if allsame else "SOME ACCESS DIFFERS"}')
Path(outf).parent.mkdir(parents=True, exist_ok=True)
Path(outf).write_text('\n'.join(rep) + '\n')
print('\n'.join(l for l in rep if l.startswith(('#', '\n##', 'RESULT'))))
