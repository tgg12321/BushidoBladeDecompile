"""mkpv2.py LANDED OUT -- one-variable-per-value spelling of the landed (loop-scoped) body.
temp -> ang (turn block) / hit (collision result); work -> amt (turn block) / rest (restitution).
Each new local is declared at the innermost scope enclosing its writes (Ruling 11 (C)(1)):
ang/amt at the top of the turn block, hit at the top of the loop body (where temp was),
rest at the top of the `if (hit != 0)` block. Statements are unchanged; only declarations and
identifiers differ."""
import re
import sys

lines = open(sys.argv[1], encoding='utf-8').read().split('\n')
split = next(k for k, l in enumerate(lines) if 'temp = func_8005344C' in l)
turn = next(k for k, l in enumerate(lines) if '&& obj->unk_00 >= 14) {' in l)
hitblk = next(k for k, l in enumerate(lines) if k > split and l.strip() == 'if (temp != 0) {')
out = []
k = 0
while k < len(lines):
    l = lines[k]
    s = l.strip()
    if s.startswith('/* work holds') or s.startswith('/* temp holds'):
        while not lines[k].strip().endswith('*/'):
            k += 1
        k += 1
        continue
    if s == 's32 work;':
        k += 1
        continue
    if s == 's32 temp;':
        out.append(l.replace('temp', 'hit'))
        k += 1
        continue
    if k < split:
        l = re.sub(r'\btemp\b', 'ang', l)
        l = re.sub(r'\bwork\b', 'amt', l)
    else:
        l = re.sub(r'\btemp\b', 'hit', l)
        l = re.sub(r'\bwork\b', 'rest', l)
    out.append(l)
    ind = ' ' * (len(l) - len(l.lstrip()) + 4)
    if k == turn:
        bi = ' ' * (len(lines[k + 1]) - len(lines[k + 1].lstrip()))
        out += [bi + 's32 ang;', bi + 's32 amt;', '']
    if k == hitblk:
        out += [ind + 's32 rest;', '']
    k += 1
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write('\n'.join(out))
