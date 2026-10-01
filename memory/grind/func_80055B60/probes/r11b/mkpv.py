# mkpv.py: one-variable-per-value spelling of g2.c (Ruling 11 (C)(1)) -> tmp/b60/pv.c
# Each value of temp / temp2 / temp3 / work gets its own local at its innermost scope;
# only declarations and identifiers change.
import re, sys
src = open(sys.argv[1] if len(sys.argv) > 1 else 'tmp/b60/g2.c').read()
out = sys.argv[2] if len(sys.argv) > 2 else 'tmp/b60/pv.c'
v = src

def R(o, n, cnt=1):
    global v
    assert v.count(o) == cnt, (o, v.count(o))
    v = v.replace(o, n)

# function-scope declarations: drop the reused ones, add dist/ret/lim/diff
v = re.sub(r"    /\* work holds.*?    s32 temp3;\n",
           "    s32 diff;\n    s32 lim;\n    s32 dist;\n    s32 ret;\n", v, count=1, flags=re.S)
assert "    s32 diff;\n" in v
# work: diff, then sign
R("    temp2 = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;\n    work = D_800A387C - rec->unk_43E;\n    if (temp2 < (work >= 0 ? work : -work)) {\n        work >>= 31;\n        if (work != (rec->unk_3F0 >> 15)) {\n            rec->unk_3F0 = 0;\n        }\n        rec->unk_3F0 += work ? -1 : 1;\n",
  "    lim = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;\n    diff = D_800A387C - rec->unk_43E;\n    if (lim < (diff >= 0 ? diff : -diff)) {\n        s32 sign;\n\n        sign = diff >> 31;\n        if (sign != (rec->unk_3F0 >> 15)) {\n            rec->unk_3F0 = 0;\n        }\n        rec->unk_3F0 += sign ? -1 : 1;\n")
R("    temp = rec->unk_00->unk_148 - rec->unk_148;\n    if (temp < -1000) {", "    dist = rec->unk_00->unk_148 - rec->unk_148;\n    if (dist < -1000) {")
R("    } else if (temp > 1000) {", "    } else if (dist > 1000) {")
# slot block: least, n, add
R("                s32 slot;\n                s32 found;\n\n                temp3 = 0x100;",
  "                s32 slot;\n                s32 found;\n                s32 least;\n                s32 add;\n\n                least = 0x100;")
R("                    } else {\n                        temp = rec->unk_414[i][1] - 1;\n                        if (rec->unk_414[i][1] < temp3) {\n                            slot = i;\n                            temp3 = rec->unk_414[i][1];\n                        }\n                        if (temp < 0) {\n                            temp = 0;\n                        }\n                        rec->unk_414[i][1] = temp;",
  "                    } else {\n                        s32 n;\n\n                        n = rec->unk_414[i][1] - 1;\n                        if (rec->unk_414[i][1] < least) {\n                            slot = i;\n                            least = rec->unk_414[i][1];\n                        }\n                        if (n < 0) {\n                            n = 0;\n                        }\n                        rec->unk_414[i][1] = n;")
R("                temp = rec->unk_6A == 0x11 ? 8 : 4;",
  "                add = rec->unk_6A == 0x11 ? 8 : 4;")
R("                    rec->unk_414[slot][1] = temp;", "                    rec->unk_414[slot][1] = add;")
R("                    temp += rec->unk_414[found][1];\n                    if (temp > 0xFF) {\n                        temp = 0xFF;\n                    }\n                    rec->unk_414[found][1] = temp;",
  "                    add += rec->unk_414[found][1];\n                    if (add > 0xFF) {\n                        add = 0xFF;\n                    }\n                    rec->unk_414[found][1] = add;")
# near / far block
R("            } else {\n                rec->unk_428 = (rec->unk_00->unk_6C",
  "            } else {\n                s32 near;\n                s32 far;\n\n                rec->unk_428 = (rec->unk_00->unk_6C")
R("                temp2 = func_80056FE8(rec);\n                temp3 = temp2 + 800;",
  "                near = func_80056FE8(rec);\n                far = near + 800;")
R("                    if (temp2 / 2 >= D_800A387C) {", "                    if (near / 2 >= D_800A387C) {")
R("                    } else if (temp2 >= D_800A387C) {", "                    } else if (near >= D_800A387C) {")
R("                    if (temp2 >= D_800A387C) {", "                    if (near >= D_800A387C) {")
R("                    } else if (temp3 >= D_800A387C) {", "                    } else if (far >= D_800A387C) {")
R("                rec->unk_42E = temp3;", "                rec->unk_42E = far;")
# mask
R("                    rec->unk_414[i][1] >= rec->unk_424) {\n                    temp = 0x20;\n                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {\n                        temp = 0x60;\n                    }\n                    rec->unk_430 |= temp;",
  "                    rec->unk_414[i][1] >= rec->unk_424) {\n                    s32 mask;\n\n                    mask = 0x20;\n                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {\n                        mask = 0x60;\n                    }\n                    rec->unk_430 |= mask;")
# len / da
R("            Obj80106A78 *obj = &D_80106A78[i];\n\n            temp2 = SquareRoot0(",
  "            Obj80106A78 *obj = &D_80106A78[i];\n            s32 len;\n\n            len = SquareRoot0(")
R("            if (obj->unk_02 != -1 && obj->unk_04 != 0 && obj->unk_06 != rec->unk_04) {\n                temp = (ratan2(",
  "            if (obj->unk_02 != -1 && obj->unk_04 != 0 && obj->unk_06 != rec->unk_04) {\n                s32 da;\n\n                da = (ratan2(")
R("                if (temp > 0x800) {\n                    temp -= 0x1000;\n                }\n                if ((temp < 0 ? -temp : temp) < 0x80) {",
  "                if (da > 0x800) {\n                    da -= 0x1000;\n                }\n                if ((da < 0 ? -da : da) < 0x80) {")
R("                        rec->unk_425 = temp2 < 3000 ? 2 : 1;", "                        rec->unk_425 = len < 3000 ? 2 : 1;")
# ret
R("            temp = func_80055948((u8 *)rec);", "            ret = func_80055948((u8 *)rec);")
R("            temp = func_80058580(rec);", "            ret = func_80058580(rec);")
R("    } while (temp == -1 && i < 4);\n    if (temp != -1) {\n        pad.held = temp;",
  "    } while (ret == -1 && i < 4);\n    if (ret != -1) {\n        pad.held = ret;")
for name in ('temp', 'temp2', 'temp3', 'work'):
    assert not re.search(r'\b%s\b' % name, v), name
open(out, 'w', newline='\n').write(v)
print('ok')
