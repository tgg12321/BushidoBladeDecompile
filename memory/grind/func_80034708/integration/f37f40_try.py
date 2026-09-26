"""func_80037F40 member-respecting copy spellings (in place, unsplit tree); objdiff vs build/.
usage: f37f40_try.py [variant ...]; prints per variant the instruction count and differing lines."""
import subprocess, sys
R = 'tmp/func_80034708/integ'
p = R + '/src/code6cac_c_mid.c'
orig = open(p).read()
i0 = orig.index('void func_80037F40(u8 *a0) {')
i1 = orig.index('\n}\n', i0) + 3

HEAD = '''void func_80037F40(u8 *a0) {
    s32 checksum;
    u8 *p;
    s32 i;

    checksum = 0;
    p = (u8 *)&D_80106A50;
    i = 0;
    do {
        checksum += *p++;
        i++;
    } while ((u32)i < 0x24);

'''
TAIL_LOOP = '''            *(s32 *)(base2 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            base2 += 4;
            i++;
'''
def body(pre, copy, step='            offset += 0x24;\n', post='        *(s32 *)(base + 0xFC) = 0;\n'):
    return HEAD + '    {\n' + pre + '        do {\n' + copy + TAIL_LOOP + step + '        } while (i < 3);\n' + post + '    }\n}\n'

V = {
 # struct assignment, current preheader order
 's1': body('''        u8 *base = a0;
        u8 *base2 = base;
        s32 offset = 0;
        i = 0;
''', '''            *(FileRecord *)(offset + (s32)base) = D_80106A50;
'''),
 # i = 0 first (as the original's statement order), then the pointer inits
 's2': body('''        u8 *base;
        u8 *base2;
        s32 offset;
        i = 0;
        base = a0;
        base2 = base;
        offset = 0;
''', '''            *(FileRecord *)(offset + (s32)base) = D_80106A50;
'''),
 # named source-record pointer computed first
 's3': body('''        FileRecord *rec;
        u8 *base;
        u8 *base2;
        s32 offset;
        i = 0;
        rec = &D_80106A50;
        base = a0;
        base2 = base;
        offset = 0;
''', '''            *(FileRecord *)(offset + (s32)base) = *rec;
'''),
 # destination as a FileRecord array (the slot stride 0x24 == sizeof(FileRecord))
 's4': body('''        FileRecord *slot;
        u8 *base;
        u8 *base2;
        i = 0;
        base = a0;
        base2 = base;
        slot = (FileRecord *)base;
''', '''            *slot = D_80106A50;
''', step='            slot++;\n'),
 's5': body('''        u8 *base;
        u8 *base2;
        i = 0;
        base = a0;
        base2 = base;
''', '''            ((FileRecord *)base)[i] = D_80106A50;
''', step=''),
 # member-wise: header word, the three time records, then colour/flags
 's6': body('''        u8 *base = a0;
        u8 *base2 = base;
        s32 offset = 0;
        i = 0;
''', '''            {
                FileRecord *d = (FileRecord *)(offset + (s32)base);
                d->unk_00 = D_80106A50.unk_00;
                d->unk_04 = D_80106A50.unk_04;
                d->times[0] = D_80106A50.times[0];
                d->times[1] = D_80106A50.times[1];
                d->times[2] = D_80106A50.times[2];
            }
'''),
 # struct assign with the end-of-times member address as its own first statement (read by nothing)
 's7': body('''        FileTimeRec *tend;
        u8 *base;
        u8 *base2;
        s32 offset;
        i = 0;
        tend = &D_80106A50.times[3];
        base = a0;
        base2 = base;
        offset = 0;
''', '''            *(FileRecord *)(offset + (s32)base) = D_80106A50;
'''),
}

V['s8'] = HEAD + '''    i = 0;
    do {
        ((FileRecord *)a0)[i] = D_80106A50;
        *(s32 *)(a0 + i * 4 + 0x6C) = checksum;
        {
            s32 j = 0;
            s16 *hp = (s16 *)a0;
            u8 *bp = a0;
            do {
                *(s32 *)(bp + 0x78) = 0;
                *(s16 *)((u8 *)hp + 0xD0) = 0;
                hp++;
                j++;
                bp += 4;
            } while (j < 0x16);
        }
        i++;
    } while (i < 3);
    *(s32 *)(a0 + 0xFC) = 0;
}
'''
V['s9'] = HEAD + '''    {
        u8 *base = a0;
        i = 0;
        do {
            ((FileRecord *)base)[i] = D_80106A50;
            *(s32 *)(base + i * 4 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            i++;
        } while (i < 3);
        *(s32 *)(base + 0xFC) = 0;
    }
}
'''
V['s10'] = HEAD + '''    {
        u8 *base2 = a0;
        i = 0;
        do {
            ((FileRecord *)a0)[i] = D_80106A50;
            *(s32 *)(base2 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)a0;
                u8 *bp = a0;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            base2 += 4;
            i++;
        } while (i < 3);
        *(s32 *)(a0 + 0xFC) = 0;
    }
}
'''

def run(k):
    s = orig[:i0] + V[k] + orig[i1:]
    open(p, 'w', newline='\n').write(s)
    r = subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_c_mid'], capture_output=True, text=True)
    if 'BUILD FAIL' in r.stdout:
        return 'BUILD FAIL ' + r.stdout[-400:]
    sc = subprocess.run(['python3', 'tmp/func_80034708/scorefn.py', 'tmp/func_80034708/integ/build/code6cac_c_mid.o', 'build/src/code6cac_c_mid.o', 'func_80037F40'], capture_output=True, text=True).stdout.strip()
    d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_c_mid'], capture_output=True, text=True).stdout
    i = d.find('===== func_80037F40')
    if i < 0:
        return sc + ' IDENTICAL'
    j = d.find('\n=====', i + 1)
    blk = d[i:j if j > 0 else None].strip().split('\n')
    return sc + ' ' + blk[0] + '  diff-lines=%d\n' % (len(blk) - 1) + '\n'.join(blk[1:24])

try:
    for k in (sys.argv[1:] or V):
        print('==', k, run(k))
finally:
    open(p, 'w', newline='\n').write(orig)
