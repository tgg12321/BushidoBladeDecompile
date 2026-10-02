"""splice.py <tu.c> <candidate.c> : substitute the func_80020E74 body in place (copies only)."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
import importlib
inl = importlib.import_module('engine.inlineasm')
tu, cand = sys.argv[1], sys.argv[2]
b = Path(tu).read_text(encoding='utf-8')
c = Path(cand).read_text(encoding='utf-8')
open(tu, 'w', newline='\n', encoding='utf-8').write(inl.substitute_body(b, 'func_80020E74', c))
