# F02 batch 3, second review fix: one union member for the two-point pair (func_8002A458 and
# func_80031B24 lay it out identically), not two per-function views.
NL = "\n"
def sub1(t, a, b):
    assert t.count(a) == 1, a[:70]; return t.replace(a, b)
p = "include/game.h"
g = open(p, encoding="utf-8").read()
i = g.index("/* Unk1F8002B8Rec.unk00 is per-function scratch:")
j = g.index("} Unk1F8002B8Unk00;\n") + len("} Unk1F8002B8Unk00;\n")
g = g[:i] + (
"/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch. unk00 is two points that unk60[0] / unk60[1] aim at\n"
" * for func_8002E838 / func_8002EA24: func_8002A458's segment, base then tip (func_8002AB08 writes\n"
" * them before each call), and func_80031B24's D_80106A78 object step, prev_pos then pos.\n"
" * func_8002AB08 lays out more points after these two and still uses the bytes through its u8 *\n"
" * pointer. func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument\n"
" * block, its work area from +0x38 through +0x123) and have no member here. raw sizes the union\n"
" * to 0x60. */\n"
"typedef union {\n"
"    u8 raw[0x60];\n"
"    LeafPos unk00[2];\n"
"} Unk1F8002B8Unk00;\n") + g[j:]
g = sub1(g, " * (0,0) / unkA8 / unkB8. unk00 is per-function scratch (Unk1F8002B8Unk00): func_8002A458 /\n"
            " * func_80031B24 use their members; func_8002AB08 / func_80030D7C (u8 *) and func_800321E8\n"
            " * (s32 *) still use it through raw pointers. */\n",
            " * (0,0) / unkA8 / unkB8. unk00 is scratch (Unk1F8002B8Unk00): func_8002A458 / func_80031B24\n"
            " * use its two points; func_8002AB08 / func_80030D7C (u8 *) and func_800321E8 (s32 *) still\n"
            " * use it through raw pointers. */\n")
open(p, "w", encoding="utf-8", newline=NL).write(g)
p = "src/main/17AFC.c"
s = open(p, encoding="utf-8").read()
assert s.count("scr->unk00.v8002A458.unk00") == 3 and s.count("scr->unk00.v80031B24.unk00") == 1
s = s.replace("scr->unk00.v8002A458.unk00", "scr->unk00.unk00").replace("scr->unk00.v80031B24.unk00", "scr->unk00.unk00")
open(p, "w", encoding="utf-8", newline=NL).write(s)
m = "tmp/p2/msg_f02b3.txt"
t = open(m, encoding="utf-8").read()
t = sub1(t, "phase2: Unk1F8002B8Rec.unk00 is a union of per-function views; func_8002A458 / func_80031B24 and their callees read the record typed\n",
            "phase2: Unk1F8002B8Rec.unk00 holds two LeafPos; func_8002A458 / func_80031B24 and their callees read the record typed\n")
i = t.index("  - Unk1F8002B8Rec.unk00 (0x60 bytes) is per-function scratch.")
j = t.index("  - The record comment:")
t = t[:i] + (
"  - Unk1F8002B8Rec.unk00 (0x60 bytes of scratch) becomes `Unk1F8002B8Unk00 unk00`, a union:\n"
"    - `LeafPos unk00[2]`: the two points unk60[0] / unk60[1] aim at for func_8002E838 /\n"
"      func_8002EA24. func_8002A458's segment, base then tip (func_8002AB08 writes them before\n"
"      each call), and func_80031B24's object step, prev_pos then pos. The two functions lay\n"
"      these bytes out identically, so they share the one member.\n"
"    - `u8 raw[0x60]` sizes the union. The comment says func_8002AB08 lays out more points after\n"
"      these two and still uses the bytes through its u8 * pointer, and that func_80030D7C /\n"
"      func_800321E8 lay the bytes out differently (func_8005344C's argument block; see the debt\n"
"      rows), so they have no member here.\n") + t[j:]
t = sub1(t, "scratch (Unk1F8002B8Unk00): func_8002A458 / func_80031B24 use their members, and\n",
            "scratch (Unk1F8002B8Unk00): func_8002A458 / func_80031B24 use its two points, and\n")
t = sub1(t, "&scr->unk00.v8002A458.unk00[0 / 1]`;", "&scr->unk00.unk00[0 / 1]`;")
t = sub1(t, "`scr->unkC8 = ...v8002A458.unk00[1]`;", "`scr->unkC8 = scr->unk00.unk00[1]`;")
t = sub1(t, "`seg` is `scr->unk00.v80031B24.unk00`", "`seg` is `scr->unk00.unk00`")
t = sub1(t, "so its own unk00 view comes with it in a later\n  batch.", "so a view of its further points comes with it\n  in a later batch.")
t = sub1(t, "seg[0 / 1] and v8002A458.unk00[0 / 1] index real", "seg[0 / 1] and unk00.unk00[0 / 1] index real")
t = sub1(t, "  - The record comment: \"pass it as `obj` to\"", "  - The record comment: \"pass it as `obj` to\"")
open(m, "w", encoding="utf-8", newline=NL).write(t)
print("ok")
