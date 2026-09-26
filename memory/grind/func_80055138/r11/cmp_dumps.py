"""Compare the allocation records of two dumps (Register lines, dispositions, ALLOCDBG,
function asm).  usage: python3 tmp/func_80055138/r11/cmp_dumps.py <tagA> <tagB>"""
import re, sys
R = 'tmp/func_80055138/r11/rtl/'
a, b = sys.argv[1], sys.argv[2]


def recs(t):
    lreg = open(R + t + '.lreg').read()
    greg = open(R + t + '.greg').read()
    reg = re.findall(r'^Register \d+ used[^\n]*', lreg, re.M)
    disp = re.search(r';; Register dispositions:\n(.*?)\n\n', greg, re.S).group(1)
    alloc = open(R + t + '.alloc').read()
    asm = open(R + t + '.inst.fn.s').read()
    return dict(register_lines=reg, dispositions=disp, allocdbg=alloc, asm=asm)


ra, rb = recs(a), recs(b)
for k in ra:
    print('%-15s %s' % (k, 'IDENTICAL' if ra[k] == rb[k] else 'DIFFERENT'))
