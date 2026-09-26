"""Summarize the sanctioned-family escape dumps: for each ce_* tag, the case-2 level andi, the
mask `or`, the clear-loop counter register (sltiu ...,8), and the Register line of the split
variable's pseudo (from the RTL set pattern)."""
import re, glob, os
R = 'tmp/func_80055138/r11/rtl'
names = sorted(os.path.basename(p)[:-5] for p in glob.glob(R + '/ce_*.lreg'))
for n in names:
    fn = open('%s/%s.fn.s' % (R, n)).read().split('\n')
    andi = next((fn[i + 1].strip() for i, l in enumerate(fn) if re.search(r'\tsrl\t\$\d+,\$2[01],2$', l)), '?')
    orl = [l.strip() for l in fn if l.startswith('\tor\t')]
    ctr = next((l.strip() for l in fn if re.search(r'\tsltu\t\$2,\$\d+,8$|\tsltu\t\$\d+,\$\d+,8$', l) or ',8\n' in l and 'sltu' in l), None)
    if ctr is None:
        ctr = next((l.strip() for l in fn if 'sltu' in l and l.rstrip().endswith(',8')), '?')
    print('%-20s andi: %-22s or#3: %-16s ctr: %s' % (n, andi, orl[2] if len(orl) > 2 else orl, ctr))
