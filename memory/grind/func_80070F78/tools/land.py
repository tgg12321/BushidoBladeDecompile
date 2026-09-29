"""Splice a func_80070F78 candidate into src/text1b.c with the landing-time edits (sc_reps.apply).
usage: python3 tmp/func_80070F78/land.py cand.c"""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
import importlib.util
s = importlib.util.spec_from_file_location('sc_reps', 'tmp/func_80070F78/sc_reps.py')
m = importlib.util.module_from_spec(s); s.loader.exec_module(m)
p = Path('src/text1b.c')
t = p.read_text(encoding='utf-8')
t = inlineasm.substitute_body(t, 'func_80070F78', Path(sys.argv[1]).read_text(encoding='utf-8'))
t = m.apply(t)
p.write_bytes(t.encode('utf-8'))
print('spliced')
