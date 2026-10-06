#!/usr/bin/env python3
# Descriptor unification, batch (ii-a) (on batch (i), committed as 1b9a7a3ee): 3AB48's local
# descriptor types (Env5C8A8, Env5E54C, Env5D814, S5E098, S5F1C8, SFC9C, S60C8, S414, S544) and the
# shared S46C become Unk8007352CEnv; Unk8009B398Record (the same 12-byte sheet header) merges into
# Unk8009B0E0Record; the sheet / cell data the 3AB48 walkers' callers reach through s32 externs is
# typed. 51268 is batch (ii-b).
# usage: fdesc2.py [opt=<name>,...]   writes tmp/p2/fdesc2/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import fdesc1 as D
sys.argv = _argv
OUT = "tmp/p2/fdesc2/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1, fn, retype, drop = D.sub1, D.fn, D.retype, D.drop
ENV = "Unk8007352CEnv"
H, C = "Unk8009B0E0Record", "Unk8009B400Record"

BASE_REV = "1b9a7a3ee"

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {"3AB48.c": show("src/main/3AB48.c")}
    for f in ("game.h", "bb2.h"):
        out[f] = show("include/" + f)
    out["undefined_syms_auto.txt"] = show("undefined_syms_auto.txt")
    return out

# ---------------------------------------------------------------- headers
def headers(g, b):
    # Unk8009B398Record is the sheet header: its evidence comment moves to D_8009B398's extern
    i = g.index("/* 0x8009B398: table of 4 twelve-byte records")
    j = g.index("} Unk8009B398Record;\n", i) + len("} Unk8009B398Record;\n")
    blk = g[i:j]
    k = blk.index("typedef struct {")
    doc = blk[:k].replace("/* 0x8009B398: table of 4 twelve-byte records", "/* 0x8009B398: table of 4 sprite-sheet headers")
    g = g[:i] + g[j:].lstrip(NL)
    b = sub1(b, "extern Unk8009B398Record D_8009B398[4];\n", doc + "extern Unk8009B0E0Record D_8009B398[4];\n")
    b = sub1(b, " * D_8009B2E0 header records has cell count 1.", " * D_8009B2C8[row][2..4] headers has cell count 1.")
    # S46C (its last users, 5ED34's and 3AB48's, take Unk8007352CEnv)
    i = g.index("typedef struct {\n    void *p0;\n    s32 *p1;\n    s32 pad08;\n    s32 ret;")
    j = g.index("} S46C;\n", i) + len("} S46C;\n")
    g = g[:i] + g[j:].lstrip(NL)
    return g, b

# ---------------------------------------------------------------- 3AB48 data
DATA = [
    ("extern s32 D_8009B2C8;\nextern s32 D_8009B340;\nextern s32 D_8009B358;\n",
     "/* 0x8009B2C8: two rows of five sprite-sheet headers (func_8005D46C draws headers 0 / 1 of row\n"
     " * idx with the cells at D_8009B340 / D_8009B358; func_8005D554 headers 2 and 3 / 4 with\n"
     " * D_8009B388's), then the cells: three under header 0 (count 3), six under header 1 (count 6). */\n"
     "extern Unk8009B0E0Record D_8009B2C8[2][5];\nextern Unk8009B400Record D_8009B340[3];\nextern Unk8009B400Record D_8009B358[6];\n"),
    ("extern u8 D_8009B2E0[];\n", ""),
    ("extern s32 D_8009B488;\nextern u8 D_8009B48E;\n", "extern Unk8009B400Record D_8009B488;\nextern u8 D_8009B48E;\n"),
    ("extern s32 D_8009B610;\nextern s32 D_8009B634;\nextern s32 D_8009B63C;\nextern s32 D_8009B660;\nextern s32 D_8009B670;\nextern s32 D_8009B678;\n",
     "/* func_8005FA98's sheets: three headers at 0x8009B610 (the second draw, header arg0, its one cell at\n"
     " * 0x8009B634) and three at 0x8009B63C (counts 2 / 1 / 4) whose cells follow at 0x8009B660 (0-1,\n"
     " * 2, 3-6). */\n"
     "extern Unk8009B0E0Record D_8009B610[3];\nextern Unk8009B400Record D_8009B634;\nextern Unk8009B0E0Record D_8009B63C[3];\nextern Unk8009B400Record D_8009B660[7];\n"),
    ("extern s32 D_8009B698;\nextern s32 D_8009B6B0;\n",
     "extern Unk8009B0E0Record D_8009B698[2];\nextern Unk8009B400Record D_8009B6B0[8];\n"),
    ("extern s32 D_8009B6F0;\nextern s32 D_8009B6FC;\nextern s32 D_8009B708[10][2];\nextern s32 D_8009B758;\n",
     "/* func_800600C8's sheets: the frame header (3 cells, D_8009B758) and the digit header (1 cell;\n"
     " * one of the ten digit cells D_8009B708). */\n"
     "extern Unk8009B0E0Record D_8009B6F0[2];\nextern Unk8009B400Record D_8009B708[10];\nextern Unk8009B400Record D_8009B758[3];\n"),
    ("extern s32 D_8009B7AC;\nextern s32 D_8009B7B8;\nextern s32 D_8009B7C4;\n", "extern Unk8009B0E0Record D_8009B7AC[3];\n"),
]
RETIRED = ["D_8009B48E", "D_8009B7B8", "D_8009B7C4", "D_8009B2E0"]

S46C_MAP = D.S46C_MAP
ENVOUT = {"out": "sprt_out", "pad0C": "ft4_out", "pad20": "scale_x", "pad24": "scale_y"}
SEXT = {"p0": "header", "p1": "table", "unk_08": "sprt_out", "pad0C": "ft4_out", "zero10": "semi",
        "arg3": "ot_idx", "arg2": "ot_idx", "unk_14": "ot_idx", "unk_18": "x", "unk_1C": "y", "zero1C": "y",
        "pad20": "scale_x", "pad24": "scale_y", "byte28": "has_color", "byte29": "col_r", "byte2A": "col_g", "byte2B": "col_b"}
S414_MAP = {"unk_04": "table", "unk_08": "sprt_out", "pad0C": "ft4_out", "zero10": "semi", "arg2_field": "ot_idx",
            "pad20": "scale_x", "pad24": "scale_y", "byte28": "has_color"}
S544_MAP = {"unk_00": "header", "unk_04": "table", "unk_08": "sprt_out", "unk_0C": "ft4_out", "zero10": "semi",
            "unk_14": "ot_idx", "unk_18": "x", "unk_1C": "y", "unk_20": "scale_x", "unk_24": "scale_y",
            "byte28": "has_color", "byte29": "col_r", "byte2A": "col_g", "byte2B": "col_b"}

def digits(b, oldtype, n):
    """the descriptor-plus-digits local `s` (S5E098 / S5F1C8 / S60C8): the descriptor, and the digit
    array as its own local (measured identical)"""
    b = re.sub(r"\b%s s;" % oldtype, "Unk8007352CEnv s;\n    s16 d[%d];" % n, b)
    def r(m):
        mem = m.group(1)
        if mem == "d":
            return "d"
        return "s." + SEXT.get(mem, mem)
    return re.sub(r"\bs\.(\w+)", r, b)

def s3AB48(t):
    t = t.replace("Unk8009B398Record", H)
    for a, b in DATA:
        t = sub1(t, a, b)
    for name in ("Env5C8A8", "Env5E54C", "Env5D814", "SFC9C", "S414", "S544"):
        i = t.index("} %s;\n" % name)
        i = t.rindex("typedef struct {", 0, i)
        c = t.rfind("*/\n", 0, i)
        if c != -1 and t[c + 3:i].strip() == "":
            o = t.rindex("/*", 0, c)
            if t[o - 1] == NL:   # a comment block of its own, not a trailing line comment
                i = o
        t = t[:i] + t[t.index("} %s;\n" % name) + len("} %s;\n" % name):]
    for name in ("S5E098", "S5F1C8", "S60C8"):
        i = t.rindex("typedef struct {", 0, t.index("} %s;\n" % name))
        j = t.index("} %s;\n" % name) + len("} %s;\n" % name)
        t = t[:i] + t[j:]

    t = fn(t, "func_8005C8A8", lambda b: retype(b, "s", "Env5C8A8", ENVOUT))
    def d46c(b):
        b = retype(b, "s", "S46C", S46C_MAP)
        b = sub1(b, "    s.header = (void *)((u8 *)(&D_8009B2C8) + stride);\n", "    s.header = &D_8009B2C8[idx][0];\n")
        b = sub1(b, "    s.header = (void *)(((u8 *)(&D_8009B2C8) + stride) + 0xC);\n", "    s.header = &D_8009B2C8[idx][1];\n")
        b = sub1(b, "    s.table = &D_8009B340;\n", "    s.table = D_8009B340;\n")
        b = sub1(b, "    s.table = &D_8009B358;\n", "    s.table = D_8009B358;\n")
        if "keepstride" not in OPT:
            b = sub1(b, "    s32 stride;\n", "")
            b = sub1(b, "    stride = idx * 0x3C;\n", "")
        return b
    t = fn(t, "func_8005D46C", d46c)
    def d554(b):
        b = retype(b, "s", "Env5E54C", {})
        b = sub1(b, "    u8 *hdr0;     /* FAKE: pointer alias of D_8009B2E0; direct use does not match */\n",
                 "    u8 *hdr0;     /* FAKE: pointer alias of D_8009B2C8[0][2]; direct use does not match */\n")
        b = sub1(b, "        hdr0 = D_8009B2E0;\n", "        hdr0 = (u8 *)&D_8009B2C8[0][2];\n")
        return b
    t = fn(t, "func_8005D554", d554)
    t = fn(t, "func_8005D814", lambda b: retype(b, "s", "Env5D814", ENVOUT))
    def e098(b):
        return digits(b, "S5E098", 2)
    t = fn(t, "func_8005E098", e098)
    t = fn(t, "func_8005E54C", lambda b: retype(b, "s", "Env5E54C", {}))
    t = fn(t, "func_8005F1C8", lambda b: digits(b, "S5F1C8", 3))
    def fa98(b):
        b = retype(b, "s", "S46C", S46C_MAP)
        b = sub1(b, "    s.header = (void *)((u8 *)(&D_8009B63C) + (arg0 * 0xC));\n", "    s.header = &D_8009B63C[arg0];\n")
        b = sub1(b, "        s.table = &D_8009B660;\n", "        s.table = &D_8009B660[0];\n")
        b = sub1(b, "        s.table = &D_8009B670;\n", "        s.table = &D_8009B660[2];\n")
        b = sub1(b, "        s.table = &D_8009B678;\n", "        s.table = &D_8009B660[3];\n")
        b = sub1(b, "    s.header = (void *)((u8 *)(&D_8009B610) + (arg0 * 0xC));\n", "    s.header = &D_8009B610[arg0];\n")
        return b
    t = fn(t, "func_8005FA98", fa98)
    def fc9c(b):
        b = retype(b, "s", "SFC9C", SEXT)
        b = sub1(b, "    s.table = &D_8009B6B0;\n", "    s.table = D_8009B6B0;\n")
        b = sub1(b, "            s.header = (s32 *)((u8 *)&D_8009B698 + i * 12);\n", "            s.header = &D_8009B698[i];\n")
        b = sub1(b, "func_8006E480((s32)&D_8009B698, 0x20)", "func_8006E480((s32)D_8009B698, 0x20)")
        return b
    t = fn(t, "func_8005FC9C", fc9c)
    def c8(b):
        b = digits(b, "S60C8", 2)
        b = sub1(b, "    s.header = &D_8009B6F0;\n", "    s.header = &D_8009B6F0[0];\n")
        b = sub1(b, "    s.table = &D_8009B758;\n", "    s.table = D_8009B758;\n")
        b = sub1(b, "    s.header = &D_8009B6FC;\n", "    s.header = &D_8009B6F0[1];\n")
        b = sub1(b, "    s.table = D_8009B708[d[i]];\n", "    s.table = &D_8009B708[d[i]];\n")
        b = sub1(b, "func_8006E480((s32)&D_8009B6F0, 0)", "func_8006E480((s32)D_8009B6F0, 0)")
        return b
    t = fn(t, "func_800600C8", c8)
    def c414(b):
        b = retype(b, "s", "S414", S414_MAP)
        b = sub1(b, "        s.header = &D_8009B7AC;\n", "        s.header = &D_8009B7AC[0];\n")
        b = sub1(b, "        s.header = &D_8009B7B8;\n", "        s.header = &D_8009B7AC[1];\n")
        b = sub1(b, "        s.header = &D_8009B7C4;\n", "        s.header = &D_8009B7AC[2];\n")
        return b
    t = fn(t, "func_80060414", c414)
    t = fn(t, "func_80060544", lambda b: sub1(retype(b, "s", "S544", S544_MAP), "    s.table = &D_8009B820;\n", "    s.table = D_8009B820;\n"))
    return t

# ---------------------------------------------------------------- carried-construct sweep (checklist B4)
# byte-identical removals (measured alone and together)
SWEEP = [
    ("func_800600C8", [("    v = ((s16)arg0) / 10;\n    d[1] = v % 10;\n", "    d[1] = ((s16)arg0) / 10 % 10;\n"),
        ("    i = 0;\nloop_60C8:\n", "    for (i = 0; i < 2; i++) {\n"),
        ("    s.table = &D_8009B708[d[i]];\n    if (arg0 < 0xA) {\n        s.x = 0x64;\n    } else {\n        s.x = (((1 - i) << 2) << 3) + 0x54;\n    }\n    s.sprt_out = cur;\n    cur = func_8007352C((s32)&s);\n",
         "        s.table = &D_8009B708[d[i]];\n        if (arg0 < 0xA) {\n            s.x = 0x64;\n        } else {\n            s.x = (((1 - i) << 2) << 3) + 0x54;\n        }\n        s.sprt_out = cur;\n        cur = func_8007352C((s32)&s);\n"),
        ("    if (d[1] != 0) {\n        i += 1;\n        if (i < 2) goto loop_60C8;\n    }\n", "        if (d[1] == 0) {\n            break;\n        }\n    }\n")]),
]

ENDOFF = "    /* FAKE: the returned size is the chunk's end minus its start; sizeof(%s): score %d. */\n"
LABELS = [
    ("func_8005D46C", "    s32 ret;\n", "    /* FAKE: holder of the first walk's cursor until the second descriptor is set; stored\n       directly: score 6. */\n    s32 ret;\n"),
    ("func_8005FA98", "    s32 ret;\n", "    /* FAKE: holder of the first walk's cursor until the second descriptor is set; stored\n       directly: score 11. */\n    s32 ret;\n"),
    ("func_8005FA98", "    s32 start = arg1;\n", "    /* FAKE: copy of arg1 for the first cursor; arg1 directly: score 2. */\n    s32 start = arg1;\n"),
    ("func_8005FA98", "    s32 end = arg1 + 0x190;\n", "    /* FAKE: the returned size as end minus start; 0x190: score 12. */\n    s32 end = arg1 + 0x190;\n"),
    ("func_8005FC9C", "    s32 end_off;\n", ENDOFF % ("Unk8005FC9CRec", 55) + "    s32 end_off;\n"),
    ("func_800600C8", "    s32 end_off = arg1", (ENDOFF % ("Unk800600C8Rec", 20)) + "    s32 end_off = arg1"),
    ("func_800600C8", "    v = arg0;\n", "    /* FAKE: dead stores (both digits are set again below): removed, score 25. */\n    v = arg0;\n"),
    ("func_80060414", "    s32 end_off;\n", ENDOFF % ("Unk80060414Rec", 22) + "    s32 end_off;\n"),
    ("func_80060544", "    s32 end_off;\n", ENDOFF % ("Unk80060544Rec", 25) + "    s32 end_off;\n"),
    ("func_80060544", "        if (i < 3) {\n            if (i > 0) {\n",
     "        /* FAKE: the per-i cell table through a goto chain; the if / else-if form: score 17. */\n        if (i < 3) {\n            if (i > 0) {\n"),
    ("func_8005D554", "the literal does not match */\n    s32 ot;", "the literal: score 15 */\n    s32 ot;"),
    ("func_8005D554", "    s32 ot;    /* FAKE: constant 1 in a local; the literal does not match */\n", "    s32 ot;    /* FAKE: constant 1 in a local; the literal: score 2 */\n"),
    ("func_8005D554", "direct use does not match */\n", "direct use: score 41 */\n"),
    ("func_8005D554", "/* FAKE: hdr0 + 0xC named; folding it does not match */\n", "/* FAKE: hdr0 + 0xC named; folded: score 2 */\n"),
    ("func_8005D554", "/* FAKE: hoisted row address; inlining it does not match */\n", "/* FAKE: hoisted row address; inlined: score 24 */\n"),
    ("func_8005D554", "/* FAKE: integer sum keeps `addu v0,s0,fp`; the pointer sum swaps it */\n", "/* FAKE: integer sum keeps `addu v0,s0,fp`; the pointer sum swaps it: score 1 */\n"),
]

def sweep_labels(t):
    for f, reps in SWEEP:
        def g(b, reps=reps):
            for old, new in reps:
                b = sub1(b, old, new)
            return b
        t = fn(t, f, g)
    for f, old, new in LABELS:
        t = fn(t, f, lambda b, old=old, new=new: sub1(b, old, new))
    return t


# ---------------------------------------------------------------- rev-fdesc2 fixes (measured by the reviewer)
CH = """        /* FAKE: the per-i cell table through a goto chain; the if / else-if form: score 17. */
        if (i < 3) {
            if (i > 0) {
                goto S800;
            }
            if (i == 0) {
                goto S7D8;
            }
            goto Skip;
        }
        if (i == 3) {
            goto Case3;
        }
        goto Skip;
    S7D8:
        s.table = D_8009B7D8;
        goto Skip;
    S800:
        s.table = D_8009B800;
        goto Skip;
    Case3:
        s.table = &D_8009B7D0;
        s.ft4_out = ft4;
        ft4 = func_80073728((s32)&s, 0);
    Skip:
"""
SW1 = """        switch (i) {
        case 0:
            s.table = D_8009B7D8;
            break;
        case 1:
        case 2:
            s.table = D_8009B800;
            break;
        case 3:
            s.table = &D_8009B7D0;
            s.ft4_out = ft4;
            ft4 = func_80073728((s32)&s, 0);
            break;
        }
"""
def rev1(t):
    def r(old, new):
        nonlocal t
        assert t.count(old) == 1, old[:80]
        t = t.replace(old, new)
    r(CH, SW1)
    r("    s32 i;\n    s16 v;\n\n    s.header = &D_8009B6F0[0];\n", "    s32 i;\n\n    s.header = &D_8009B6F0[0];\n")
    r("    /* FAKE: dead stores (both digits are set again below): removed, score 25. */\n    v = arg0;\n    s.header = &D_8009B6F0[1];\n    d[1] = v;\n    d[0] = v;\n    d[1] = ((s16)arg0) / 10 % 10;\n    d[0] = ((s16)arg0) % 10;\n",
      "    s.header = &D_8009B6F0[1];\n    d[0] = d[1] = arg0;\n    d[1] = d[1] / 10 % 10;\n    d[0] = d[0] % 10;\n")
    r("extern Unk8009B400Record D_8009B488;\nextern u8 D_8009B48E;\n", "extern Unk8009B400Record D_8009B488;\n")
    r("                D_8009B48E = 0x2D;\n", "                s.table->w = 0x2D;\n")
    r("                D_8009B48E = 0x3C;\n", "                s.table->w = 0x3C;\n")
    r("    /* FAKE: alias of s.table for the digit cell's x store; through s.table: score 68. */\n    Unk8009B400Record *p;\n", "")
    r("            p = &D_8009B400[d[i]];\n            s.table = p;\n            if (j != 0) {\n                p->x = 0x50;\n            } else {\n                p->x = 0x209;\n",
      "            s.table = &D_8009B400[d[i]];\n            if (j != 0) {\n                s.table->x = 0x50;\n            } else {\n                s.table->x = 0x209;\n")
    r("    u8 *hdr0;     /* FAKE: pointer alias of D_8009B2C8[0][2]; direct use: score 41 */\n    u8 *hdr1;     /* FAKE: hdr0 + 0xC named; folded: score 2 */\n    u8 *hdr1_row; /* FAKE: hoisted row address; inlined: score 24 */\n",
      "    Unk8009B0E0Record *hdr0;     /* FAKE: pointer alias of D_8009B2C8[0][2]; direct use: score 41 */\n    Unk8009B0E0Record *hdr1_row; /* FAKE: the row's header 3 in a local; at the use: score 36 */\n")
    r("        hdr0 = (u8 *)&D_8009B2C8[0][2];\n        hdr1 = hdr0 + 0xC;\n        hdr1_row = hdr1 + row_off;\n", "        hdr0 = &D_8009B2C8[0][2];\n        hdr1_row = &D_8009B2C8[arg1][3];\n")
    r("the pointer sum swaps it: score 1 */\n", "the byte-pointer sum swaps it: score 1 */\n")
    r("            s.header = (Unk8009B0E0Record *)(hdr1_row + (D_800A3418 & 1) * 0xC);\n", "            s.header = hdr1_row + (D_800A3418 & 1);\n")
    return t

def syms(u):
    out = []
    for l in u.split(NL):
        if any(l.startswith(r + " = ") for r in RETIRED):
            continue
        out.append(l)
    return NL.join(out)

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = base()
    out["game.h"], out["bb2.h"] = headers(out["game.h"], out["bb2.h"])
    out["3AB48.c"] = rev1(sweep_labels(s3AB48(out["3AB48.c"])))
    out["undefined_syms_auto.txt"] = syms(out["undefined_syms_auto.txt"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fdesc2", sorted(OPT))
