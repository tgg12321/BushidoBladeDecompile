"""build_flat.py <out_dir> <A|B>: copy the 5 files from the repo tree into a scratch root, apply the
data model (apply_dm --c4 array) + patch_cd (B: + func_80020E74 body) + patch_4e0 + patch_extra; write
flat copies into <out_dir>. Run from repo root (Windows python ok)."""
import shutil, subprocess, sys
from pathlib import Path
out, part = Path(sys.argv[1]), sys.argv[2]
A6 = Path('tmp/func_80020E74/a6')
R = A6 / 'rootF'
shutil.rmtree(R, ignore_errors=True); shutil.rmtree(out, ignore_errors=True)
FILES = ['include/code6cac.h', 'src/code6cac_tu2.c', 'src/code6cac.c', 'src/ings.c']
for f in FILES:
    (R / f).parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(f, R / f)
shutil.copyfile('bb2.ld', R / 'bb2.ld')
(R / 'x').mkdir()
shutil.copyfile('memory/grind/func_80020E74/dm/apply_dm.py', R / 'x/apply_dm.py')
py = sys.executable
subprocess.run([py, str(R / 'x/apply_dm.py'), str(out), '--c4', 'array'], check=True)
args = [py, str(A6 / 'patch_cd.py'), str(out)] + ([str(A6 / 'e74_array.c')] if part == 'B' else [])
subprocess.run(args, check=True)
subprocess.run([py, str(A6 / 'patch_4e0.py'), str(out)], check=True)
subprocess.run([py, str(A6 / 'patch_extra.py'), str(out)], check=True)
