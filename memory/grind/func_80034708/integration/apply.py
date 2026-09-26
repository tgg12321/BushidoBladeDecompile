"""Apply the func_80034708 landing to the live tree (run ONLY under the landing lock).
Regenerates every integ/ copy from the CURRENT tree first, then copies the changed files in.
Prints the exact changed-path list (for git add)."""
import os, shutil, subprocess, filecmp, sys
R = 'tmp/func_80034708/integ'
for s in ('mk_integ3.py', 'mk_cfg.py', 'mk_ld.py'):
    subprocess.run(['python3', f'tmp/func_80034708/{s}'], check=True)

changed = []
def put(src, dst):
    if os.path.exists(dst) and filecmp.cmp(src, dst, shallow=False):
        return
    shutil.copyfile(src, dst)
    changed.append(dst)

for h in ('code6cac.h', 'system.h'):
    put(f'{R}/include/{h}', f'include/{h}')
for f in sorted(os.listdir(f'{R}/src')):
    if f.endswith('.c'):
        put(f'{R}/src/{f}', f'src/{f}')
for f in ('undefined_syms_auto.txt', 'named_syms.txt', 'symbol_addrs.txt', 'sdata_syms.txt', 'bb2.ld'):
    put(f'{R}/{f}', f)
put(f'{R}/asm/data/91C98.data.s', 'asm/data/91C98.data.s')

def edit(path, old, new):
    s = open(path, encoding='utf-8').read()
    assert s.count(old) == 1, (path, old)
    open(path, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))
    changed.append(path)

edit('Makefile', 'GP_FILES := text1a_pre text1a_post\n', 'GP_FILES := text1a_pre text1a_post code6cac_b3\n')
edit('Makefile', "# project's sole -G8 file, and under -G8", "# project's first -G8 file, and under -G8")
edit('engine/buildconfig.py', 'GP_FILES = {"text1a_pre", "text1a_post"}\n',
     'GP_FILES = {"text1a_pre", "text1a_post", "code6cac_b3"}\n')
print('\n'.join(changed))
open('tmp/func_80034708/applied_paths.txt', 'w', newline='\n').write('\n'.join(changed) + '\n')
