def sub(p, old, new):
    t = open(p, encoding="utf-8", newline="").read()
    assert t.count(old) == 1, (p, old[:60])
    open(p, "w", encoding="utf-8", newline="\n").write(t.replace(old, new))
sub("include/game.h",
    "/* Game types for the translation units in src/main/ (SLUS_006.63's game code): records, tables\n"
    " * and object layouts. Some are used by several translation units, others by only one. The objects\n"
    " * and functions are declared in bb2.h, which includes this file; Sony's library types come from\n"
    " * include/psxsdk/. */",
    "/* Game types for the translation units in src/main/ (SLUS_006.63's game code): records, tables\n"
    " * and object layouts. The objects and functions are declared in bb2.h, which includes this file;\n"
    " * Sony's library types come from include/psxsdk/. */")
sub("include/bb2.h",
    "/* Declarations of the game's objects and functions for the translation units in src/main/\n"
    " * (SLUS_006.63's game code); their types are in game.h. Many are shared by several translation\n"
    " * units; the others are used by only one and have not been moved back to it. */",
    "/* Declarations of the game's objects and functions for the translation units in src/main/\n"
    " * (SLUS_006.63's game code); their types are in game.h. */")
sub("include/game.h",
    " * CdState D_80101E58 below, where the evidence that they are one object with\n",
    " * CD state block D_80101E58 (type CdState below), where the evidence that they are one object with\n")
print("ok")
