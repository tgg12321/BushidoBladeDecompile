"""patch_cd.py <dir>: rewrite func_80020CDC / func_80020D38 in <dir>/code6cac_tu2.c to the pointer form
and splice the array-model func_80020E74 body (memory/grind/func_80020E74/rejected/array-model...)."""
import sys
from pathlib import Path
d = Path(sys.argv[1])
p = d / "code6cac_tu2.c"
t = p.read_text(encoding="utf-8")
OLD_CDC = """void func_80020CDC(void) {
    if (D_800A38C4[1] == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    D_800A38C4[1] = 0;
    D_800A38C4[0] = 0;
"""
NEW_CDC = """void func_80020CDC(void) {
    u16 *p = D_800A38C4;

    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    p[1] = 0;
    p[0] = 0;
"""
OLD_D38 = """void func_80020D38(void) {
    if (D_800A38C4[1] == 0xFFFF) {
        seq_Reset();
    }
    D_800A38C4[1] = 0;
}
"""
NEW_D38 = """void func_80020D38(void) {
    u16 *p = D_800A38C4;

    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    p[1] = 0;
}
"""
for o, n in ((OLD_CDC, NEW_CDC), (OLD_D38, NEW_D38)):
    assert t.count(o) == 1, o[:40]
    t = t.replace(o, n)
if len(sys.argv) > 2:
    sys.path.insert(0, '.')
    import importlib
    inl = importlib.import_module('engine.inlineasm')
    t = inl.substitute_body(t, 'func_80020E74', Path(sys.argv[2]).read_text(encoding='utf-8'))
open(p, "w", newline="\n", encoding="utf-8").write(t)
print("patched", p)
