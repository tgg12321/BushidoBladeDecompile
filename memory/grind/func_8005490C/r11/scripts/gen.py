"""gen.py <base.c> <outdir>: Ruling 11 partition variants of the reuse body.
player values: I0 (init player 0), I1 (init player 1), L (loop player i).
rot_z values: Z0 (camera rotation), Z1 (loop rotation)."""
import sys
from pathlib import Path

base = Path(sys.argv[1]).read_text()
out = Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)

INIT0 = """        player = func_8004153C(0);
        if (player != 0) {
            func_8003FFC4(player);
        }
"""
INIT1 = """        player = func_8004153C(1);
        if (player != 0) {
            func_8003FFC4(player);
        }
"""
LOOPW = "            player = func_8004153C(i);\n"
LOOPR = "*(s16 *)((u8 *)player + 0x12)"
DECLS = "    s32 *player;\n    s32 rot_z;\n"
for k in (INIT0, INIT1, LOOPW, LOOPR, DECLS):
    assert base.count(k) == 1, k
ROT = """                rot_z = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
                vec.vz = rot_z;
"""
ROT0 = ROT.replace("        ", "", 1).replace("\n        ", "\n")
assert base.count(ROT) == 1 and base.count(ROT0) == 1


def make(pgroups, zsplit):
    """pgroups: list of groups (subsets of 'I0','I1','L') sharing one local."""
    s = base
    decl_fn = []
    names = {}
    for gi, g in enumerate(pgroups):
        nm = "player" if len(pgroups) == 1 else "player%d" % gi
        for v in g:
            names[v] = nm
    # innermost scope: a group containing L and an init value -> function scope;
    # init-only group -> init block; L-only -> loop block
    init_decl, loop_decl = [], []
    for g in pgroups:
        nm = names[g[0]]
        if "L" in g and len(g) > 1:
            decl_fn.append(nm)
        elif "L" in g:
            loop_decl.append(nm)
        else:
            init_decl.append(nm)
    s = s.replace(INIT0, INIT0.replace("player", names["I0"]))
    s = s.replace(INIT1, INIT1.replace("player", names["I1"]))
    s = s.replace(LOOPW, LOOPW.replace("player", names["L"]))
    s = s.replace(LOOPR, LOOPR.replace("player", names["L"]))
    d = "".join("    s32 *%s;\n" % n for n in decl_fn)
    if zsplit:
        s = s.replace(ROT, ROT.replace("rot_z", "rot_z1"))
        s = s.replace(ROT0, ROT0.replace("rot_z", "rot_z0"))
        s = s.replace("        s32 z = vec.vz;\n        rot_z0", "        s32 z = vec.vz;\n        s32 rot_z0;\n\n        rot_z0")
        s = s.replace("                s32 z = vec.vz;\n                rot_z1", "                s32 z = vec.vz;\n                s32 rot_z1;\n\n                rot_z1")
    else:
        d += "    s32 rot_z;\n"
    s = s.replace(DECLS, d)
    if init_decl:
        k = "    if (s->unk0 == 0) {\n"
        assert s.count(k) == 1
        s = s.replace(k, k + "".join("        s32 *%s;\n" % n for n in init_decl) + "\n")
    if loop_decl:
        assert s.count("        if (s->unk34[i] != 0) {\n            s32 ang;") == 1
        s = s.replace("        if (s->unk34[i] != 0) {\n            s32 ang;",
                      "        if (s->unk34[i] != 0) {\n" + "".join("            s32 *%s;\n" % n for n in loop_decl)
                      + "            s32 ang;")
    return s


PART = {
    "P_all": [["I0", "I1", "L"]],
    "P_split": [["I0"], ["I1"], ["L"]],
    "P_I0|I1L": [["I0"], ["I1", "L"]],
    "P_I1|I0L": [["I1"], ["I0", "L"]],
    "P_I0I1|L": [["I0", "I1"], ["L"]],
}
for pn, pg in PART.items():
    for zn, zs in (("Zsh", False), ("Zsplit", True)):
        fn = out / ("%s_%s.c" % (pn.replace("|", "-"), zn))
        open(fn, "w", newline="\n").write(make(pg, zs))
        print(fn)
