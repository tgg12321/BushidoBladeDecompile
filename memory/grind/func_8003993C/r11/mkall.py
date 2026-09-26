"""Generate every measured body of the Ruling 11 proof FROM candidate.c (= r11/final.c), into r11/v/.

  v/pv.c        full one-variable-per-value twin (mk_pv.py both): temp -> sel + window, entry -> entry_a + entry_b
  v/pv_temp.c   only `temp` split            v/pv_entry.c  only `entry` split
  v/ext_<V>_<m>.c  pv.c + ONE sanctioned-family construct on V in {sel, entry_a}, outside V's arm
                   (m = hoist | dead | self | chain | use | ann | pre | alias | dowhile | dowhile2 | dup)
  v/st_<k>.c    structural respellings of pv.c (noentry | row | tern | cond)
  v/h_<k>.c     candidate.c with one other construct removed (nonext | rec)
usage: python3 mkall.py [candidate.c]
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CAND = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "final.c")
V = os.path.join(HERE, "v")
os.makedirs(V, exist_ok=True)


def sub1(s, old, new):
    assert s.count(old) == 1, (old, s.count(old))
    return s.replace(old, new)


def w(name, s):
    open(os.path.join(V, name + ".c"), "w", newline="\n").write(s)


for mode, name in (("both", "pv"), ("temp", "pv_temp"), ("entry", "pv_entry")):
    subprocess.run([sys.executable, os.path.join(HERE, "mk_pv.py"), CAND, os.path.join(V, name + ".c"), mode], check=True)
cand = open(CAND).read()
pv = open(os.path.join(V, "pv.c")).read()

LOOP_TOP = "    for (i = 0; i < 2; i++) {\n"
PRE_IF = "        if (*(u8 *)(p + 0x17) & 1) {\n"
J_TEST = "        if (*(u8 *)(p + 0x18) & 0x40) {\n"
SELW = "            sel = (*(u8 *)(p + 0x17) >> 1) & 1;\n"
EB = "            entry_b = D_801027B0[sel][1] + *(u16 *)(*(s32 *)p + 4) * 4;\n"
EAW = "            entry_a = D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4;\n"
DECL = {"sel": "            s32 sel;\n", "entry_a": "            s32 entry_a;\n\n"}


def hoist(s, v):
    s = sub1(s, DECL[v], "")
    return sub1(s, LOOP_TOP, LOOP_TOP + "        s32 %s;\n\n" % v)


for v in ("sel", "entry_a"):
    h = hoist(pv, v)
    w("ext_%s_hoist" % v, h)
    w("ext_%s_dead" % v, sub1(h, PRE_IF, "        %s = 0; /* FAKE */\n" % v + PRE_IF))
    w("ext_%s_self" % v, sub1(h, PRE_IF, "        %s = %s; /* FAKE */\n" % (v, v) + PRE_IF))
    w("ext_%s_chain" % v, sub1(h, PRE_IF, "        %s = %s + 1 - 1; /* FAKE */\n" % (v, v) + PRE_IF))
    w("ext_%s_use" % v, sub1(h, J_TEST, "        if (*(u8 *)(p + 0x18) & (0x40 + (%s - %s))) {\n" % (v, v)))
    w("ext_%s_ann" % v, sub1(h, J_TEST, "        if (*(u8 *)(p + 0x18) & (0x40 + ((%s & 1) >> 1))) {\n" % v))
    a = sub1(h, "        s32 %s;\n\n" % v, "        s32 %s;\n        s32 *%s_p = &%s; /* FAKE */\n\n" % (v, v, v))
    if v == "sel":
        a = a.replace("D_801027B0[sel]", "D_801027B0[*sel_p]")
    else:
        a = sub1(a, "*(u16 *)(entry_a + 2)", "*(u16 *)(*entry_a_p + 2)")
    w("ext_%s_alias" % v, a)
    # function-scope declaration + a read of the (still unwritten) variable before the loop
    pre = sub1(pv, DECL[v], "")
    pre = sub1(pre, "    s32 window;\n", "    s32 window;\n    s32 %s;\n" % v)
    pre = sub1(pre, "    func_8001E6E4(prog);\n", "    func_8001E6E4(prog + ((%s & 1) >> 1)); /* FAKE */\n" % v)
    w("ext_%s_pre" % v, pre)
w("ext_sel_dowhile", sub1(pv, SELW, SELW + "            do {\n            } while (0); /* FAKE */\n"))
w("ext_sel_dowhile2", sub1(pv, SELW + EB, SELW + "            do {\n    " + EB + "            } while (0); /* FAKE */\n"))
w("ext_sel_dup", sub1(pv, EB, EB + SELW.replace(";\n", "; /* FAKE */\n")))
w("ext_entry_a_dowhile", sub1(pv, EAW, EAW + "            do {\n            } while (0); /* FAKE */\n"))

IFARM = '''            s32 entry_a;

            entry_a = D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4;
            *(s32 *)((u8 *)rob + 0x58) = D_80102768 + *(u16 *)(entry_a + 2);
'''
ELSEARM = '''            s32 sel;
            s32 entry_b;

            sel = (*(u8 *)(p + 0x17) >> 1) & 1;
            entry_b = D_801027B0[sel][1] + *(u16 *)(*(s32 *)p + 4) * 4;
            *(s32 *)((u8 *)rob + 0x58) = D_801027B0[sel][2] + *(u16 *)(entry_b + 2);
'''
s = sub1(pv, IFARM, '''            *(s32 *)((u8 *)rob + 0x58) = D_80102768
                + *(u16 *)(D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4 + 2);
''')
s = sub1(s, ELSEARM, '''            s32 sel = (*(u8 *)(p + 0x17) >> 1) & 1;
            *(s32 *)((u8 *)rob + 0x58) = D_801027B0[sel][2]
                + *(u16 *)(D_801027B0[sel][1] + *(u16 *)(*(s32 *)p + 4) * 4 + 2);
''')
w("st_noentry", s)
w("st_row", sub1(pv, ELSEARM, '''            s32 sel;
            s32 *row;
            s32 entry_b;

            sel = (*(u8 *)(p + 0x17) >> 1) & 1;
            row = D_801027B0[sel];
            entry_b = row[1] + *(u16 *)(*(s32 *)p + 4) * 4;
            *(s32 *)((u8 *)rob + 0x58) = row[2] + *(u16 *)(entry_b + 2);
'''))
i0 = pv.index("    if (D_800A3782 != 0) {\n        window = ")
i1 = pv.index("    e = (u8 *)D_800F68E0;")
w("st_tern", pv[:i0] + '''    {
        s32 next = D_800A37D0 + 1;
        window = D_800A3782 != 0 ? 0x77 - D_800A37D0 : D_800A36F8 - next;
    }
''' + pv[i1:])
w("st_cond", sub1(pv, "        if (*(u8 *)(p + 0x17) & 1) {\n" + IFARM + "        } else {\n" + ELSEARM + "        }\n", '''        {
            s32 frame = *(u16 *)(*(s32 *)p + 4);
            s32 sel = (*(u8 *)(p + 0x17) >> 1) & 1;
            *(s32 *)((u8 *)rob + 0x58) = (*(u8 *)(p + 0x17) & 1)
                ? D_80102768 + *(u16 *)(D_80102764 + frame * 4 + 2)
                : D_801027B0[sel][2] + *(u16 *)(D_801027B0[sel][1] + frame * 4 + 2);
        }
'''))

k0 = cand.index("        /* FAKE: named intermediate")
k1 = cand.index("        temp = D_800A36F8 - next;\n") + len("        temp = D_800A36F8 - next;\n")
w("h_nonext", cand[:k0] + "        temp = D_800A36F8 - (D_800A37D0 + 1);\n" + cand[k1:])
R = "(u8 *)(D_800A36EC + idx * 56)"
pre_end = cand.index("    func_8001E6E4(prog);")
k = cand.index("    func_8001BAE4(")
s = cand[:k] + "    rec = " + R + ";\n" + cand[k:pre_end].replace(R, "rec") + cand[pre_end:]
w("h_rec", sub1(s, "    u8 *e;\n", "    u8 *e;\n    u8 *rec;\n"))
print("\n".join(sorted(os.listdir(V))))
