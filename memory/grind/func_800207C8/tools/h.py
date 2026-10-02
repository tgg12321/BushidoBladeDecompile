"""Private data-model harness for func_800207C8 (tree untouched).

usage (repo root, WSL): python3 tmp/func_800207C8/h.py <candidate.c> [tag] [--q]
Builds a copy of src/code6cac_tu2.c with the candidate substituted and a
modified include/code6cac.h beside it (cpp finds it first), then scores the
function against build/src/code6cac_tu2.o and prints the diff."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score, inlineasm

FUNC = 'func_800207C8'
HDR_EDITS = [
    ("    u8  unk_198[0x1C8 - 0x198];\n",
     "    Vec3i32 unk_198[2];\n"
     "    s32 unk_1B0[2];\n"
     "    u8  unk_1B8[0x1BA - 0x1B8];\n"
     "    s16 unk_1BA;\n"
     "    u8  unk_1BC[0x1C2 - 0x1BC];\n"
     "    s16 unk_1C2;\n"
     "    u8  unk_1C4[0x1C8 - 0x1C4];\n"),
    ("    u8  unk_1EC[0x1F8 - 0x1EC];\n", "    Vec3i32 unk_1EC;\n"),
    ("extern u8 D_8008D864;\nextern s32 D_8008D86C;\nextern s32 D_8008D88C;\n",
     "extern u8 D_8008D864[];\nextern SVec4i16 *D_8008D86C[];\nextern SVec4i16 *D_8008D88C[];\n"
     "extern SVec4i16 D_8008D774[];\nextern SVec4i16 D_800A3138;\n"),
]
import os
if os.environ.get('HDR_FILE'):
    HDR_EDITS = []


def make_header():
    if os.environ.get('HDR_FILE'):
        return Path(os.environ['HDR_FILE']).read_text(encoding='utf-8')
    h = Path('include/code6cac.h').read_text(encoding='utf-8')
    for a, b in HDR_EDITS:
        assert h.count(a) == 1, a
        h = h.replace(a, b)
    return h


def main():
    cand = sys.argv[1]
    tag = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith('--') else Path(cand).stem
    quiet = '--q' in sys.argv
    d = Path('tmp/func_800207C8/w') / tag
    d.mkdir(parents=True, exist_ok=True)
    (d / 'code6cac.h').write_text(make_header(), encoding='utf-8', newline='\n')
    tu = Path('src/code6cac_tu2.c').read_text(encoding='utf-8')
    c = Path(cand).read_text(encoding='utf-8')
    (d / 'code6cac_tu2.c').write_text(inlineasm.substitute_body(tu, FUNC, c), encoding='utf-8', newline='\n')
    out = d / 'tu.o'
    try:
        pipeline.build_c_object('code6cac_tu2', str(out), cheat_overrides={"src_override": str(d / 'code6cac_tu2.c')})
    except RuntimeError as e:
        print(str(e)[-3000:])
        return
    ref = 'build/src/code6cac_tu2.o'
    s = score.score_func(str(out), ref, FUNC)
    print(f"{tag}: score={s['score']} target={s['target_insns']} build={s['build_insns']}")
    if quiet:
        return
    dd = score.insn_diff(str(out), ref, FUNC)
    for h in dd['hunks']:
        print(f"--- {h['class']} {h['tag']} t@{h['target_at']} b@{h['build_at']}")
        n = max(len(h['target']), len(h['built']))
        for i in range(n):
            t = h['target'][i] if i < len(h['target']) else ''
            b = h['built'][i] if i < len(h['built']) else ''
            print(f"   {t:<42} | {b}")


main()
