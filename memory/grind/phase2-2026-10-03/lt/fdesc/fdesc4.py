#!/usr/bin/env python3
# Descriptor unification, the parameter retype (on batch (ii-b), committed as 5180b9e9a), with FZZ batch 1 (../fzz/fzz1.py: 760D0,
# memcard.c, cdrom.c, pad.c / comb.c / libapi.h) and 32D04's packet-stream pairs folded in.
# func_8007352C / func_80073728 take
# Unk8007352CEnv *, func_8006E480 takes the Unk8009B0E0Record * header; every caller's `(s32)`
# conversion of the descriptor / header address goes.
# usage: fdesc4.py   writes tmp/p2/fdesc4/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import fdesc1 as D
sys.argv = _argv
OUT = "tmp/p2/fdesc4/"
sub1, fn = D.sub1, D.fn
TUS = ("63D2C", "64FD8", "5ED34", "3AB48", "51268")

BASE_REV = "5180b9e9a"   # batch (ii-b) as committed

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {t + ".c": show("src/main/%s.c" % t) for t in TUS}
    for f in ("game.h", "bb2.h"):
        out[f] = show("include/" + f)
    return out

def headers(b):
    b = sub1(b, "extern s32 func_8006E480(s32, s32);", "extern s32 func_8006E480(Unk8009B0E0Record *, s32);")
    b = sub1(b, "extern s32 func_8007352C(s32);", "extern s32 func_8007352C(Unk8007352CEnv *);")
    b = sub1(b, "extern s32 func_80073728(s32, s32);", "extern s32 func_80073728(Unk8007352CEnv *, s32);")
    return b

def defs(out):
    m = out["63D2C.c"]
    m = sub1(m, "s32 func_8007352C(s32 env_addr) {\n    Unk8007352CEnv *env = (Unk8007352CEnv *)env_addr;\n",
             "s32 func_8007352C(Unk8007352CEnv *env) {\n")
    m = sub1(m, "s32 func_80073728(s32 env_addr, s32 mode) {\n    Unk8007352CEnv *env = (Unk8007352CEnv *)env_addr;\n",
             "s32 func_80073728(Unk8007352CEnv *env, s32 mode) {\n")
    out["63D2C.c"] = m
    out["51268.c"] = sub1(out["51268.c"], """s32 func_8006E480(s32 a0_addr, s32 a1) {
    u8 *a0 = (u8 *)a0_addr;
    s32 v0 = a0[0] & 0xFE1F;
    s32 v1 = a0[1] << 7;
    return v0 + v1 + a1;
}""", """s32 func_8006E480(Unk8009B0E0Record *hdr, s32 a1) {
    return (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7) + a1;
}""")
    return out

CALL = re.compile(r"\b(func_8007352C|func_80073728)\(\(s32\)\s*(\(&\w+(?:\.\w+)?\)|&\w+(?:\.\w+)?|\w+)\s*([,)])")
HDR = re.compile(r"\bfunc_8006E480\(\(s32\)(&?[\w\.\->\[\]]+)\s*,")

def calls(t):
    def c(m):
        a = m.group(2)
        if a.startswith("(&") and a.endswith(")"):
            a = a[1:-1]
        return "%s(%s%s" % (m.group(1), a, m.group(3))
    t = CALL.sub(c, t)
    t = HDR.sub(lambda m: "func_8006E480(%s," % m.group(1), t)
    return t

# ---------------------------------------------------------------- folded in: FZZ batch 1 (../fzz/fzz1.py) and 32D04
sys.path.insert(0, os.path.join(HERE, "..", "fzz"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import fzz1 as Z
sys.argv = _argv
FZZ_FILES = {"760D0.c": "src/main/psxsdk/libsnd/760D0.c", "memcard.c": "src/main/memcard.c", "cdrom.c": "src/main/cdrom.c",
             "pad.c": "src/main/psxsdk/libapi/pad.c", "comb.c": "src/main/psxsdk/libcomb/comb.c",
             "libapi.h": "include/psxsdk/libapi.h", "32D04.c": "src/main/32D04.c"}

PAIRS = [("FT3", "0x20"), ("FT4", "0x28"), ("GT3", "0x28"), ("GT4", "0x34")]

def s32D04(t):
    def g(b):
        for k, off in PAIRS:   # the pair's second primitive: the cursor as a primitive pointer, plus one
            b = sub1(b, "gpu_OffsetTexPoly%s((u8 *)SCRATCH_PTR + %s, " % (k, off), "gpu_OffsetTexPoly%s((POLY_%s *)SCRATCH_PTR + 1, " % (k, k))
        return sub1(b, "                               live length seat count/base/kind in s3/s4/s5 as the target\n                               does. */\n",
                    "                               live length seat count/base/kind in s3/s4/s5 as the target\n                               does. Shared labels: score 109. */\n")
    t = fn(t, "func_80043454", g)
    def a2a0(b):
        b = sub1(b, "    u16 arg4_lo = *(u16 *)&arg4;\n", "    u16 arg4_lo = arg4;\n")
        # B4 sweep: byte-identical removals
        b = sub1(b, "        func_80043454((s16)arg1, (s16)arg2, (s16)arg3, (s16)arg4_lo);\n", "        func_80043454(arg1, arg2, arg3, arg4_lo);\n")
        b = sub1(b, "(*basePtr)[(s16)i]", "(*basePtr)[i]")
        b = sub1(b, "if ((s16)i < *cntPtr)", "if (i < *cntPtr)")
        # B4 sweep: labels
        b = sub1(b, "    s32 idx = arg0;\n    u16 *cnt_base = D_80103658;\n",
                 "    /* FAKE: the index (an s32 copy of arg0) and the count table's base in locals: arg0 / the\n"
                 "       table at the use, score 4 each. */\n    s32 idx = arg0;\n    u16 *cnt_base = D_80103658;\n")
        b = sub1(b, "        s32 **base_addr = D_80103608;\n", "        s32 **base_addr = D_80103608; /* FAKE: the base table in a local; at the use: score 4 */\n")
        b = sub1(b, "    loop:\n", "    /* FAKE: the loop goto-formed; as a do-while: score 21. */\n    loop:\n")
        b = sub1(b, "        u16 *cntPtr = countPtr; /* load-bearing named intermediate: countPtr's\n",
                 "        u16 *cntPtr = countPtr; /* FAKE (score 5 read through countPtr): countPtr's\n")
        return b
    t = fn(t, "func_800432A0", a2a0)
    def a670(b):
        b = sub1(b, "    v0 = *(u16 *)a0;\n", "    v0 = (u16)*a0;\n")
        b = sub1(b, "    {\n        s32 val = D_800A9CF8.unk6;\n        return a2 + val * 104;\n    }\n", "    return a2 + D_800A9CF8.unk6 * 104;\n")
        return sub1(b, "       LABEL_OUTSIDE_LOOP_P, suppressing the invert-jump peephole) */\n",
                    "       LABEL_OUTSIDE_LOOP_P, suppressing the invert-jump peephole); removed: score 4 */\n")
    t = fn(t, "func_80044670", a670)
    def a170(b):   # the variadic arguments walked as PsyQ's stdarg does (&last + 1)
        b = sub1(b, "    s32 *varptr;\n", "")
        b = sub1(b, "    s32 size;\n", "    s32 size;\n    s32 *ap;\n")
        b = sub1(b, "    count = *(s32 *)((s32)&a0 + 4);\n", "    ap = (s32 *)&a0 + 1;\n    count = *ap++;\n")
        b = sub1(b, "    varptr = (s32 *)((s32)&a0 + 8);\n", "")
        b = sub1(b, "            varptr++;\n            entry = *(varptr - 1);\n", "            ap++;\n            entry = *(ap - 1);\n")
        b = sub1(b, "    a0 = base + 1;\n", "    /* FAKE: a0 reused for the slot table (base + 1); its own local: score 34. */\n    a0 = base + 1;\n")
        return sub1(b, "            tbl = (s32 *)((entry * 4) + (s32)a0);\n",
                    "            /* FAKE: the slot address as an integer sum; &a0[entry]: score 3. */\n            tbl = (s32 *)((entry * 4) + (s32)a0);\n")
    return fn(t, "func_80044170", a170)

def fzz(out):
    for k, v in FZZ_FILES.items():
        out[k] = show(v)
    out["760D0.c"] = Z.s760(out["760D0.c"])
    out["libapi.h"] = Z.libapi(out["libapi.h"])
    out["memcard.c"] = Z.smemcard(out["memcard.c"])
    out["bb2.h"] = Z.sbb2(out["bb2.h"])
    out["cdrom.c"] = Z.scdrom(out["cdrom.c"])
    out["comb.c"] = Z.scomb(out["comb.c"])
    out["pad.c"] = Z.spad(out["pad.c"])
    out["32D04.c"] = s32D04(out["32D04.c"])
    return out

# ---------------------------------------------------------------- rev-fdesc4's fixes (measured forms)
VA = ("typedef char *va_list;\n"
      "#define va_start(ap, last) ((ap) = (va_list)(&(last) + 1))\n"
      "#define va_arg(ap, type) ((type *)(void *)(ap += 4))[-1]\n")

def rev1(out):
    def a670(b):   # the length read straight into unk2 (post-increment), in statement order
        return sub1(b, "    v0 = (u16)*a0;\n    a0++;\n    D_800A9CF8.unk8 = (s32)a0;\n    D_800A9CF8.unkC = a2;\n    D_800A9CF8.unk2 = v0;\n",
                    "    D_800A9CF8.unk2 = *a0++;\n    D_800A9CF8.unk8 = (s32)a0;\n    D_800A9CF8.unkC = a2;\n")
    def a454(b):   # the packed records advance in u16 units
        for k in (0x14, 0x18, 0x24, 0x2C):
            n = b.count("SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x%X);" % k)
            assert n >= 1, k
            b = b.replace("SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x%X);" % k, "SCRATCH_PTR += %d;" % (k // 2))
        return b
    def a170(b):   # the variadic walk spelled with 2B344's local stdarg macros
        b = sub1(b, "    s32 *ap;\n\n", "    va_list ap;\n\n")
        b = sub1(b, "    ap = (s32 *)&a0 + 1;\n    count = *ap++;\n", "    va_start(ap, a0);\n    count = va_arg(ap, s32);\n")
        b = sub1(b, "            ap++;\n            entry = *(ap - 1);\n", "            entry = va_arg(ap, s32);\n")
        return sub1(b, "its own local: score 34. */", "its own local: score 38. */")
    t = out["32D04.c"]
    t = fn(t, "func_80044670", a670)
    t = fn(t, "func_80043454", a454)
    i = t.index("s32 func_80044170(s32 *a0, ...) {")
    t = t[:i] + VA + t[i:]
    out["32D04.c"] = fn(t, "func_80044170", a170)
    out["memcard.c"] = fn(out["memcard.c"], "func_80037AA4", lambda b: Z.subs(b, [
        ("    s32 var_a0;\n    s32 var_v0;\n",
         "    /* FAKE: reused -- the byte total, then the block count; 0xF - (var_v0 >> sh) returned directly: score 15. */\n"
         "    s32 var_a0;\n"
         "    /* FAKE: reused -- each entry's size in the loop, then the rounded total; its own local for either value: score 2. */\n"
         "    s32 var_v0;\n"),
        ("(update_equiv_regs) then substitutes 13 and deletes the li: zero extra bytes. */",
         "(update_equiv_regs) then substitutes 13 and deletes the li: zero extra bytes;\n"
         "               literal 0xD: score 11. */")]))
    out["memcard.c"] = fn(out["memcard.c"], "func_80037B00", lambda b: sub1(b,
        "            var_t0 = var_a3->name + 0x15;\n",
        "            /* the bound is 21 bytes: the compare runs one byte past name[20] into attr, as the target does */\n"
        "            var_t0 = var_a3->name + 0x15;\n"))
    out["cdrom.c"] = fn(out["cdrom.c"], "func_80037110", lambda b: Z.subs(b, [
        ("    s32 v0;\n", ""),
        ("    v0 = func_80036EA8(5, s0->unk_0);\n    v0 = cdrom_StartAudio(v0, s0->unk_1);\n    if (v0 != 0) {\n",
         "    if (cdrom_StartAudio(func_80036EA8(5, s0->unk_0), s0->unk_1) != 0) {\n"),
        ("            v0 = CdPosToInt(&g_cd_file_table[D_80101E58.rec.unk00].loc);\n            D_80101E58.rec.unk14 = v0 + s0->unk_4;\n",
         "            D_80101E58.rec.unk14 = CdPosToInt(&g_cd_file_table[D_80101E58.rec.unk00].loc) + s0->unk_4;\n")]))
    out["64FD8.c"] = fn(out["64FD8.c"], "func_80074D2C", lambda b: sub1(b, "unk_1C[(arg2 << 16) >> 16]", "unk_1C[(s16)arg2]"))
    out["5ED34.c"] = fn(out["5ED34.c"], "func_8006F528", lambda b: sub1(b,
        "    /* FAKE: the holder between the header's cells and s.table; stored directly: score 9. */\n",
        "    /* FAKE: the holder between the header's cells and s.table; stored directly at all five sites:\n"
        "       score 26 (one site at a time: 13 / 3 / 4 / 3 / 3). */\n"))
    return out

def write():
    os.makedirs(OUT, exist_ok=True)
    out = base()
    out["bb2.h"] = headers(out["bb2.h"])
    out = defs(out)
    for t in TUS:
        out[t + ".c"] = calls(out[t + ".c"])
    out["5ED34.c"] = fn(out["5ED34.c"], "func_80073200", lambda b: sub1(b, "             * `(s32)&s` argument set land 4th,", "             * `&s` argument set land 4th,"))
    # staged-read sweep (B4, single-use temps read ahead of an intervening store / call)
    out["5ED34.c"] = fn(out["5ED34.c"], "func_8006F528", lambda b: sub1(b,
        "    {\n        u8 *ot;\n\n        ot = g_gpu_ot_ptr + 0x38;\n        prim->x0 = 0x4C;\n        prim->y0 = 0x80;\n        prim->w = 0x215;\n        prim->h = 0x4C;\n        AddPrim(ot, prim);\n    }\n",
        "    prim->x0 = 0x4C;\n    prim->y0 = 0x80;\n    prim->w = 0x215;\n    prim->h = 0x4C;\n    AddPrim(g_gpu_ot_ptr + 0x38, prim);\n"))
    out["63D2C.c"] = fn(out["63D2C.c"], "func_8007352C", lambda b: sub1(b, "    s32 x0, y0, x1, y1;\n",
        "    /* FAKE: x1 computed ahead of y1 and read once in the bounds test; at its use: score 2. */\n    s32 x0, y0, x1, y1;\n"))
    out = fzz(out)
    out = rev1(out)
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    left = []
    for t in TUS:
        for l in out[t + ".c"].split(NL):
            if re.search(r"func_8007352C\(\(s32\)|func_80073728\(\(s32\)|func_8006E480\(\(s32\)", l):
                left.append("%s: %s" % (t, l.strip()))
    for l in left:
        print("LEFT", l)
    return out

if __name__ == "__main__":
    write()
    print("wrote fdesc4")
