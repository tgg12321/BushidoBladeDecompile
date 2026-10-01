# abl2.py <body.c> <outprefix>: Ruling 11 per-value ablations of the landed body.
# For each value: that value alone gets its own local (innermost scope), every other value
# stays in its shared variable. Also one variant per variable with all its values split.
import sys
body = open(sys.argv[1]).read()
prefix = sys.argv[2]

V = {
 'dist': [("    temp = rec->unk_00->unk_148 - rec->unk_148;\n    if (temp < -1000) {",
           "    dist = rec->unk_00->unk_148 - rec->unk_148;\n    if (dist < -1000) {"),
          ("    } else if (temp > 1000) {", "    } else if (dist > 1000) {"),
          ("    s32 i;\n", "    s32 i;\n    s32 dist;\n")],
 'n': [("                    } else {\n                        temp = rec->unk_414[i][1] - 1;",
        "                    } else {\n                        s32 n;\n\n                        n = rec->unk_414[i][1] - 1;"),
       ("                        if (temp < 0) {\n                            temp = 0;\n                        }\n                        rec->unk_414[i][1] = temp;",
        "                        if (n < 0) {\n                            n = 0;\n                        }\n                        rec->unk_414[i][1] = n;")],
 'add': [("                s32 found;\n\n", "                s32 found;\n                s32 add;\n\n"),
         ("                temp = rec->unk_6A == 0x11 ? 8 : 4;",
          "                add = rec->unk_6A == 0x11 ? 8 : 4;"),
         ("                    rec->unk_414[slot][1] = temp;", "                    rec->unk_414[slot][1] = add;"),
         ("                    temp += rec->unk_414[found][1];\n                    if (temp > 0xFF) {\n                        temp = 0xFF;\n                    }\n                    rec->unk_414[found][1] = temp;",
          "                    add += rec->unk_414[found][1];\n                    if (add > 0xFF) {\n                        add = 0xFF;\n                    }\n                    rec->unk_414[found][1] = add;")],
 'mask': [("                    temp = 0x20;\n                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {\n                        temp = 0x60;\n                    }\n                    rec->unk_430 |= temp;",
           "                    s32 mask;\n\n                    mask = 0x20;\n                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {\n                        mask = 0x60;\n                    }\n                    rec->unk_430 |= mask;")],
 'da': [("                temp = (ratan2(", "                s32 da;\n\n                da = (ratan2("),
        ("                if (temp > 0x800) {\n                    temp -= 0x1000;\n                }\n                if ((temp < 0 ? -temp : temp) < 0x80) {",
         "                if (da > 0x800) {\n                    da -= 0x1000;\n                }\n                if ((da < 0 ? -da : da) < 0x80) {")],
 'ret': [("            temp = func_80055948((u8 *)rec);", "            ret = func_80055948((u8 *)rec);"),
         ("            temp = func_80058580(rec);", "            ret = func_80058580(rec);"),
         ("    } while (temp == -1 && i < 4);\n    if (temp != -1) {\n        pad.held = temp;",
          "    } while (ret == -1 && i < 4);\n    if (ret != -1) {\n        pad.held = ret;"),
         ("    s32 i;\n", "    s32 i;\n    s32 ret;\n")],
 'lim': [("    temp2 = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;", "    lim = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;"),
         ("    if (temp2 < (work >= 0 ? work : -work)) {", "    if (lim < (work >= 0 ? work : -work)) {"),
         ("    s32 i;\n", "    s32 i;\n    s32 lim;\n")],
 'near': [("            } else {\n                rec->unk_428 = (rec->unk_00->unk_6C",
           "            } else {\n                s32 near;\n\n                rec->unk_428 = (rec->unk_00->unk_6C"),
          ("                temp2 = func_80056FE8(rec);\n", "                near = func_80056FE8(rec);\n"),
          ("                temp3 = temp2 + 800;", "                temp3 = near + 800;"),
          ("                    if (temp2 / 2 >= D_800A387C) {", "                    if (near / 2 >= D_800A387C) {"),
          ("                    } else if (temp2 >= D_800A387C) {", "                    } else if (near >= D_800A387C) {"),
          ("                    if (temp2 >= D_800A387C) {", "                    if (near >= D_800A387C) {")],
 'len': [("            Obj80106A78 *obj = &D_80106A78[i];\n\n            temp2 = SquareRoot0(",
          "            Obj80106A78 *obj = &D_80106A78[i];\n            s32 len;\n\n            len = SquareRoot0("),
         ("                        rec->unk_425 = temp2 < 3000 ? 2 : 1;", "                        rec->unk_425 = len < 3000 ? 2 : 1;")],
 'least': [("                s32 found;\n\n                temp3 = 0x100;", "                s32 found;\n                s32 least;\n\n                least = 0x100;"),
           ("                        if (rec->unk_414[i][1] < temp3) {\n                            slot = i;\n                            temp3 = rec->unk_414[i][1];",
            "                        if (rec->unk_414[i][1] < least) {\n                            slot = i;\n                            least = rec->unk_414[i][1];")],
 'far': [("            } else {\n                rec->unk_428 = (rec->unk_00->unk_6C",
          "            } else {\n                s32 far;\n\n                rec->unk_428 = (rec->unk_00->unk_6C"),
         ("FARSET", "FARSET"),
         ("                    } else if (temp3 >= D_800A387C) {", "                    } else if (far >= D_800A387C) {"),
         ("                rec->unk_42E = temp3;", "                rec->unk_42E = far;")],
 'sign': [("        work >>= 31;\n        if (work != (rec->unk_3F0 >> 15)) {\n            rec->unk_3F0 = 0;\n        }\n        rec->unk_3F0 += work ? -1 : 1;",
           "        s32 sign;\n\n        sign = work >> 31;\n        if (sign != (rec->unk_3F0 >> 15)) {\n            rec->unk_3F0 = 0;\n        }\n        rec->unk_3F0 += sign ? -1 : 1;")],
}
GROUPS = {'temp': ['dist', 'n', 'add', 'mask', 'da', 'ret'], 'temp2': ['lim', 'near', 'len'],
          'temp3': ['least', 'far'], 'work': ['sign']}

ITAG = "    s32 i; /* SOTN: src/dra/5D5BC.c:173 @aa53500 */\n"

def apply(names):
    t = apply0(names, body.replace(ITAG, "    s32 i;\n"))
    return t.replace("    s32 i;\n", ITAG, 1) if ITAG in body else t

def apply0(names, t):
    for nm in names:
        for o, n in V[nm]:
            if o == 'FARSET':
                t = t.replace('                temp3 = temp2 + 800;', '                far = temp2 + 800;')
                t = t.replace('                temp3 = near + 800;', '                far = near + 800;')
                continue
            assert t.count(o) >= 1, (nm, o)
            t = t.replace(o, n, 1)
    return t

made = []
for nm in V:
    open('%s_%s.c' % (prefix, nm), 'w', newline='\n').write(apply([nm]))
    made.append('%s_%s' % (prefix.split('/')[-1], nm))
for g, names in GROUPS.items():
    if len(names) > 1:
        open('%s_all_%s.c' % (prefix, g), 'w', newline='\n').write(apply(names))
        made.append('%s_all_%s' % (prefix.split('/')[-1], g))
print(' '.join(made))
