#!/usr/bin/env python3
"""Restructure step 4d, split B: src/main/psxsdk/libgpu/prim.c (ex gpu.c, as committed at 1be68857c)
-> LIBC2 MEMMOVE, the ten LIBCARD modules, LIBGPU EXT and PRIM; LIBGPU SYS's head (ResetGraph ..
GetGraphDebug) moves in front of src/main/psxsdk/libgpu/sys.c (ex display.c, 6011f1eb3), which then
spans the whole LIBGPU SYS module across the old gpu|display cut (0x8007B244).

Source lines move verbatim; only declarations a part needs to compile are added. Dropped: the two
stale "--- Functions 0x... ---" banners and gpu.c's unused typedef block (NULL, Vec*, VECTOR, SVECTOR,
CVECTOR, DVECTOR, MATRIX, GameObj: no part uses any of them)."""
import os, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):  # the repo root, from tmp/ or the banked copy
    ROOT = os.path.dirname(ROOT)
PRIM = os.path.join(ROOT, "src/main/psxsdk/libgpu/prim.c")
SYS = os.path.join(ROOT, "src/main/psxsdk/libgpu/sys.c")


def lines(p):
    t = open(p, encoding="utf-8", newline="").read()
    assert "\r" not in t
    L = t.split("\n")
    assert L[-1] == ""
    return L[:-1]


G, D = lines(PRIM), lines(SYS)


def gl(a, b=None):
    return G[a - 1:(a if b is None else b)]


# anchors (gpu.c as committed)
assert gl(1, 5) == ['#include "common.h"', '#include "include_asm.h"', '#include "bios.h"',
                    '#include "gpu.h"', '#include "psx.h"']
assert gl(8, 9) == ["extern void StopCARD2(void);", "extern void _ExitCard(void);"]
assert G[12 - 1] == "extern s32 D_80015D58;" and G[27 - 1] == "extern s32 D_80015E7C;"
assert gl(28, 30) == ["extern s32 D_8009BE2C;", "extern s32 D_8009BEF4[];", "extern s32 D_8009BF08[];"]
assert G[32 - 1].startswith("/* --- Functions 0x8007A28C") and G[34 - 1] == "#define NULL ((void *)0)"
assert G[71 - 1] == "} GameObj;" and G[72 - 1].startswith("/* PsyQ 4.0 LIBC2 MEMMOVE")
assert G[85 - 1] == "}" and G[86 - 1] == "BIOS_A_FUNCTION(_card_info, 0xAB);"
assert G[88 - 1].startswith("void _card_clear(") and G[91 - 1] == "}"
assert G[94 - 1].startswith("void InitCARD(") and G[113 - 1] == "}"
assert G[115 - 1].startswith("BIOS_B_FUNCTION(InitCARD2") and G[118 - 1] == 'INCLUDE_ASM("asm/funcs", _ExitCard);'
assert G[119 - 1].startswith("u16 LoadTPage(") and G[198 - 1] == "}"
assert G[199 - 1].startswith("u32 GetTPage(") and G[442 - 1] == "}"
assert G[443 - 1].startswith("/* PsyQ LIBGPU sys.c v1.129: ResetGraph") and len(G) == 523
# anchors (display.c as committed)
assert D[1 - 1] == '#include "common.h"' and D[5 - 1] == '#include "psx.h"' and D[6 - 1] == "" and D[7 - 1] == ""
assert D[8 - 1] == "/* Forward declarations */" and D[31 - 1] == "extern const char D_80015F4C[];"
assert D[32 - 1] == "" and D[33 - 1].startswith("/* --- Functions 0x8007B244") and D[34 - 1] == ""
assert D[35 - 1].startswith("u32 DrawSyncCallback(")

SPAN = "a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3"
FROM = "/* Declarations from the file this module was split from (gpu.c). */"


def comment(lib, mod, st, en, what):
    s = f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{st:08X}..0x{en:08X}, {SPAN}. */"
    out, cur = [], ""
    for w in s.split(" "):  # wrap at 100 columns
        if len(cur) + len(w) + 1 > 100:
            out.append(cur)
            cur = " *"
        cur = (cur + " " + w) if cur else w
    return "\n".join(out + [cur])


def asm_only(lib, mod, st, what, body, bios=True):
    return (comment(lib, mod, st, st + 0x10 if bios else ENDS[mod], what) + "\n"
            "#define INCLUDE_ASM_USE_MACRO_INC 1\n#include \"include_asm.h\"\n"
            + ("#include \"bios.h\"\n" if bios else "") + "\n" + "\n".join(body) + "\n")


def c_file(lib, mod, st, en, what, body, incs=("common.h",), decls=()):
    s = comment(lib, mod, st, en, what) + "\n" + "".join(f'#include "{i}"\n' for i in incs) + "\n"
    if decls:
        s += FROM + "\n" + "\n".join(decls) + "\n\n"
    return s + "\n".join(body) + "\n"


ENDS = {"END": 0x8007A4D8}
C2, CARD, GPU = "src/main/psxsdk/libc2/", "src/main/psxsdk/libcard/", "src/main/psxsdk/libgpu/"
files = [  # link order
    (C2 + "memmove.c", c_file("LIBC2", "MEMMOVE", 0x8007A28C, 0x8007A2F8,
                              "memmove (LIBC and LIBC2 MEMMOVE are byte-identical; libc2/ as for sprintf.c)",
                              gl(72, 85))),
    (CARD + "c171.c", asm_only("LIBCARD", "C171", 0x8007A2F8, "_card_info, the BIOS A(0xAB) trampoline", gl(86))),
    (CARD + "c172.c", asm_only("LIBCARD", "C172", 0x8007A308, "_card_load, the BIOS A(0xAC) trampoline", gl(87))),
    (CARD + "card.c", c_file("LIBCARD", "CARD", 0x8007A318, 0x8007A350, "_card_clear", gl(88, 91))),
    (CARD + "a78.c", asm_only("LIBCARD", "A78", 0x8007A350, "_card_write, the BIOS B(0x4E) trampoline", gl(92))),
    (CARD + "a80.c", asm_only("LIBCARD", "A80", 0x8007A360, "_new_card, the BIOS B(0x50) trampoline", gl(93))),
    (CARD + "init.c", c_file("LIBCARD", "INIT", 0x8007A370, 0x8007A428, "InitCARD, StartCARD and StopCARD",
                             gl(94, 113), decls=gl(8, 9))),
    (CARD + "a74.c", asm_only("LIBCARD", "A74", 0x8007A428, "InitCARD2, the BIOS B(0x4A) trampoline", gl(115))),
    (CARD + "a75.c", asm_only("LIBCARD", "A75", 0x8007A438, "StartCARD2, the BIOS B(0x4B) trampoline", gl(116))),
    (CARD + "a76.c", asm_only("LIBCARD", "A76", 0x8007A448, "StopCARD2, the BIOS B(0x4C) trampoline", gl(117))),
    (CARD + "end.c", asm_only("LIBCARD", "END", 0x8007A458, "_ExitCard (hand-written asm)", gl(118), bios=False)),
    (GPU + "ext.c", c_file("LIBGPU", "EXT", 0x8007A4D8, 0x8007A788,
                           "LoadTPage, LoadClut, LoadClut2, SetDefDrawEnv and SetDefDispEnv", gl(119, 198))),
]
IDS = [p[len("src/"):-2] for p, _ in files]

PRIM_TOP = comment("LIBGPU", "PRIM", 0x8007A788, 0x8007AE7C,
                   "the primitive and environment helpers (GetTPage .. DumpDispEnv)")
SYS_STRS = ("D_80015E5C", "D_80015E7C", "D_80015E90", "D_80015EA8", "D_80015ED4")
STR_EXTERNS = gl(12, 27)
assert all(l.startswith("extern s32 D_80015") for l in STR_EXTERNS)
prim_ex = [l for l in STR_EXTERNS if l[11:21] not in SYS_STRS]
sys_ex = [l for l in STR_EXTERNS if l[11:21] in SYS_STRS]
assert len(prim_ex) == 11 and len(sys_ex) == 5
prim = ([PRIM_TOP] + gl(1, 1) + gl(4, 5) + [""] + ["/* Externs for globals */"] + prim_ex + [""]
        + gl(199, 442))
SYS_TOP = (comment("LIBGPU", "SYS", 0x8007AE7C, 0x8007DF10,
                   "the GPU system layer (ResetGraph .. memset; $Id: sys.c,v 1.129)") + "\n"
           "/* One file across the old gpu.c|display.c cut at 0x8007B244, which was mid-module (Q106 D3).
"
           " * Until the next step-4d commit the file also carries LIBAPI C73, LIBGTE GEO_00..PATCHGTE and LIBCD
"
           " * EVENT (.text 0x8007DF10..0x8008008C), the rest of the old display.c. */")
sys_ = ([SYS_TOP] + D[0:5] + [""] + D[7:31] + [""]
        + ["/* Declarations from the file ResetGraph .. GetGraphDebug were split from (gpu.c). */"]
        + sys_ex + gl(28, 30) + [""] + gl(443, 523) + [""] + D[34:])

# every gpu.c line is accounted for
used = set(range(1, 6)) | {8, 9} | set(range(12, 31)) | set(range(72, 119)) | set(range(119, 524))
drop = {6, 7, 10, 11, 31, 32, 33} | set(range(34, 72)) | {114}
for i in range(1, len(G) + 1):
    assert i in used or i in drop, i
    if i in drop and i not in range(34, 72):
        assert G[i - 1].strip() in ("", "/* Forward declarations */", "/* Externs for globals */") \
            or G[i - 1].startswith("/* --- Functions"), (i, G[i - 1])
assert G[3 - 1] == '#include "bios.h"'  # prim.c no longer has a BIOS stub

if "--apply" in sys.argv:
    for path, text in files:
        p = os.path.join(ROOT, path)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        assert not os.path.exists(p), p
        open(p, "w", encoding="utf-8", newline="\n").write(text)
    open(PRIM, "w", encoding="utf-8", newline="\n").write("\n".join(prim) + "\n")
    open(SYS, "w", encoding="utf-8", newline="\n").write("\n".join(sys_) + "\n")
if "--ids" in sys.argv:
    print(" ".join(IDS))
else:
    for path, text in files:
        print("=====", path); print(text)
    print("===== prim head"); print("\n".join(prim[:24]))
    print("===== sys head"); print("\n".join(sys_[:50]))
