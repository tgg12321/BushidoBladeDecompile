"""Build two dump trees for code6cac_b_tu2.c:
  A = the landed form (src as is): func_8002BC68 record pointers, func_80032314 `v1_v`;
  B = the alternatives: func_8002BC68 handle-free (s4_direct_all), func_80032314 direct
      `ent = &g_practice_menu_table[*(u8 *)(a3 + 1) == 0];`."""
import re
import shutil
import sys
from pathlib import Path
sys.path.insert(0, 'tmp/laneH')
from edits import _span

for tag in ('A', 'B'):
    d = Path(f'tmp/laneH/dtree_{tag}')
    if d.exists():
        shutil.rmtree(d)
    shutil.copytree('include', d / 'include')
    (d / 'src').mkdir()
    t = open('src/code6cac_b_tu2.c', encoding='utf-8', newline='').read()
    if tag == 'B':
        # BC68 handle-free: regenerate from the landed body (reads through the handles -> direct)
        a, b = _span(t, 'func_8002BC68')
        body = t[a:b]
        g = 'g_practice_menu_table'
        body = body.replace('    PracticeMenuRec *t2_base;\n    PracticeMenuRec *t3_base;\n', '')
        body = body.replace('    t2_base = g_practice_menu_table;\n    t3_base = t2_base + 1;\n', '')
        body = body.replace('t2_base->', f'{g}[0].').replace('t3_base->', f'{g}[1].')
        assert 't2_base' not in body and 't3_base' not in body
        t = t[:a] + body + t[b:]
        a, b = _span(t, 'func_80032314')
        body = t[a:b]
        body2 = re.sub(r'    \{\n        /\* FAKE: named intermediate.*?\*/\n        s32 v1_v = \(\*\(u8 \*\)\(a3 \+ 1\) == 0\);\n'
                       r'        ent = &g_practice_menu_table\[v1_v\];\n    \}\n',
                       '    ent = &g_practice_menu_table[*(u8 *)(a3 + 1) == 0];\n', body, flags=re.S)
        assert body2 != body
        t = t[:a] + body2 + t[b:]
    open(d / 'src' / 'code6cac_b_tu2.c', 'w', encoding='utf-8', newline='').write(t)
    print('tree', tag)
