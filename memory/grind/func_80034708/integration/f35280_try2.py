"""func_80035280 loop-2 respelling: FileTimeRec pointer instead of a byte re-view (in place, pre-split)."""
import subprocess
R = 'tmp/func_80034708/integ'
orig = open(R + '/src/code6cac_b.c').read()
s = orig
pairs = [
    ('    base = (u8 *)D_80106A50.times;\n', '    base = D_80106A50.times;\n'),
    ('        mn = *(s32 *)(base + i * 8 + 4) / 1800;\n', '        mn = base[i].unk_4 / 1800;\n'),
    ('        sc = (*(s32 *)(base + i * 8 + 4) / 30) % 60;\n', '        sc = (base[i].unk_4 / 30) % 60;\n'),
    ('        hs = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;\n', '        hs = (base[i].unk_4 % 30) * 100 / 30;\n'),
    ('        t = base[i * 8];\n', '        t = base[i].unk_0;\n'),
]
for o, n in pairs:
    assert s.count(o) == 1, o
    s = s.replace(o, n)
i = s.index('void func_80035280(void) {')
j = s.index('    u8 *base;\n', i)
s = s[:j] + '    FileTimeRec *base;\n' + s[j + len('    u8 *base;\n'):]
open(R + '/src/code6cac_b.c', 'w', newline='\n').write(s)
subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_b'], capture_output=True, text=True)
d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_b'], capture_output=True, text=True).stdout
k = d.find('func_80035280')
print(d[k:k + 900] if k >= 0 else 'identical')
open(R + '/src/code6cac_b.c', 'w', newline='\n').write(orig)
