#!/usr/bin/env python3
# Local declarations hoisted (hand-off debt "87 game symbols declared locally"): the copies that agree with
# their definition go to the header that owns the symbol (libc.h rand, libgte.h rsin / rcos, libgpu.h
# SetDefDrawEnv / SetDefDispEnv, bb2.h game globals / func_800520B8); local copies of what a header
# already declares go (file scope only: a block-scope copy inside a body stays, so no body moves). Kept on purpose (hand-off / owner rulings): the original-call K&R facts
# (func_80052C10, func_80073C78, AddPrim in 3AB48), snd_VabFakeOpen / func_8005C2A8 / func_80054434 /
# func_80060414, the Q21-Q25 per-file round-score bytes (D_800A3898 / 99 / AA / AB).
# usage: fdecl1.py   writes tmp/p2/fdecl1/ (scratch only); BASE_REV env (default 54e21bed1)
import os, subprocess
NL = chr(10)
OUT = "tmp/p2/fdecl1/"
BASE_REV = os.environ.get("BASE_REV", "54e21bed1")   # D_800963EE retired by w2b5

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

# (file, the local declaration line, expected count) -- every copy goes
DROP = [
    # rand: libc2/rand.c defines s32 rand(void)
    ("src/main/175A4.c", "extern s32 rand(void);", 1), ("src/main/17AFC.c", "extern s32 rand(void);", 1),
    ("src/main/24F08.c", "extern s32 rand(void);", 1), ("src/main/25C38.c", "extern s32 rand(void);", 1),
    ("src/main/2B344.c", "extern u16 rand(void);", 1), ("src/main/3AB48.c", "extern s32 rand(void);", 1),
    ("src/main/51268.c", "extern s32 rand(void);", 1),
    ("src/main/6CF8.c", "extern s32 rand(void);", 1), ("src/main/87A0.c", "extern s32 rand();", 1),
    ("src/main/9F9C.c", "extern s32 rand();", 1),
    # rsin / rcos: libgte geo_00.c / geo_01.c define s32 rsin(s32) / s32 rcos(s32)
    ("src/main/31D3C.c", "extern s32 rcos(s32);", 1), ("src/main/31D3C.c", "extern s32 rsin(s32);", 1),
    ("src/main/32D04.c", "extern s32 rcos(s32);", 1), ("src/main/32D04.c", "extern s32 rsin(s32);", 1),
    ("src/main/35000.c", "extern s32 rcos(s32);", 2), ("src/main/35000.c", "extern s32 rsin(s32);", 2),
    ("src/main/368E4.c", "extern s32 rcos();", 1), ("src/main/368E4.c", "extern s32 rsin();", 1),
    ("src/main/51268.c", "extern s32 rcos();", 1), ("src/main/51268.c", "extern s32 rsin();", 1),
    ("src/main/5ED34.c", "extern s32 rcos();", 1), ("src/main/5ED34.c", "extern s32 rsin();", 1),
    ("src/main/63D2C.c", "extern s32 rsin();", 1), ("src/main/64FD8.c", "extern s32 rsin();", 1),
    ("src/main/64FD8.c", "extern s32 rsin(s32);", 1),
    # SetDefDrawEnv / SetDefDispEnv: libgpu ext.c
    ("src/main/51268.c", "extern DRAWENV *SetDefDrawEnv(DRAWENV *, s32, s32, s32, s32);", 1),
    ("src/main/51268.c", "extern DISPENV *SetDefDispEnv(DISPENV *, s32, s32, s32, s32);", 1),
    ("src/main/6CF8.c", "extern DRAWENV *SetDefDrawEnv(DRAWENV *, s32, s32, s32, s32);", 1),
    ("src/main/6CF8.c", "extern void SetDefDispEnv(DISPENV *, s32, s32, s32, s32);", 1),
    # game globals / functions declared in 2+ TUs (one type everywhere)
    ("src/main/2B344.c", "extern Unk80101DF0Record *D_800A3708;", 1), ("src/main/32D04.c", "extern Unk80101DF0Record *D_800A3708;", 1),
    ("src/main/368E4.c", "extern Unk80101DF0Record *D_800A3708;", 1),
    ("src/main/28514.c", "extern s32 D_800A3924;", 1),
    ("src/main/32D04.c", "extern void func_800520B8(s32, s32, s32);", 3), ("src/main/35000.c", "extern void func_800520B8(s32, s32, s32);", 2),
    # CdGetSector: libcd sys.c defines s32 CdGetSector(s32 madr, s32 size)
    ("src/main/267A8.c", "extern void CdGetSector(s32, s32);", 1),
    ("src/main/psxsdk/libcd/cdread.c", "s32 CdGetSector(s32 madr, s32 size);", 1),
    # already in a header, same type
    ("src/main/5ED34.c", "extern u32 *ClearOTagR(u32 *, s32);", 1), ("src/main/64FD8.c", "extern u32 *ClearOTagR(u32 *, s32);", 1),
    ("src/main/6CF8.c", "extern u32 *ClearOTagR(u32 *, s32);", 1),
]

ADD = [
    ("include/psxsdk/libcd.h", "extern s32 CdGetSector2(s32, s32); /* PsyQ: int CdGetSector2(void *, int) */\n",
     "extern s32 CdGetSector(s32, s32);  /* PsyQ: int CdGetSector(void *, int) */\n"
     "extern s32 CdGetSector2(s32, s32); /* PsyQ: int CdGetSector2(void *, int) */\n"),
    ("include/psxsdk/libc.h", "extern void srand(u32);\n", "extern s32 rand(void);\nextern void srand(u32);\n"),
    ("include/psxsdk/libgte.h", "extern s32 ratan2(s32, s32);\n", "extern s32 ratan2(s32, s32);\nextern s32 rsin(s32);\nextern s32 rcos(s32);\n"),
    ("include/psxsdk/libgpu.h", "extern s32 MoveImage(RECT *, s32, s32);\n",
     "extern s32 MoveImage(RECT *, s32, s32);\n"
     "extern DRAWENV *SetDefDrawEnv(DRAWENV *, s32, s32, s32, s32);\n"
     "extern DISPENV *SetDefDispEnv(DISPENV *, s32, s32, s32, s32);\n"),
]
BB2_ANCHOR = "extern Unk80101DF0Record *D_800A370C;\n"
BB2_ADD = ("extern Unk80101DF0Record *D_800A3708; /* defined in main/32D04.c */\n"
           "extern s32 D_800A3924;               /* tentative definitions in main/28514.c and main/memcard.c */\n"
           "extern void func_800520B8(s32, s32, s32);\n")

def drop(t, line, n):
    ls = t.split(NL)
    hit = [i for i, l in enumerate(ls) if l.rstrip() == line]   # file scope only: block-scope copies stay
    assert len(hit) == n, (line, len(hit), n)
    return NL.join(l for i, l in enumerate(ls) if i not in set(hit))

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {}
    for p, line, n in DROP:
        if p not in out:
            out[p] = show(p)
        out[p] = drop(out[p], line, n)
    # the banner left with nothing under it
    a = "/* Declarations from the file this TU was split from (text1b.c). */\n\n/* Q65: this file's statics"
    assert out["src/main/51268.c"].count(a) == 1
    out["src/main/51268.c"] = out["src/main/51268.c"].replace(a, "/* Q65: this file's statics")
    # 9F9C func_8001C820 called the K&R-declared rand with an argument; rand takes none (libc2/rand.c), and the
    # call without it is byte-identical
    a = "        if (rand(0x56) & 1) {\n"
    assert out["src/main/9F9C.c"].count(a) == 1
    out["src/main/9F9C.c"] = out["src/main/9F9C.c"].replace(a, "        if (rand() & 1) {\n")
    for p, a, b in ADD:
        t = out.get(p) or show(p)
        assert t.count(a) == 1, (p, a)
        out[p] = t.replace(a, b)
    t = show("include/bb2.h")
    assert t.count(BB2_ANCHOR) == 1
    out["include/bb2.h"] = t.replace(BB2_ANCHOR, BB2_ANCHOR + BB2_ADD)
    for p, x in out.items():
        open(OUT + os.path.basename(p), "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fdecl1")
