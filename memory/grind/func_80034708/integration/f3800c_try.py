"""func_8003800C: restore the record by struct assignment instead of a CopyBlock view of it."""
import subprocess
R = 'tmp/func_80034708/integ'
p = R + '/src/code6cac_c_mid.c'
orig = open(p).read()
old = '''        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            do {
                *dst = *sp2;
                sp2++;
                dst++;
            } while (sp2 != end);
            *(s32 *)dst = *(s32 *)sp2;
        }
'''
new = '''        if (!(*(src + 0x23) & 0x80)) {
            D_80106A50 = *(FileRecord *)src;
        }
'''
assert old in orig
open(p, 'w', newline='\n').write(orig.replace(old, new))
subprocess.run(['python3', 'tmp/func_80034708/integ.py', 'code6cac_c_mid'], capture_output=True, text=True)
d = subprocess.run(['bash', 'tmp/func_80034708/showdiff.sh', 'code6cac_c_mid'], capture_output=True, text=True).stdout
i = d.find('===== func_8003800C')
j = d.find('\n=====', i + 1)
print(d[i:j] if i >= 0 else 'IDENTICAL')
open(p, 'w', newline='\n').write(orig)
