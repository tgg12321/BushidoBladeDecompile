"""Which asm statements of a func_8002D780 body are qualifying gtemacro units? usage: units.py <body.c>"""
import sys

sys.path.insert(0, ".")
from engine import gtemacro  # noqa: E402

text = open(sys.argv[1], encoding="utf-8").read()
spans = gtemacro.unit_spans(text)
print("asm statements:", text.count("__asm__"), "in units:", len(spans))
seen = []
for s, e, name in spans:
    if not seen or seen[-1][0] != name:
        seen.append([name, 0])
    seen[-1][1] += 1
print(seen)
