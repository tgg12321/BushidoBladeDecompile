"""usage: splice.py base.c cand.c out.c"""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
t = inlineasm.substitute_body(Path(sys.argv[1]).read_text(), 'func_80036140', Path(sys.argv[2]).read_text())
open(sys.argv[3], 'w', newline='\n').write(t)
