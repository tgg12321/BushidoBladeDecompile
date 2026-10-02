"""Reproduce the rejected manual-session floors; run from repo root in WSL.

python3 memory/grind/func_80048FFC/tools/codex_probe.py
All substitutions/build products stay in tmp/. No production inputs change.
"""
from pathlib import Path
import sys

sys.path.insert(0, '.')
from engine import cheats, inlineasm, pipeline, score
from engine.cli import _print_insn_diff

ledger = Path('memory/grind/func_80048FFC')
work = Path('tmp/codex_f48ffc_receipts')
work.mkdir(parents=True, exist_ok=True)
src = Path('src/text1b.c').read_text()
start = src.index('extern u8 D_800EF848[];\n')
marker = 'INCLUDE_ASM("asm/funcs", func_80048FFC);\n'
end = src.index(marker) + len(marker)

for name in ('castfree-plain-floor13', 'f6-scheduling-floor4'):
    region = (ledger / 'rejected' / (name + '.c')).read_text()
    full = src[:start] + region + src[end:]
    full = full.replace('#include "gte.h"\n', '#include "gte.h"\n#include "gpu.h"\n', 1)
    full, _ = inlineasm.strip_cheat_asm_file(full)
    candidate = work / (name + '.c')
    candidate.write_text(full, newline='\n')
    overrides = cheats.empty_overrides(str(work / 'cfg'))
    overrides['src_override'] = str(candidate)
    obj = str(work / (name + '.o'))
    pipeline.build_c_object('text1b', obj, cheat_overrides=overrides)
    print(name, score.score_func(obj, 'build/src/text1b.o', 'func_80048FFC'))
    _print_insn_diff(score.insn_diff(obj, 'build/src/text1b.o', 'func_80048FFC'))

gpu = Path('src/gpu.c').read_text()
span = inlineasm._func_body_span(gpu, 'SetDrawMove')
assert span is not None
typed = (ledger / 'tools' / 'gpu-typed-proposal.c').read_text()
gpu = gpu[:span[0]] + typed + gpu[span[1]:]
gpu, _ = inlineasm.strip_cheat_asm_file(gpu)
candidate = work / 'gpu.c'
candidate.write_text(gpu, newline='\n')
overrides = cheats.empty_overrides(str(work / 'gpu_cfg'))
overrides['src_override'] = str(candidate)
obj = str(work / 'gpu.o')
pipeline.build_c_object('gpu', obj, cheat_overrides=overrides)
print('typed SetDrawMove proposal', score.score_func(obj, 'build/src/gpu.o', 'SetDrawMove'))
_print_insn_diff(score.insn_diff(obj, 'build/src/gpu.o', 'SetDrawMove'))
