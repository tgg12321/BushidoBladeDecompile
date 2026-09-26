"""Respellings of func_8003B5A4's two PracticeParams stores (case 3), measured against build/ (in place)."""
import subprocess, sys
R = 'tmp/func_80034708/integ'
base = open(R + '/src/code6cac_c_ab.c').read()
A = '''void func_8003B5A4(void) {
    s32 done;

    func_8005B5AC();
    done = 0;
'''
S1 = '                D_80102778.unk_4[1] = byte0;\n'
S2 = '                D_80102778.unk_6[1] = p[1];\n'
assert A in base and S1 in base and S2 in base
V = {
 # pointer to player 1's lesson character, second store through the member
 'v1': (A.replace('    s32 done;\n', '    s32 done;\n    u8 *chardata;\n').replace('    done = 0;\n', '    done = 0;\n    chardata = &D_80102778.unk_4[1];\n'),
        '                *chardata = byte0;\n', S2),
 # one pointer per written byte
 'v4': (A.replace('    s32 done;\n', '    s32 done;\n    u8 *c4;\n    u8 *c6;\n').replace('    done = 0;\n', '    done = 0;\n    c4 = &D_80102778.unk_4[1];\n    c6 = &D_80102778.unk_6[1];\n'),
        '                *c4 = byte0;\n', '                *c6 = p[1];\n'),
 # struct pointer
 'v3': (A.replace('    s32 done;\n', '    s32 done;\n    PracticeParams *pp;\n').replace('    done = 0;\n', '    done = 0;\n    pp = &D_80102778;\n'),
        '                pp->unk_4[1] = byte0;\n', '                pp->unk_6[1] = p[1];\n'),
}
orig = base
for k in (sys.argv[1:] or V):
    a, s1, s2 = V[k]
    s = orig.replace(A, a).replace(S1, s1).replace(S2, s2)
    open(R + '/src/code6cac_c_ab.c', 'w', newline='\n').write(s)
    r = subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_c_ab'], capture_output=True, text=True)
    d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_c_ab'], capture_output=True, text=True).stdout
    i = d.find('func_8003B5A4')
    print('==', k, [l for l in r.stdout.splitlines() if 'B5A4' in l], '\n', d[i:i+700] if i >= 0 else 'IDENTICAL/reloc-only')
open(R + '/src/code6cac_c_ab.c', 'w', newline='\n').write(orig)
