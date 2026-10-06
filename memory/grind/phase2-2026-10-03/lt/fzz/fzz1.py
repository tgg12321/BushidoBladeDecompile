#!/usr/bin/env python3
# FZZ in the unowned TUs, batch 1: 760D0 (the SpuSetVoiceAttr record), memcard.c (PsyQ's DIRENTRY),
# cdrom.c (the 8-byte audio-cue rows at D_8008F13C), pad.c (the SIO register block, shared with
# libcomb's SioRegs).
# usage: fzz1.py [opt=<name>,...]   writes tmp/p2/fzz1/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "fdesc"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import fdesc1 as D
sys.argv = _argv
OUT = "tmp/p2/fzz1/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1, fn = D.sub1, D.fn
BASE_REV = "HEAD"
FILES = {"760D0.c": "src/main/psxsdk/libsnd/760D0.c", "memcard.c": "src/main/memcard.c", "cdrom.c": "src/main/cdrom.c",
         "pad.c": "src/main/psxsdk/libapi/pad.c", "comb.c": "src/main/psxsdk/libcomb/comb.c",
         "libapi.h": "include/psxsdk/libapi.h", "bb2.h": "include/bb2.h", "game.h": "include/game.h"}

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True, encoding="utf-8").stdout

def subs(b, pairs):
    for p in pairs:
        if len(p) == 3 and p[2] is None:
            assert p[0] in b, p[0][:70]
            b = b.replace(p[0], p[1])
        else:
            b = sub1(b, p[0], p[1])
    return b

# ---------------------------------------------------------------- 760D0: SpuVoiceAttr
def s760(t):
    return fn(t, "func_800858D0", lambda b: subs(b, [
        ("    s32 buf[16];\n", "    SpuVoiceAttr attr;\n"),
        ("    buf[1] = 0x60093;\n", "    attr.mask = 0x60093;\n"),
        ("    *(s16 *)((u8 *)buf + 0x14) = 0x1000;\n", "    attr.pitch = 0x1000;\n"),
        ("    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;\n", "    attr.addr = 0x1000;\n"),
        ("    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;\n", "    attr.adsr1 = 0x80FF;\n"),
        ("    *(s16 *)((u8 *)buf + 0x08) = 0;\n", "    attr.volume.left = 0;\n"),
        ("    *(s16 *)((u8 *)buf + 0x0A) = 0;\n", "    attr.volume.right = 0;\n"),
        ("    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;\n", "    attr.adsr2 = 0x4000;\n"),
        ("            buf[0] = 1 << var_s0;\n            func_8008B488(buf);\n", "            attr.voice = 1 << var_s0;\n            func_8008B488(&attr);\n")]))

# ---------------------------------------------------------------- memcard.c: DIRENTRY
DIRENT = """/* PsyQ's directory entry (sys/file.h), what firstfile / nextfile fill (sizeof = 0x28). */
struct DIRENTRY {
    char name[20];
    s32 attr;
    s32 size;
    struct DIRENTRY *next;
    s32 head;
    char system[4];
};

"""

def libapi(h):
    a = "extern s32 firstfile(s32 *, s32 *); /* PsyQ: struct DIRENTRY *firstfile(char *, struct DIRENTRY *) */\n"
    h = sub1(h, a, "extern struct DIRENTRY *firstfile(s32 *, struct DIRENTRY *); /* PsyQ: struct DIRENTRY *firstfile(char *, struct DIRENTRY *) */\n")
    h = sub1(h, "extern s32 nextfile(s32 *);     /* PsyQ: struct DIRENTRY *nextfile(struct DIRENTRY *) */\n",
             "extern struct DIRENTRY *nextfile(struct DIRENTRY *);\n")
    i = h.index("extern ")
    i = h.rindex("\n\n", 0, i) + 2
    return h[:i] + DIRENT + SIOREGS + h[i:]

def smemcard(t):
    t = sub1(t, "extern s32 g_memcard_file_list;\n", "extern struct DIRENTRY g_memcard_file_list[];\n")
    t = fn(t, "memcard_CountFiles", lambda b: subs(b, [
        ("    s32 *var_s0;\n", "    struct DIRENTRY *var_s0;\n"),
        ("    var_s0 = (s32 *)&g_memcard_file_list;\n", "    var_s0 = g_memcard_file_list;\n"),
        ("            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);\n", "            var_s0++;\n")]))
    t = fn(t, "func_80037AA4", lambda b: subs(b, [
        ("    s8 *var_v1;\n", "    struct DIRENTRY *var_v1;\n"),
        ("        var_v1 = (s8 *)&g_memcard_file_list;\n", "        var_v1 = g_memcard_file_list;\n"),
        ("            var_v0 = *(s32 *)(var_v1 + 0x18);\n", "            var_v0 = var_v1->size;\n"),
        ("            var_v1 += 0x28;\n", "            var_v1++;\n")]))
    t = fn(t, "func_80037B00", lambda b: subs(b, [
        ("    s8 *var_a3;\n", "    struct DIRENTRY *var_a3;\n"),
        ("        var_a3 = (s8 *)&g_memcard_file_list;\n", "        var_a3 = g_memcard_file_list;\n"),
        ("            var_a1 = var_a3;\n", "            var_a1 = var_a3->name;\n"),
        ("            var_t0 = var_a3 + 0x15;\n", "            var_t0 = var_a3->name + 0x15;\n"),
        ("            var_a3 += 0x28;\n", "            var_a3++;\n")]))
    return t

# ---------------------------------------------------------------- cdrom.c: D_8008F13C rows
CUE = """/* 0x8008F13C: twelve 8-byte rows func_80037110 reads by index (asm/data 0x8008F13C..0x8008F19B):
 * unk_0, the group-5 file func_80036EA8 resolves; unk_1, cdrom_StartAudio's second argument;
 * unk_4, an offset added to the file's start sector (-1: none). */
typedef struct {
    u8 unk_0;
    u8 unk_1;
    u16 unk_2;
    s32 unk_4;
} Unk8008F13CRow;
"""

def sbb2(b):
    i = b.index("extern u8 D_8008F13C;\n")
    return b[:i] + CUE + "extern Unk8008F13CRow D_8008F13C[12];\n" + b[i + len("extern u8 D_8008F13C;\n"):]

def scdrom(t):
    return fn(t, "func_80037110", lambda b: subs(b, [
        ("    u8 *s0 = (u8 *)&D_8008F13C + (arg0 << 3);\n", "    Unk8008F13CRow *s0 = &D_8008F13C[arg0];\n"),
        ("    v0 = func_80036EA8(5, s0[0]);\n    v0 = cdrom_StartAudio(v0, s0[1]);\n", "    v0 = func_80036EA8(5, s0->unk_0);\n    v0 = cdrom_StartAudio(v0, s0->unk_1);\n"),
        ("        if (*(s32 *)(s0 + 4) != -1) {\n", "        if (s0->unk_4 != -1) {\n"),
        ("            D_80101E58.rec.unk14 = v0 + *(s32 *)(s0 + 4);\n", "            D_80101E58.rec.unk14 = v0 + s0->unk_4;\n")]))

# ---------------------------------------------------------------- pad.c / comb.c: SioRegs
SIOREGS = """/* An SIO port's registers (hardware I/O, volatile at the use: mmio-volatile-type-level): port 0
 * (controllers / memory cards) at 0x1F801040, D_8009BD84 in pad.c; port 1 (link cable) at
 * 0x1F801050, libcomb's D_800A3044. */
typedef struct {
    u8 data;
    u8 unk1[3];
    u16 stat;
    u16 unk6;
    u16 mode;
    u16 ctrl;
    u16 misc;
    u16 baud;
} SioRegs;

"""

def scomb(t):
    i = t.index("/* SIO port registers (0x1F801050, hardware I/O: volatile is type-level). */\n")
    j = t.index("} SioRegs;\n") + len("} SioRegs;\n")
    return t[:i] + t[j:].lstrip(NL)

def spad(t):
    t = sub1(t, "extern s32 D_8009BD84;\n", "extern volatile SioRegs *D_8009BD84; /* SIO port 0, 0x1F801040 (asm/data/7D920.data.s:23915) */\n")
    t = sub1(t, "extern s32 D_8009BD88;\n", "extern volatile u32 *D_8009BD88; /* I_STAT (I_MASK at [1]), 0x1F801070 (asm/data/7D920.data.s:23921) */\n")
    t = fn(t, "_IsVSync", lambda b: sub1(b, "    s32 *p = (s32 *)D_8009BD88;\n", "    volatile u32 *p = D_8009BD88;\n"))
    return fn(t, "_Pad1", lambda b: sub1(b, "    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;\n", "    D_8009BD84->ctrl = 0;\n"))

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {k: show(v) for k, v in FILES.items()}
    out["760D0.c"] = s760(out["760D0.c"])
    out["libapi.h"] = libapi(out["libapi.h"])
    out["memcard.c"] = smemcard(out["memcard.c"])
    out["bb2.h"] = sbb2(out["bb2.h"])
    out["cdrom.c"] = scdrom(out["cdrom.c"])
    out["comb.c"] = scomb(out["comb.c"])
    out["pad.c"] = spad(out["pad.c"])
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fzz1", sorted(OPT))
