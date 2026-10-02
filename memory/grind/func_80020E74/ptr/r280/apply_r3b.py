"""apply_r3b.py: round-3 edits in func_800224E0 (FAKE label on the signed pointer compare) and
func_8001DB9C (drop the no-op (u16) cast), in the working tree AND the index copy of src/code6cac_tu2.c.
Prints the new index blob hash."""
import subprocess
from pathlib import Path

E = [
    ("    } while ((s32)p < (s32)end);\n",
     "    } while ((s32)p < (s32)end); /* FAKE: signed compare (slt); `p < end` gives sltu, score 1 */\n"),
    ("    D_800A38C4[1] = (u16)0xFFFF;\n", "    D_800A38C4[1] = 0xFFFF;\n"),
]


def fix(t):
    for o, n in E:
        assert t.count(o) == 1, o
        t = t.replace(o, n)
    return t


p = Path('src/code6cac_tu2.c')
new_wt = fix(p.read_text(encoding='utf-8'))
open(p, 'w', newline='\n', encoding='utf-8').write(new_wt)
idx = subprocess.run(['git', 'show', ':src/code6cac_tu2.c'], capture_output=True, check=True).stdout.decode('utf-8')
out = Path('tmp/func_80020E74/a6/idx_tu2.c')
out.write_bytes(fix(idx).encode('utf-8'))
print(subprocess.run(['git', 'hash-object', '-w', str(out)], capture_output=True, check=True, text=True).stdout.strip())
