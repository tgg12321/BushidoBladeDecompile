"""Per-value spellings with duplicated-into-arms / hoisted writes (layer-2 FAIL 2026-09-26 frontier:
jump2 cross-jump, duplicated-statement-into-arms, hoisting/sinking across branches).
Base: v/pv.c (the one-variable-per-value twin of candidate.c). V's declaration is hoisted to the
loop body so a statement in the other arm / the join can name it. J_ANN = the join block's flag
test reading V through an annihilating detour `(V & 1) >> 1` (== 0 for every V; combine folds it).
Outputs v/arm_<name>.c.
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
V = os.path.join(HERE, "v")
pv = open(os.path.join(V, "pv.c")).read()


def sub1(s, old, new):
    assert s.count(old) == 1, (old, s.count(old))
    return s.replace(old, new)


LOOP_TOP = "    for (i = 0; i < 2; i++) {\n"
PRE_IF = "        if (*(u8 *)(p + 0x17) & 1) {\n"
J_TEST = "        if (*(u8 *)(p + 0x18) & 0x40) {\n"
IF_TAIL = "            *(s32 *)((u8 *)rob + 0x58) = D_80102768 + *(u16 *)(entry_a + 2);\n"
ELSE_TAIL = "            *(s32 *)((u8 *)rob + 0x58) = D_801027B0[sel][2] + *(u16 *)(entry_b + 2);\n"
SELW = "            sel = (*(u8 *)(p + 0x17) >> 1) & 1;\n"
EAW = "            entry_a = D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4;\n"
DECL = {"sel": "            s32 sel;\n", "entry_a": "            s32 entry_a;\n\n"}


def hoist_decl(s, v):
    s = sub1(s, DECL[v], "")
    return sub1(s, LOOP_TOP, LOOP_TOP + "        s32 %s;\n\n" % v)


def j_ann(s, v):
    return sub1(s, J_TEST, "        if (*(u8 *)(p + 0x18) & (0x40 + ((%s & 1) >> 1))) { /* FAKE */\n" % v)


out = {}
h = hoist_decl(pv, "sel")
# selector: the selector statement duplicated at the tail of the if arm (+ join read / without)
d = sub1(h, IF_TAIL, IF_TAIL + SELW.replace(";\n", "; /* FAKE */\n"))
out["arm_sel_dup_ann"] = j_ann(d, "sel")
out["arm_sel_dup"] = d
# selector: the if arm writes other values into V, read after the join
out["arm_sel_zero_ann"] = j_ann(sub1(h, IF_TAIL, IF_TAIL + "            sel = 0; /* FAKE */\n"), "sel")
out["arm_sel_i8_ann"] = j_ann(sub1(h, IF_TAIL, IF_TAIL + "            sel = i * 8; /* FAKE */\n"), "sel")
# selector: write hoisted above the branch (read only in the else arm)
out["arm_sel_hoist"] = sub1(sub1(h, SELW, ""), PRE_IF, SELW[4:] + PRE_IF)
h = hoist_decl(pv, "entry_a")
# entry: entry_a's statement duplicated at the else-arm tail (+ join read / without)
d = sub1(h, ELSE_TAIL, ELSE_TAIL + EAW.replace(";\n", "; /* FAKE */\n"))
out["arm_ent_dup_ann"] = j_ann(d, "entry_a")
out["arm_ent_dup"] = d
# entry: the else arm copies its own entry into entry_a, read after the join
out["arm_ent_copy_ann"] = j_ann(sub1(h, ELSE_TAIL, ELSE_TAIL + "            entry_a = entry_b; /* FAKE */\n"), "entry_a")
out["arm_ent_zero_ann"] = j_ann(sub1(h, ELSE_TAIL, ELSE_TAIL + "            entry_a = 0; /* FAKE */\n"), "entry_a")
# entry: write hoisted above the branch (read only in the if arm)
out["arm_ent_hoist"] = sub1(sub1(h, EAW, ""), PRE_IF, EAW[4:] + PRE_IF)
for k, s in out.items():
    open(os.path.join(V, k + ".c"), "w", newline="\n").write(s)
print(" ".join(sorted(out)))
