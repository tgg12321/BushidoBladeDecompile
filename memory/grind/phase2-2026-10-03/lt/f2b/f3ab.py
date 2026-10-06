#!/usr/bin/env python3
# 3AB48 sound slots (on the 60-body batch as committed, ac9d98537): g_vab_rec_ptr[16] / g_vab_vb_sbaddr[16] are reached through their
# per-slot alias symbols (g_vab_rec_ptr_plus_0x4 / 0x8 / 0xC / 0x10 / 0x14 / 0x18 / 0x20 / 0x24 and the
# g_vab_vb_sbaddr ones) and byte views; every access becomes an index of the one array, and the
# alias rows leave named_syms.txt.
# usage: f3ab.py [opt=<name>,...]   writes tmp/p2/f3ab/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f2b as P
sys.argv = _argv
OUT = "tmp/p2/f3ab/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = P.sub1
fn = P.fn
ALIAS = {"0x4": 1, "0x8": 2, "0xC": 3, "0x10": 4, "0x14": 5, "0x18": 6, "0x20": 8, "0x24": 9}

def s3AB48(a):
    # alias extern declarations go
    a = re.sub(r"extern s32 g_vab_(rec_ptr|vb_sbaddr)_plus_0x[0-9A-F]+(\[\])?;\n", "", a)
    a = sub1(a, "    s2 = g_vab_vb_sbaddr_plus_0x4;\n    s1 = g_vab_rec_ptr_plus_0x4;\n",
             "    s2 = &g_vab_vb_sbaddr[1];\n    s1 = &g_vab_rec_ptr[1];\n")
    a = fn(a, "func_8005B72C", lambda b: sub1(b, "    s32 *s1;\n", "    s32 **s1;\n"))
    for k, n in ALIAS.items():
        a = a.replace("g_vab_rec_ptr_plus_%s[0]" % k, "g_vab_rec_ptr[%d]" % n)
        a = a.replace("g_vab_vb_sbaddr_plus_%s[0]" % k, "g_vab_vb_sbaddr[%d]" % n)
        a = re.sub(r"\bg_vab_rec_ptr_plus_%s\b" % k, "g_vab_rec_ptr[%d]" % n, a)
        a = re.sub(r"\bg_vab_vb_sbaddr_plus_%s\b" % k, "g_vab_vb_sbaddr[%d]" % n, a)
    assert "_plus_0x" not in a
    # whole-array walkers and byte views
    def init(b):
        b = sub1(b, "    s32 *p2;\n", "    s32 **p2;\n")
        return sub1(b, "    p2 = (s32 *)g_vab_rec_ptr;\n", "    p2 = g_vab_rec_ptr;\n")
    a = fn(a, "snd_Init", init)
    a = fn(a, "snd_Quit", lambda b: sub1(b, "    s32 *v1;\n", "    s32 **v1;\n"))
    a = sub1(a, "    *(s32*)((u8*)&g_vab_rec_ptr + (v * 4)) = 0;\n    *(s32*)((u8*)&g_vab_vb_sbaddr + (v * 4)) = 0;\n",
             "    g_vab_rec_ptr[v] = 0;\n    g_vab_vb_sbaddr[v] = 0;\n")
    a = sub1(a, "    u32 *s3 = g_vab_rec_ptr;\n    u32 *s2 = g_vab_vb_sbaddr;\n",
             "    s32 **s3 = g_vab_rec_ptr;\n    s32 *s2 = g_vab_vb_sbaddr;\n")
    def fake(b):
        if not OPT & {"keepbase", "pplus", "plain"}:
            b = sub1(b, "    u8 *base;\n", "    s32 **base;\n")
            b = sub1(b, "    base = (u8 *)&g_vab_rec_ptr;\n    p = (s32 **)(base + idx * 4);\n",
                     "    /* FAKE: the slot array's address is staged in base ahead of the slot index:\n"
                     "       written `&g_vab_rec_ptr[idx]`, the index's `sll s3,s2,2` rises above the\n"
                     "       array's lui / addiu (score 2). */\n"
                     "    base = g_vab_rec_ptr;\n    p = base + idx;\n")
        elif "plain" in OPT or "pplus" in OPT:
            b = sub1(b, "    u8 *base;\n", "")
            b = sub1(b, "    base = (u8 *)&g_vab_rec_ptr;\n    p = (s32 **)(base + idx * 4);\n",
                     "    p = &g_vab_rec_ptr[idx];\n" if "pplus" not in OPT else "    p = g_vab_rec_ptr + idx;\n")
        b = sub1(b, "        *(s32 *)((u8 *)vv + 4) = *(s32 *)((u8 *)vv + 4) + arg0;\n", "        vv[1] = vv[1] + arg0;\n")
        b = sub1(b, "SsVabFakeHead(*(s32 *)((u8 *)*p + 4), idx, *(s32 *)((u8 *)&g_vab_vb_sbaddr + idx * 4));",
                 "SsVabFakeHead((*p)[1], idx, g_vab_vb_sbaddr[idx]);")
        return b
    if "nofake" not in OPT:
        a = fn(a, "snd_VabFakeOpen", fake)
    return a

HDR = """/* The stage data a stage loads (func_800469C4, or the buffer func_80054604 is handed; Unk800EFAE8Ctrl
   .unk2C): its header holds file-relative offsets of the parts func_80054604 / func_8005490C use. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10[2];
    s32 unk18[2];
} Unk800469C4Hdr;

"""

def stagehdr(a, g):
    H = "((Unk800469C4Hdr *)s->unk2C)"
    a = sub1(a, "*(s32 *)(s->unk2C + 0xC)", H + "->unkC")
    a = sub1(a, "*(s32 *)(s->unk2C + 0x10)", H + "->unk10[0]")
    a = sub1(a, "*(s32 *)(s->unk2C + 0x14)", H + "->unk10[1]")
    a = sub1(a, "*(s32 *)(s->unk2C + 0x18)", H + "->unk18[0]")
    a = sub1(a, "*(s32 *)(s->unk2C + 0x1C)", H + "->unk18[1]")
    a = sub1(a, "s->unk4 = *(s32 *)(*(s32 *)(p + 4) + p);", "s->unk4 = *(s32 *)(((Unk800469C4Hdr *)p)->unk4 + p);")
    a = sub1(a, "s->unk2 = *(u16 *)(*(s32 *)(p + 8) + p);", "s->unk2 = *(u16 *)(((Unk800469C4Hdr *)p)->unk8 + p);")
    anchor = "typedef struct {\n    /* 0x00 */ s16 unk0;    /* phase (func_8005490C: -1 = done, 0 = init) */"
    g = sub1(g, anchor, HDR + anchor)
    return a, g

def syms(n):
    out = []
    for l in n.split(NL):
        if re.match(r"g_vab_(rec_ptr|vb_sbaddr)_plus_0x[0-9A-F]+ ", l):
            continue
        out.append(l)
    return NL.join(out)

BASE_REV = "ac9d98537"   # the 60-body batch as committed (equal to f2b.py's output)

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {}
    for f in ("51268", "64FD8", "3AB48", "5ED34", "25788", "2B344", "63D2C"):
        out[f + ".c"] = show("src/main/%s.c" % f)
    for f in ("game.h", "bb2.h"):
        out[f] = show("include/" + f)
    out["undefined_syms_auto.txt"] = show("undefined_syms_auto.txt")
    return out

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = base()
    out["3AB48.c"] = s3AB48(out["3AB48.c"])
    if "nohdr" not in OPT:
        out["3AB48.c"], out["game.h"] = stagehdr(out["3AB48.c"], out["game.h"])
    out["named_syms.txt"] = syms(show("named_syms.txt"))
    out["undefined_syms_auto.txt"] = syms(out["undefined_syms_auto.txt"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f3ab", sorted(OPT))
