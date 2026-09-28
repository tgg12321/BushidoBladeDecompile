"""Build a permuter workspace tmp/c21c/perm_<name> seeded from tmp/c21c/<name>.c.
Prelude = the s2 workspace's standalone type/extern prelude (up to the Env typedef)."""
import os, shutil, sys
name = sys.argv[1]
old = 'tmp/func_8006C21C/perm'
dst = f'tmp/c21c/perm_{name}'
if os.path.exists(dst):
    shutil.rmtree(dst)
os.makedirs(dst)
for f in ('compile.sh', 'settings.toml', 'target.o'):
    shutil.copy(f'{old}/{f}', f'{dst}/{f}')
pre = open(f'{old}/base.c').read()
pre = pre[:pre.index('typedef struct {\n    s32 *header;')]
body = open(f'tmp/c21c/{name}.c').read()
body = '\n'.join(l for l in body.split('\n') if 'SANDBOX-ONLY' not in l)
open(f'{dst}/base.c', 'w', newline='\n').write(pre + body + '\n')
print(dst)
