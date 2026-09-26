"""Registers at temp's six sites + the clear-loop counter, from rtl/<tag>.inst.fn.s (dump.sh).
usage: python3 tmp/func_80055138/r11/site_report.py <tag>..."""
import re, sys

R = 'tmp/func_80055138/r11/rtl'
for tag in sys.argv[1:]:
    L = [l.strip() for l in open('%s/%s.inst.fn.s' % (R, tag))]
    def after(pat, k=1):
        for i, l in enumerate(L):
            if re.search(pat, l):
                return ' / '.join(L[i + 1:i + 1 + k])
        return '?'
    v1 = after(r'^srl\t\$\d+,\$2[01],2$')
    v2 = after(r'^srl\t\$\d+,\$2[01],1$')
    v3 = after(r'^srl\t\$\d+,\$2[01],3$', 2)
    ors = [l for l in L if l.startswith('or\t')]
    v4 = ors[2] if len(ors) > 2 else '?'
    moves = [l for l in L if re.match(r'move\t\$\d+,\$\d+$', l)]
    ctr = next((l for l in L if re.match(r'sltu\t\$2,\$\d+,8$', l)), '?')
    print('%-16s v1:%-20s v2:%-20s v3:%-34s v4:%-16s ctr:%s' % (tag, v1, v2, v3, v4, ctr))
