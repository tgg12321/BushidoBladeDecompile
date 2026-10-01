O = """                temp = 4;
                if ((u16)rec->unk_6A == 0x11) {
                    temp = 8;
                }"""
for src, dst, var in [('F_A_add.c', 'R2_addsplit_tern.c', 'add'), ('F_A_all_temp.c', 'R2_alltemp_tern.c', 'add')]:
    t = open(src).read()
    o = O.replace('temp', var)
    assert t.count(o) == 1, src
    t = t.replace(o, "                %s = (u16)rec->unk_6A == 0x11 ? 8 : 4;" % var)
    open(dst, 'w', newline='\n').write(t)
pv = open('../../memory/grind/func_80055B60/probes/r11/onevar_PV-91.c').read()
import re
pv = pv[pv.index('void func_80055B60('):]
pv = pv[:pv.index('\n}\n') + 3]
o = O.replace('temp', 'add')
assert pv.count(o) == 1
open('R2_pv_tern.c', 'w', newline='\n').write(pv.replace(o, "                add = (u16)rec->unk_6A == 0x11 ? 8 : 4;"))
