"""Write verbatim excerpts of each dumped body into memory/grind/func_80027AD8/q19/rtl/<tag>.txt."""
import os, re, glob
R = os.path.dirname(os.path.abspath(__file__)) + '/rtl/'
OUT = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../memory/grind/func_80027AD8/q19/rtl') + '/'
os.makedirs(OUT, exist_ok=True)
tags = sorted(os.path.basename(p)[:-6] for p in glob.glob(R + '*.alloc'))
for t in tags:
    o = []
    def rd(ext):
        try:
            return open(R + t + '.' + ext).read()
        except FileNotFoundError:
            return ''
    o.append(f'# {t}: verbatim excerpts (tmp/func_80027AD8/r11/dump.sh {t} r11/v/{t}.c)\n')
    o.append('## .flow Register lines (pseudos 72-99)')
    o += [l for l in rd('flow').splitlines() if re.match(r'Register (7[2-9]|8\d|9\d) ', l)]
    o.append('\n## .sched life updates (sched1 recomputation of reg_live_length)')
    o += [l for l in rd('sched').splitlines() if re.match(r';; register (7[2-9]|8\d|9\d) life', l)]
    o.append('\n## .lreg Register lines (pseudos 72-99)')
    o += [l for l in rd('lreg').splitlines() if re.match(r'Register (7[2-9]|8\d|9\d) ', l)]
    o.append('\n## .lreg entry insns (parameter load / copies)')
    lreg = rd('lreg')
    for m in re.finditer(r'\(insn \d+ \d+ \d+ \(set \(reg/v:SI (7[7-9]|8[01])\)\n(?:.*\n){1,4}', lreg):
        o.append(m.group(0).rstrip())
        if len(o) > 400:
            break
    o.append('\n## .greg dispositions')
    g = rd('greg')
    m = re.search(r';; Register dispositions:\n(.*?)\n\n', g, re.S)
    o.append(m.group(1) if m else 'NOTFOUND')
    o.append('\n## ALLOCDBG (global.c allocation order)')
    o.append(rd('alloc').rstrip())
    for fr in sorted(glob.glob(R + t + '.findreg.*')):
        o.append(f'\n## FINDREG {os.path.basename(fr)}')
        o.append(open(fr).read().rstrip())
    o.append('\n## local-alloc quantities (SUGGDBG-QTY) whose reg1 is a long-lived pseudo 72-99')
    q = [l for l in rd('sugg').splitlines() if re.search(r'reg1=(7[2-9]|8\d|9\d) ', l)]
    o += q if q else ['(none: every pseudo 72-99 is global)']
    o.append('\n## function asm (build cc1; identity-checked against the instrumented cc1)')
    o.append(rd('fn.s'))
    open(OUT + t + '.txt', 'w', newline='\n').write('\n'.join(o) + '\n')
    print(t)
