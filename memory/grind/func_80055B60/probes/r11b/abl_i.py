# abl_i.py <body.c> <outprefix>: Q51 per-loop counter ablations — one loop at a time gets its own counter
# (declared in the innermost scope enclosing that loop); the other loops keep the shared i.
import sys
body = open(sys.argv[1]).read()
prefix = sys.argv[2]

def one(t, o, n):
    assert t.count(o) == 1, (o, t.count(o))
    return t.replace(o, n)

V = {
 'slot': [("                s32 found;\n\n                temp3 = 0x100;\n                i = 0;\n",
           "                s32 found;\n                s32 k;\n\n                temp3 = 0x100;\n                k = 0;\n"),
          ("                for (; i < 8; i++) {\n                    if (rec->unk_414[i][0] == rec->unk_428) {\n                        found = i;\n                    } else {\n                        temp = rec->unk_414[i][1] - 1;\n                        if (rec->unk_414[i][1] < temp3) {\n                            slot = i;\n                            temp3 = rec->unk_414[i][1];\n                        }\n                        if (temp < 0) {\n                            temp = 0;\n                        }\n                        rec->unk_414[i][1] = temp;",
           "                for (; k < 8; k++) {\n                    if (rec->unk_414[k][0] == rec->unk_428) {\n                        found = k;\n                    } else {\n                        temp = rec->unk_414[k][1] - 1;\n                        if (rec->unk_414[k][1] < temp3) {\n                            slot = k;\n                            temp3 = rec->unk_414[k][1];\n                        }\n                        if (temp < 0) {\n                            temp = 0;\n                        }\n                        rec->unk_414[k][1] = temp;")],
 'mask': [("        if (rec->unk_428 != rec->unk_00->unk_5C) {\n", "        if (rec->unk_428 != rec->unk_00->unk_5C) {\n            s32 k;\n\n"),
          ("            for (i = 0; i < 8; i++) {\n                if (rec->unk_414[i][0] == rec->unk_428 && rec->unk_414[i][1] != 0 &&\n                    rec->unk_414[i][1] >= rec->unk_424) {\n                    temp = 0x20;\n                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {",
           "            for (k = 0; k < 8; k++) {\n                if (rec->unk_414[k][0] == rec->unk_428 && rec->unk_414[k][1] != 0 &&\n                    rec->unk_414[k][1] >= rec->unk_424) {\n                    temp = 0x20;\n                    if (rec->unk_414[k][1] >= rec->unk_424 * 4) {")],
 'obj': [("        rec->unk_425 = 0;\n        for (i = 0; i < 12; i++) {\n            Obj80106A78 *obj = &D_80106A78[i];",
          "        s32 k;\n\n        rec->unk_425 = 0;\n        for (k = 0; k < 12; k++) {\n            Obj80106A78 *obj = &D_80106A78[k];")],
 'retry': [("    i = 0;\n    do {", "    k = 0;\n    do {"),
           ("        i++;\n    } while (temp == -1 && i < 4);", "        k++;\n    } while (temp == -1 && k < 4);"),
           ("    s32 i; /* SOTN", "    s32 k;\n    s32 i; /* SOTN")],
}
made = []
for nm, subs in V.items():
    t = body
    for o, n in subs:
        t = one(t, o, n)
    open('%s_%s.c' % (prefix, nm), 'w', newline='\n').write(t)
    made.append('%s_%s' % (prefix.split('/')[-1], nm))
print(' '.join(made))
