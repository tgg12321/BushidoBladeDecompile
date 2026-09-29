import sys
sys.path.insert(0, ".")
from engine import gtemacro, inlineasm
t = open(sys.argv[1]).read()
spans = gtemacro.unit_spans(t)
from collections import Counter
print("statements in units:", len(spans), Counter(m for _s, _e, m in spans))
print("asm statements total:", t.count("__asm__"))
out, n = inlineasm.strip_cheat_asm_file(t, keep_gte_macro_units=True)
print("keep-mode stripped:", n, "| default-mode stripped:", inlineasm.strip_cheat_asm_file(t)[1])
