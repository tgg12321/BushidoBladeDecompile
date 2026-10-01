# typed.py <cand.c> <out.c> : respell the stand-in R8002AB08 view onto PracticeMenuRec members
import re, sys
s = open(sys.argv[1]).read()
s = re.sub(r"typedef struct \{\n(?:.*\n)*?\} R8002AB08;\n", "", s)
mp = {'c':'unk_0C','e':'unk_0E','f26c':'unk_26C','f286':'unk_286','f34a':'unk_34A','f3c':'unk_3C','f8c':'unk_8C',
      'f92':'unk_92','f96':'unk_96','fad':'unk_AD','fae':'unk_AE','frame':'unk_40','move':'unk_6A','pos':'unk_F4',
      'rot':'unk_1C8','seg2':'unk_234','seg':'unk_210','vel':'unk_114','yaw':'unk_1D8','nudge':'unk_134','unk_A1':'unk_A1','unk_A3':'unk_A3'}
s = s.replace("->win[alt + 2]", "->unk_A3[alt]").replace("->win[", "->unk_A1[")
s = re.sub(r"\(\(R8002AB08 \*\)(other|self)\)->(\w+)", lambda m: f"{m[1]}->{mp[m[2]]}", s)
s = re.sub(r"\(u16\)(other|self)->unk_6A", r"\1->unk_6A", s)
s = s.replace("    u8 *other;\n    u8 *self;\n", "    PracticeMenuRec *other;\n    PracticeMenuRec *self;\n")
s = s.replace("""        self = (u8 *)g_practice_menu_table + i * 0x44C;
        other = (u8 *)g_practice_menu_table;
        if (i == 0) {
            other += 0x44C;
        }""", """        self = &g_practice_menu_table[i];
        other = g_practice_menu_table;
        if (i == 0) {
            other++;
        }""")
s = s.replace("func_8002A458(self,", "func_8002A458((u8 *)self,").replace("func_8002CA8C(self,", "func_8002CA8C((u8 *)self,").replace("func_80027AD8(0, self,", "func_80027AD8(0, (u8 *)self,")
s = s.replace("func_800274BC(&other->unk_114[", "func_800274BC(&other->unk_114[")
assert 'R8002AB08' not in s
open(sys.argv[2], 'w', newline='\n').write(s)
