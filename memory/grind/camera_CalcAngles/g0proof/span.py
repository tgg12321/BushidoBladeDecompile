"""span.py: for each text1b static, the lines (and enclosing function) that use it - does the TU's static
use span the canonical-asm block (src/text1b.c lines ~2935-3061)?"""
import re
src = open("src/text1b.c", encoding="utf-8").read().split("\n")
statics = []
for i, l in enumerate(src[:200]):
    m = re.match(r"static [\w ]+?\*?\s*(\w+)(\[\d*\])?;", l)
    if m:
        statics.append(m.group(1))
func = None
uses = {s: [] for s in statics}
for i, l in enumerate(src):
    m = re.match(r"^[\w][\w \*]*\b(\w+)\(.*\)\s*\{\s*$", l)
    if m:
        func = (i + 1, m.group(1))
    if i < 200:
        continue
    for s in statics:
        if re.search(r"\b%s\b" % s, l):
            uses[s].append((i + 1, func[1] if func else None))
for s in statics:
    fs = sorted({(f[0], f[1]) for f in uses[s]}) if uses[s] else []
    print(s, [f"{ln}:{fn}" for ln, fn in fs][:8])
