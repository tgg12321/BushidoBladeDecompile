"""Print every func_80055138 insn mentioning the given pseudos, per pass.
usage: python3 tmp/func_80055138/r11/passtrack.py <tag> <pass,pass,...> <pseudo>..."""
import re, sys

tag, passes, regs = sys.argv[1], sys.argv[2].split(','), sys.argv[3:]
pat = re.compile(r'reg(?:/[a-z]+)?:\w+ (%s)\)' % '|'.join(regs))
for p in passes:
    txt = open('tmp/rtl/r55138_%s/in.i.%s' % (tag, p)).read()
    m = re.search(r"\n;; Function func_80055138\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    body = m.group(1)
    insns = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', body)
    hits = [' '.join(i.split()) for i in insns if pat.search(' '.join(i.split()))]
    print('== %s.%s: %d insns' % (tag, p, len(hits)))
    for h in hits:
        print(h[:240])
