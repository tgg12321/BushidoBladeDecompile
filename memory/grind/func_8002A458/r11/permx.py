"""Extract the lowest permuter finds of tmp/perm_a458_onevar as sandbox candidates
(prelude of r11v/onevar_full.c + the find's func_8002A458 definition)."""
import os
import sys

D = 'tmp/perm_a458_onevar'
O = 'tmp/func_8002A458/perm'
N = int(sys.argv[1]) if len(sys.argv) > 1 else 24
os.makedirs(O, exist_ok=True)
base = open('tmp/func_8002A458/r11v/onevar_full.c', encoding='utf-8').read()
prelude = base[:base.index('void func_8002A458(')]
dirs = sorted([d for d in os.listdir(D) if d.startswith('output-')], key=lambda d: (int(d.split('-')[1]), d))
names = []
for d in dirs[:N]:
    s = open(os.path.join(D, d, 'source.c'), encoding='utf-8').read()
    i = s.index('void func_8002A458(')
    j = s.index('\n}', i) + 2
    body = s[i:j]
    assert body.count('{') == body.count('}'), d
    p = O + '/' + d + '.c'
    open(p, 'w', newline='\n').write(prelude + body + '\n')
    names.append(p)
open(O + '/list.txt', 'w').write(','.join(names))
print(len(names), dirs[N - 1])
