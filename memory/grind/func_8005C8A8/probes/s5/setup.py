# Build tmp/c8a8A/inc/include/game.h = include/game.h + the game.h hunk of fix1-merges.patch (pure text apply).
import re
src = open('include/game.h', encoding='utf-8').read()
p = open('memory/grind/func_8005C8A8/fix1-merges.patch', encoding='utf-8').read()
gh = p.split('diff --git a/undefined_syms_auto.txt')[0]
hunks = re.split(r'(?m)^@@ .*\n', gh)[1:]
for h in hunks:
    old, new = [], []
    for line in h.splitlines():
        if line.startswith('\\'):
            continue
        tag, body = line[:1], line[1:]
        if tag in (' ', '-'):
            old.append(body)
        if tag in (' ', '+'):
            new.append(body)
    o = '\n'.join(old) + '\n'
    assert src.count(o) == 1, 'hunk context not unique'
    src = src.replace(o, '\n'.join(new) + '\n')
import os
os.makedirs('tmp/c8a8A/inc/include', exist_ok=True)
open('tmp/c8a8A/inc/include/game.h', 'w', encoding='utf-8', newline='\n').write(src)
print('ok')
