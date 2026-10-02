"""apply_final.py: put r280/final.c (+ a current header comment) into the working tree AND the index copy
of src/code6cac_tu2.c. Prints the new index blob hash (stage with git update-index --cacheinfo)."""
import subprocess
import sys
from pathlib import Path
sys.path.insert(0, '.')
import importlib
inl = importlib.import_module('engine.inlineasm')

body = Path('tmp/func_80020E74/a6/r280/final.c').read_text(encoding='utf-8')
OLD_START = "/*\n * func_80021280 — BYTES PROVEN"
OLD_END = " * git history of this file) and the post-RA-scheduling forensics frontier.\n */\n"
NEW = """/*
 * func_80021280: the record's model id (unk_48) -> its slot in D_800A38C4 (unk_4A); for ids below
 * 0x2000, the nibble positions of 4 and 5 (unk_88 / unk_8E), copying unk_26C into unk_8A / unk_90
 * under mode-dependent conditions.
 * The loop-tail duplication below is the construct accepted by owner ruling 2026-08-06
 * (docs/grind/decisions.md; SOTN evidence docs/grind/sotn-evidence-2026-08-06.md).
 * Every codegen-only local is FAKE-labelled with its measured score
 * (memory/grind/func_80020E74/ptr/r280/).
 */
"""


def fix(t):
    i = t.index(OLD_START)
    j = t.index(OLD_END, i) + len(OLD_END)
    t = t[:i] + NEW + t[j:]
    t2 = inl.substitute_body(t, 'func_80021280', body)
    assert t2 != t
    return t2


p = Path('src/code6cac_tu2.c')
new_wt = fix(p.read_text(encoding='utf-8'))
open(p, 'w', newline='\n', encoding='utf-8').write(new_wt)
idx = subprocess.run(['git', 'show', ':src/code6cac_tu2.c'], capture_output=True, check=True).stdout.decode('utf-8')
out = Path('tmp/func_80020E74/a6/idx_tu2.c')
out.write_bytes(fix(idx).encode('utf-8'))
print(subprocess.run(['git', 'hash-object', '-w', str(out)], capture_output=True, check=True, text=True).stdout.strip())
