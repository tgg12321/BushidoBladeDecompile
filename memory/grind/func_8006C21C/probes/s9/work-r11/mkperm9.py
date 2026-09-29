#!/usr/bin/env python3
"""Permuter workspace tmp/c21c9/perm_<name> seeded from tmp/c21c9/<name>.c (run under WSL, venv).
Prelude = the s2 standalone workspace's type/extern prelude (tmp/func_8006C21C/perm/base.c up to its
Env typedef); the body is macro-expanded with cpp -P so pycparser sees plain C."""
import os, shutil, subprocess, sys
name = sys.argv[1]
old = 'tmp/func_8006C21C/perm'
dst = f'tmp/c21c9/perm_{name}'
if os.path.exists(dst):
    shutil.rmtree(dst)
os.makedirs(dst)
for f in ('compile.sh', 'settings.toml', 'target.o'):
    shutil.copy(f'{old}/{f}', f'{dst}/{f}')
pre = open(f'{old}/base.c').read()
pre = pre[:pre.index('typedef struct {\n    s32 *header;')]
body = open(f'tmp/c21c9/{name}.c').read()
body = '\n'.join(l for l in body.split('\n') if 'SANDBOX-ONLY' not in l)
raw = f'{dst}/raw.c'
open(raw, 'w', newline='\n').write(pre + body + '\n')
out = subprocess.run(['mipsel-linux-gnu-cpp', '-P', '-undef', raw], capture_output=True, text=True, check=True).stdout
open(f'{dst}/base.c', 'w', newline='\n').write(out)
r = subprocess.run(['bash', f'{dst}/compile.sh', f'{dst}/base.c', '-o', f'{dst}/base.o'], capture_output=True, text=True)
print(dst, 'compile rc', r.returncode, r.stderr[-400:])
