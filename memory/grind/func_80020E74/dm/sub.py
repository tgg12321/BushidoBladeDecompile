"""sub.py <src stem> <func> <candidate> <out tu path>: substitute a body into a copy."""
import sys, importlib
sys.path.insert(0, '.')
from pathlib import Path
inl = importlib.import_module('engine.inlineasm')
stem, fn, cand, out = sys.argv[1:5]
b = Path(f'src/{stem}.c').read_text(encoding='utf-8')
Path(out).parent.mkdir(parents=True, exist_ok=True)
open(out, 'w', newline='\n', encoding='utf-8').write(inl.substitute_body(b, fn, Path(cand).read_text(encoding='utf-8')))
