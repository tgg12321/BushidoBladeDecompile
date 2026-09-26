"""b3_body.c: unk_6[x] / unk_8[x] -> unk_4[2 + x] / unk_4[4 + x] (flat pairs array)."""
import re
p = 'tmp/func_80034708/b3_body.c'
s = open(p).read()
def f(m):
    k = {'6': 2, '8': 4}[m.group(1)]
    e = m.group(2)
    return f'D_80102778.unk_4[{k + int(e)}]' if e.isdigit() else f'D_80102778.unk_4[{k} + {e}]'
s = re.sub(r'D_80102778\.unk_([68])\[(\w+)\]', f, s)
assert 'unk_6' not in s and 'unk_8' not in s
open(p, 'w', newline='\n').write(s)
print(sorted(set(re.findall(r'D_80102778\.unk_4\[[^]]*\]', s))))
