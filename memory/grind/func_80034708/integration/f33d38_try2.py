"""func_80033D38: vary only the search-loop address form (record-based, member-named)."""
import subprocess, sys
R = 'tmp/func_80034708/integ'
orig = open(R + '/src/code6cac_b.c').read()
i0 = orig.index('void func_80033D38(void) {')
i1 = orig.index('\n}\n', i0) + 3
T = '''void func_80033D38(void) {
    FileRecord *rec = &D_80106A50;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        FileTimeRec *p;
        j = n - 1;
        SEARCH
        if (p->unk_4 < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        FileTimeRec *ins;
        for (k = 2; k > n; k--) {
            rec->times[k] = rec->times[k - 1];
        }
        ins = &rec->times[n];
        ins->unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        ins->unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        ins->unk_4 = D_800A3858;
    }
}
'''
V = {
 'e': 'p = &rec->times[n - 1];',
 'f': 'p = rec->times;\n        p += j;',
 'g': 'p = &rec->times[0] + j;',
 'h': 'p = rec->times - 1 + n;',
 'i': 'p = &rec->times[j];\n        (void)0;',
}
for k in (sys.argv[1:] or V):
    s = orig[:i0] + T.replace('SEARCH', V[k]) + orig[i1:]
    open(R + '/src/code6cac_b.c', 'w', newline='\n').write(s)
    subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_b'], capture_output=True, text=True)
    d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_b'], capture_output=True, text=True).stdout
    i = d.find('===== func_80033D38')
    j = d.find('\n=====', i + 1)
    print('==', k, d[i:j].strip() if i >= 0 else 'IDENTICAL')
open(R + '/src/code6cac_b.c', 'w', newline='\n').write(orig)
