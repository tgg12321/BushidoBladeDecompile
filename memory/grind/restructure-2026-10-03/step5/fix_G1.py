"""fix_G1.py: comment fixes for the game.h / bb2.h split (run from a tree root)."""
import re


def rd(p):
    return open(p, encoding="utf-8", newline="").read()


def wr(p, t):
    open(p, "w", encoding="utf-8", newline="\n").write(t)


def sub(t, old, new, p):
    assert t.count(old) == 1, (p, old[:80])
    return t.replace(old, new)


b = rd("include/bb2.h")
b = sub(b, "/* Shared declarations of the game code in SLUS_006.63 (the translation units in src/main/): the\n"
           " * objects and functions more than one of them uses. Their types are in game.h. */",
        "/* Declarations of the game's objects and functions for the translation units in src/main/\n"
        " * (SLUS_006.63's game code); their types are in game.h. Most are shared by several translation\n"
        " * units; some are used by only one and have not been moved back to it. */", "bb2.h")
b = sub(b, " * Defined in text1b_tu1b.c. */", " * Defined in src/main/3AB48.c. */", "bb2.h")
b = sub(b, "(func_8003047C);\n\n                                       0x8008E338..0x8008E3BE, then one alignment byte */\n"
           "extern u16 D_8008E3C0[28];          /* [unk_0A] -> Unk80101EC8Record.unk_274 */\n\n",
        "(func_8003047C);\n                                       0x8008E338..0x8008E3BE, then one alignment byte */\n"
        "extern u16 D_8008E3C0[28];          /* [unk_0A] -> Unk80101EC8Record.unk_274 */\n", "bb2.h")
wr("include/bb2.h", b)

g = rd("include/game.h")
g = sub(g, "/* Bushido Blade 2's shared game types: the records, tables and object layouts several\n"
           " * translation units use. The shared objects and functions themselves are declared in bb2.h,\n"
           " * which includes this file; Sony's library types come from include/psxsdk/. */",
        "/* Game types for the translation units in src/main/ (SLUS_006.63's game code): records, tables\n"
        " * and object layouts. Most are used by several translation units; some by only one. The objects\n"
        " * and functions are declared in bb2.h, which includes this file; Sony's library types come from\n"
        " * include/psxsdk/. */", "game.h")
m = re.search(r"/\* Two s16 slots at 0x800A34F0, indexed as one array\..*?D_800A34F0 / D_800A34F2\. \*/\n\n", g, re.S)
assert m
orphan = m.group(0)
g = g.replace(orphan, "")
m = re.search(r"/\* Per-character record table \(base 0x80101EC8.*?declared at its offset\)\. \*/\n", g, re.S)
assert m
rec = m.group(0)
g = g.replace(rec, "")
g = sub(g, "typedef struct Unk80101EC8Record {", rec + "typedef struct Unk80101EC8Record {", "game.h")
wr("include/game.h", g)

s = rd("src/main/51268.c")
s = sub(s, "\nstatic s16 D_800A34F0[2];\n", "\n" + orphan.rstrip("\n") + "\nstatic s16 D_800A34F0[2];\n", "51268.c")
wr("src/main/51268.c", s)
print("ok")
