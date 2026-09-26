"""func_80035280 member respellings on the in-place (unsplit) converted code6cac_b.c; objdiff vs build/."""
import subprocess, sys, re
R = 'tmp/func_80034708/integ'
orig = open(R + '/src/code6cac_b.c').read()
HEAD_OLD = '''    p = func_80077D00();
    i = 0;
    f = &D_80106A50.flags;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
'''
assert HEAD_OLD in orig
V = {
 'a': '''    p = func_80077D00();
    i = 0;
    f = &D_80106A50.flags;
    src = D_80106A50.color;
    flags = p[8];
    flags0 = (flags & ~1) | (*f & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (*f & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (*f & 4);
    p[8] = flags2;
''',
 'b': '''    p = func_80077D00();
    i = 0;
    flags = p[8];
    flags0 = (flags & ~1) | (D_80106A50.flags & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (D_80106A50.flags & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (D_80106A50.flags & 4);
    p[8] = flags2;
    src = D_80106A50.color;
''',
 'c': '''    p = func_80077D00();
    i = 0;
    src = D_80106A50.color;
    flags = p[8];
    flags0 = (flags & ~1) | (D_80106A50.flags & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (D_80106A50.flags & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (D_80106A50.flags & 4);
    p[8] = flags2;
''',
}
for k in (sys.argv[1:] or V):
    s = orig.replace(HEAD_OLD, V[k])
    if k in ('b', 'c'):
        s = s.replace('    u8 *f;\n    u8 *src;\n', '    u8 *src;\n', 1)
    open(R + '/src/code6cac_b.c', 'w', newline='\n').write(s)
    subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_b'], capture_output=True, text=True)
    d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_b'], capture_output=True, text=True).stdout
    i = d.find('func_80035280')
    print('==', k, d[i:i + 900] if i >= 0 else 'identical')
open(R + '/src/code6cac_b.c', 'w', newline='\n').write(orig)
