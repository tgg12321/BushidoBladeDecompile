def sub(p, old, new):
    t = open(p, encoding="utf-8", newline="").read()
    assert t.count(old) == 1, (p, old)
    open(p, "w", encoding="utf-8", newline="\n").write(t.replace(old, new))
sub("include/code6cac.h",
    "/* PsyQ VECTOR / SVECTOR layouts (include/gte.h), spelled with local tags for\n * the same reason as Unk80101DF0Rot below.  func_80022580 copies",
    "/* PsyQ VECTOR / SVECTOR layouts (include/psxsdk/libgte.h) under local names, like\n * Unk80101DF0Rot below; retyping them as the Sony types is Phase 2 work.  func_80022580 copies")
sub("include/code6cac.h",
    "/* PsyQ MATRIX layout (include/gte.h), spelled with a local tag for the same\n * reason as Unk80101DF0Mat below (several TUs typedef MATRIX themselves). */",
    "/* PsyQ MATRIX layout (include/psxsdk/libgte.h) under a local name, like\n * Unk80101DF0Mat below; retyping it as MATRIX is Phase 2 work. */")
sub("include/code6cac.h",
    " * Rot / Mat are the PsyQ SVECTOR / MATRIX layouts\n * (include/gte.h), spelled with local tags because several TUs typedef\n * SVECTOR/MATRIX themselves. */",
    " * Rot / Mat are the PsyQ SVECTOR / MATRIX layouts\n * (include/psxsdk/libgte.h) under local names: they were spelled locally while\n * several TUs typedef'd SVECTOR/MATRIX themselves; retyping them is Phase 2 work. */")
sub("include/game.h",
    " * this header needs no gte.h; the type is complete wherever gte.h is included. */",
    " * this header needs no libgte.h; the type is complete wherever <psxsdk/libgte.h> is\n * included (gte.h and code6cac.h include it). */")
