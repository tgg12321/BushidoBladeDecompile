#!/usr/bin/env python3
"""usage (WSL, repo root): python3 tmp/func_8001A820/mk.py <candidate.c> <tag> [--s]
Substitute candidate into src/code6cac.c, cpp with the build's flags, then either
run rtl_track/dump.py (default) or just compile to .s (--s) into tmp/func_8001A820/build_<tag>/."""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm
from engine import buildconfig as cfg

cand, tag = sys.argv[1], sys.argv[2]
out = Path(f'tmp/func_8001A820/build_{tag}')
out.mkdir(parents=True, exist_ok=True)
base = Path('src/code6cac.c').read_text()
body = Path(cand).read_text()
(out / 'full.c').write_text(inlineasm.substitute_body(base, 'func_8001A820', body))
cmd = f"{cfg.CPP} {cfg.CPP_FLAGS} {cfg.CPP_DEFS} {out}/full.c > {out}/full.i"
subprocess.run(cmd, shell=True, check=True)
if '--s' in sys.argv:
    cmd = f"tools/gcc-2.7.2/build/cc1 {cfg.CC_FLAGS} {out}/full.i -o {out}/full.s"
    subprocess.run(cmd, shell=True, check=True)
    print(f'{out}/full.s')
else:
    subprocess.run(f"python3 tools/rtl_track/dump.py {tag} --input {out}/full.i", shell=True, check=True, stdout=subprocess.DEVNULL)
    print(f'tmp/rtl/{tag}')
