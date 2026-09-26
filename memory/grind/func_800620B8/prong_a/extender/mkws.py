"""Build a permuter workspace for func_800620B8 from a candidate body.
usage (WSL, repo root): python3 tmp/func_800620B8/perm/mkws.py <candidate.c> <workspace-dir>
base.c = the preprocessed text1b.c TU with the candidate spliced over INCLUDE_ASM and every OTHER
function body reduced to a prototype (codegen of func_800620B8 does not depend on them);
target.s = macro prelude + asm/funcs/func_800620B8.s; compile.sh = the build's cc1 pipeline."""
import os
import re
import subprocess
import sys

cand, ws = sys.argv[1], sys.argv[2]
os.makedirs(ws, exist_ok=True)
v = open(cand).read()
s = open('src/text1b.c').read()
k = 'INCLUDE_ASM("asm/funcs", func_800620B8);'
assert k in s
open(ws + '/full.c', 'w').write(s.replace(k, v))
CPP = ('mipsel-linux-gnu-cpp -P -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 '
       '-D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL '
       '-D_LANGUAGE_C -DLANGUAGE_C').split()
pp = subprocess.run(CPP + [ws + '/full.c'], capture_output=True, text=True).stdout
os.remove(ws + '/full.c')

# strip other function bodies and top-level asm blocks
out = []
i = 0
n = len(pp)
while i < n:
    m = re.compile(r'__asm__\s*\(').match(pp, i)
    if m and (i == 0 or pp[i - 1] == '\n'):
        # top-level asm block: skip to matching ');'
        depth = 0
        j = m.end() - 1
        instr = False
        while j < n:
            c = pp[j]
            if instr:
                if c == '\\':
                    j += 1
                elif c == '"':
                    instr = False
            elif c == '"':
                instr = True
            elif c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        j += 1
        while j < n and pp[j] in ' \t;\n':
            j += 1
        i = j
        continue
    c = pp[i]
    if c == '{':
        prev = ''.join(out).rstrip()
        is_fn = prev.endswith(')') and not re.search(r'=\s*$', prev)
        name = re.search(r'(\w+)\s*\([^()]*(\([^()]*\)[^()]*)*\)\s*$', prev)
        keep = name and name.group(1) == 'func_800620B8'
        # scan to matching brace
        depth = 0
        j = i
        instr = None
        while j < n:
            ch = pp[j]
            if instr:
                if ch == '\\':
                    j += 1
                elif ch == instr:
                    instr = None
            elif ch in '"\'':
                instr = ch
            elif ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        if is_fn and not keep:
            out.append(';\n')
        else:
            out.append(pp[i:j + 1])
        i = j + 1
        continue
    out.append(c)
    i += 1
base = ''.join(out)
assert 'void func_800620B8(' in base
open(ws + '/base.c', 'w').write(base)

pre = open('tmp/func_8005D814/perm1/target.s').read()
pre = pre[:pre.index('glabel func_8005D814')]
open(ws + '/target.s', 'w').write(pre + open('asm/funcs/func_800620B8.s').read())
open(ws + '/settings.toml', 'w').write('func_name = "func_800620B8"\ncompiler_type = "gcc"\n')
cs = open('tmp/func_8005D814/perm1/compile.sh').read()
open(ws + '/compile.sh', 'w').write(cs)
os.chmod(ws + '/compile.sh', 0o755)
r = subprocess.run(['mipsel-linux-gnu-as', '-Iinclude', '-march=r3000', '-mtune=r3000', '-no-pad-sections',
                    '-O1', '-G0', '-o', ws + '/target.o', ws + '/target.s'], capture_output=True, text=True)
assert r.returncode == 0, r.stderr
r = subprocess.run(['bash', ws + '/compile.sh', ws + '/base.c', '-o', ws + '/base.o'], capture_output=True, text=True)
print('compile rc', r.returncode, r.stderr[-500:])
print('base.c lines', base.count('\n'))
