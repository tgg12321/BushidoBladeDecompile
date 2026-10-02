"""chk.py <candidate.c> : build every code6cac.h consumer with tmp/func_800207C8/code6cac.h
(and code6cac_tu2.c with the candidate substituted) and compare each object's .text bytes
and every function against build/src/<stem>.o.  Tree untouched.  Run from repo root in WSL."""
import os
import subprocess
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score, inlineasm

cand = sys.argv[1]
hdr = Path(os.environ.get('HDR_FILE', 'tmp/func_800207C8/code6cac.h')).read_text(encoding='utf-8')
stems = [p.stem for p in sorted(Path('src').glob('*.c')) if '#include "code6cac.h"' in p.read_text(encoding='utf-8', errors='replace')]
bad = 0
for stem in stems:
    d = Path('tmp/func_800207C8/chk') / stem
    d.mkdir(parents=True, exist_ok=True)
    (d / 'code6cac.h').write_text(hdr, encoding='utf-8', newline='\n')
    mod = Path(f'tmp/func_800207C8/srcmod/{stem}.c')
    src = (mod if mod.exists() else Path(f'src/{stem}.c')).read_text(encoding='utf-8')
    if stem == 'code6cac_tu2' and not os.environ.get('NOCAND'):
        src = inlineasm.substitute_body(src, 'func_800207C8', Path(cand).read_text(encoding='utf-8'))
    (d / f'{stem}.c').write_text(src, encoding='utf-8', newline='\n')
    out = d / f'{stem}.o'
    try:
        pipeline.build_c_object(stem, str(out), cheat_overrides={"src_override": str(d / f'{stem}.c')})
    except RuntimeError as e:
        print(stem, 'BUILD FAILED', str(e)[-1500:])
        bad += 1
        continue
    ref = f'build/src/{stem}.o'
    tref = score._o_func_table(ref)
    tout = score._o_func_table(str(out))
    nd = 0
    for fn in sorted(set(tref) | set(tout)):
        if fn not in tref or fn not in tout:
            print(f'{stem}: {fn} only in {"ref" if fn in tref else "ours"}')
            nd += 1
            continue
        if fn == 'func_800207C8' and stem == 'code6cac_tu2':
            s0 = score.score_func(str(out), ref, fn)['score']
            if s0:
                print(f'{stem}: {fn} score={s0}')
                nd += 1
            continue
        if score.normalized_insns(ref, fn, mask=False) != score.normalized_insns(str(out), fn, mask=False):
            print(f'{stem}: {fn} DIFFERS score={score.score_func(str(out), ref, fn)["score"]}')
            nd += 1
    # whole-section byte compare (data/rodata too)
    for sec in ('.text', '.data', '.rodata', '.sdata', '.bss', '.sbss'):
        a = subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', sec, ref, '/dev/stdout'], capture_output=True).stdout
        b = subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', sec, str(out), '/dev/stdout'], capture_output=True).stdout
        if a != b and not (stem == 'code6cac_tu2' and sec == '.text' and os.environ.get('SKIPTU2TEXT')):
            print(f'{stem}: section {sec} differs ({len(a)} vs {len(b)})')
            nd += 1
    print(f'{stem}: {len(tout)} funcs, {nd} differences')
    bad += nd
print('TOTAL differences:', bad)
