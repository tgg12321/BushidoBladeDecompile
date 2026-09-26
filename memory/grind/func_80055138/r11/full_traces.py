"""Replace the abridged FINDREG excerpts in ruling11.md with the full traces (incl. own_copy_prefs /
own_full_prefs) from the landed-tree dumps (tcand / tpv / tctr)."""
import re
p = 'memory/grind/func_80055138/ruling11.md'
s = open(p, encoding='utf-8').read()
R = 'tmp/func_80055138/r11/rtl/'
src = {'99': 'tcand', '89': 'tcand', '188': 'tpv', '106': 'tpv', '90': 'tctr'}


def full(pseudo):
    return open(R + '%s.findreg.%s' % (src[pseudo], pseudo)).read().rstrip('\n')


def repl(m):
    block = m.group(0)
    pseudos = re.findall(r'FINDREGDBG func=func_80055138 pseudo=(\d+) ', block)
    return '\n'.join(full(x) for x in pseudos) + '\n'


s2 = re.sub(r'(?:FINDREGDBG[^\n]*\n)+', repl, s)
open(p, 'w', encoding='utf-8', newline='\n').write(s2)
print(s2.count('own_copy_prefs'))
