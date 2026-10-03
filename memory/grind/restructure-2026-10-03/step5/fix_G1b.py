def sub(p, old, new):
    t = open(p, encoding="utf-8", newline="").read()
    assert t.count(old) == 1, (p, old[:60])
    open(p, "w", encoding="utf-8", newline="\n").write(t.replace(old, new))
sub("include/game.h", " * and object layouts. Most are used by several translation units; some by only one. The objects\n",
    " * and object layouts. Some are used by several translation units, others by only one. The objects\n")
sub("include/bb2.h", " * (SLUS_006.63's game code); their types are in game.h. Most are shared by several translation\n * units; some are used by only one and have not been moved back to it. */",
    " * (SLUS_006.63's game code); their types are in game.h. Many are shared by several translation\n * units; the others are used by only one and have not been moved back to it. */")
print("ok")
