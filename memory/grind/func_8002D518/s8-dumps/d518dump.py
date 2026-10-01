"""Regenerate func_8002D518's s8 cse evidence on the current tree.
A = src as staged (two `ud = disc;` writes); B = the banked s8 control (single write,
memory/grind/func_8002D518/s8-dumps/control-single-ud.c, its D_8008D118 table spelled
g_sqrt_table_u8[] for today's declaration). Writes trees under tmp/laneH/d518_{A,B}/."""
import re
import shutil
import sys
from pathlib import Path
sys.path.insert(0, 'tmp/laneH')
from edits import _span

ctl = open('memory/grind/func_8002D518/s8-dumps/control-single-ud.c', encoding='utf-8').read()
ctl = re.sub(r'\(&D_8008D118\)\[', 'g_sqrt_table_u8[', ctl)
for tag in ('A', 'B'):
    d = Path(f'tmp/laneH/d518_{tag}')
    if d.exists():
        shutil.rmtree(d)
    (d / 'src').mkdir(parents=True)
    t = open('src/code6cac_b_tu2.c', encoding='utf-8', newline='').read()
    if tag == 'B':
        a, b = _span(t, 'func_8002D518')
        t = t[:a] + ctl.rstrip('\n') + t[b:]
    open(d / 'src' / 'code6cac_b_tu2.c', 'w', encoding='utf-8', newline='').write(t)
print('trees ready')
