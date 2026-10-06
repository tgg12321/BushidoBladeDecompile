#!/usr/bin/env python3
# The OT family: g_gpu_ot_ptr is u32 * (PsyQ's OT element type; P7c made the second OT u32 *), reversing
# P5's u8 * choice on measured evidence: every `g_gpu_ot_ptr + n * 4` / `+ (n << 2)` / `+ 0xNN` byte offset
# becomes the element offset `+ n` (PsyQ's `ot + n`), the (u32 *) views and (u8 *) back-assignments go.
# func_80074E08's byte-offset local keeps (u8 *) views (as an element offset: score 22).
# usage: fot1.py   writes tmp/p2/fot1/ (scratch only); BASE_REV env (default 9462a0204)
import os, re, subprocess, sys
NL = chr(10)
OUT = "tmp/p2/fot1/"
BASE_REV = os.environ.get("BASE_REV", "9462a0204")
TUS = {"25C38": "src/main/25C38.c", "3AB48": "src/main/3AB48.c", "51268": "src/main/51268.c",
       "5ED34": "src/main/5ED34.c", "63D2C": "src/main/63D2C.c", "64FD8": "src/main/64FD8.c",
       "6CF8": "src/main/6CF8.c", "368E4": "src/main/368E4.c"}

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a[:70]
        t = t.replace(a, b)
    return t

def idx(off):
    off = off.strip()
    m = re.fullmatch(r"\((.*)\)", off)
    if m and m.group(1).count("(") == m.group(1).count(")"):
        off = m.group(1).strip()
    if re.fullmatch(r"0x[0-9A-Fa-f]+|\d+", off):
        v = int(off, 0)
        assert v % 4 == 0, off
        return "0x%X" % (v // 4) if v // 4 >= 10 else str(v // 4)
    m = re.fullmatch(r"(.+) \* 4", off) or re.fullmatch(r"(.+) << 2", off)
    if m:
        e = m.group(1).strip()
        m2 = re.fullmatch(r"\((.*)\)", e)
        return m2.group(1) if m2 else e
    m = re.fullmatch(r"(.+) \* 4 - 4", off)
    if m:
        return "%s - 1" % m.group(1)
    m = re.fullmatch(r"(.+) \* 4 \+ 0x24", off)
    if m:
        return "%s + 9" % m.group(1)
    return None

LEFT = []

def conv_line(l, tu, n):
    if "g_gpu_ot_ptr" not in l or l.strip().startswith(("/*", "*")):
        return l
    l = l.replace("u8 * g_gpu_ot_ptr;", "u32 *g_gpu_ot_ptr;").replace("extern u8 *g_gpu_ot_ptr;", "extern u32 *g_gpu_ot_ptr;")
    l = l.replace("g_gpu_ot_ptr + (s32)(entry * 4)", "g_gpu_ot_ptr + entry")
    # views first: (u32 *)(g_gpu_ot_ptr + E * 4 | N) -> g_gpu_ot_ptr + E | N/4
    l = re.sub(r"\(u32 \*\)\(g_gpu_ot_ptr \+ ([^()]*\[[^()]*\][^()]*|[^()]*) \* 4\)", r"g_gpu_ot_ptr + \1", l)
    l = re.sub(r"\(u32 \*\)\(g_gpu_ot_ptr \+ (0x[0-9A-Fa-f]+|\d+)\)", lambda m: "g_gpu_ot_ptr + %d" % (int(m.group(1), 0) // 4), l)
    l = l.replace("(u32 *)(g_gpu_ot_ptr + entry)", "g_gpu_ot_ptr + entry")
    if "(u32 *)(g_gpu_ot_ptr +" in l:
        LEFT.append("%s:%d %s" % (tu, n, l.strip()))
    l = re.sub(r"\(u32 \*\)g_gpu_ot_ptr\b", "g_gpu_ot_ptr", l)
    l = re.sub(r"g_gpu_ot_ptr = \(u8 \*\)", "g_gpu_ot_ptr = ", l)
    def sub(m):
        if m.group(0).startswith("g_gpu_ot_ptr + ") and not re.search(r"\* 4|<< 2|0x[0-9A-Fa-f]+|\b\d+\b", m.group(1)):
            return m.group(0)          # already an element offset (converted view)
        i = idx(m.group(1))
        if i is None:
            LEFT.append("%s:%d %s" % (tu, n, l.strip()))
            return m.group(0)
        return "g_gpu_ot_ptr + " + i + m.group(2)
    if re.search(r"AddPrim\(\(?g_gpu_ot_ptr|ot = g_gpu_ot_ptr \+ ", l):
        l = re.sub(r"g_gpu_ot_ptr \+ ((?:[^,;()]|\([^()]*(?:\([^()]*\)[^()]*)*\))+?)(\s*[,;]|\)[,;)])", sub, l)
    return l

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {}
    for t, p in TUS.items():
        s = show(p)
        if t == "64FD8":   # func_80074E08 (rev-fot1 F1): ot_idx is the OT index in both blocks; `work` goes
            fx = s.index("void func_80074E08(Unk8006EACCRec *arg0, s32 arg1) {")
            fy = s.index("\n}\n", fx)
            b = rep(s[fx:fy], [
                ("    /* work holds two values (owner Ruling 11; the first under its Q20\n"
                 "       per-branch-constant clause): the backdrop TILE's OT index (0xE for\n"
                 "       player 1, 4 for player 0), then ot_idx * 4, the OT byte offset the last\n"
                 "       three AddPrim calls add. FAKE: one local for both; a second local for the byte\n"
                 "       offset: score 22. */\n    s32 work;\n", ""),
                ("        work = 0xE;\n    } else {\n        work = 4;\n    }\n    AddPrim(g_gpu_ot_ptr + work * 4 + 0x24, prim);",
                 "        ot_idx = 0xE;\n    } else {\n        ot_idx = 4;\n    }\n    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, prim);"),
                ("    if (arg1 != 0) {\n        ot_idx = 0xE;\n        rect_x = 0x14E;",
                 "    /* FAKE: ot_idx is set again (the same value) beside rect_x; set once, above: score 26. */\n"
                 "    if (arg1 != 0) {\n        ot_idx = 0xE;\n        rect_x = 0x14E;"),
                ("    work = ot_idx * 4;\n    AddPrim(g_gpu_ot_ptr + work, arg0->unk_04.unk_18);",
                 "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0->unk_04.unk_18);"),
                ("    AddPrim(g_gpu_ot_ptr + work + 0x24, arg0->unk_04.unk_1C);", "    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0->unk_04.unk_1C);"),
                ("    AddPrim(g_gpu_ot_ptr + work, arg0->unk_04.unk_1C);", "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0->unk_04.unk_1C);")])
            s = s[:fx] + b + s[fy:]
        if t == "3AB48":   # func_80060768 (rev-fot1 F2): end_off labelled (sweep measured 26)
            fx = s.index("func_80060768(")
            fy = s.index("\n}\n", fx)
            b = rep(s[fx:fy], [("    s32 end_off;\n",
                                "    /* FAKE: the returned size is the chunk's end minus its start; 0xAC8: score 26. */\n    s32 end_off;\n")])
            s = s[:fx] + b + s[fy:]
        if t == "51268":   # func_80069E18 (rev-fot1 F3): the block-scope copy of bb2.h's g_gpu_ot_ptr goes
            s = rep(s, [("void func_80069E18(s32 arg0) {\n    extern u8 *g_gpu_ot_ptr;\n", "void func_80069E18(s32 arg0) {\n")])
        s = NL.join(conv_line(l, t, n) for n, l in enumerate(s.split(NL), 1))
        if t == "25C38" and "    u8 *ot;\n" in s:   # before P7b: the local ot holder
            s = rep(s, [("    u8 *ot;\n", "    u32 *ot;\n")])
        if t == "3AB48" and "    v3 = (s32)g_gpu_ot_ptr;\n" in s:   # before w2 b7 (b7 types D_800A3808 u8 *)
            s = rep(s, [("    s32 v3;\n    s32 s0;\n    D_800A3820 = (s32)&D_80102C00;\n    v3 = (s32)g_gpu_ot_ptr;\n"
                         "    D_800A38D6 = D_800A38D6 + 1;\n    D_800A3808 = v3;\n    D_800A378C = (u32 *)(v3 + 0x10);\n",
                         "    u32 *v3;\n    s32 s0;\n    D_800A3820 = (s32)&D_80102C00;\n    v3 = g_gpu_ot_ptr;\n"
                         "    D_800A38D6 = D_800A38D6 + 1;\n    D_800A3808 = (s32)v3;\n    D_800A378C = v3 + 4;\n")])
        if t == "368E4" and "(u32 *)((s32)g_gpu_ot_ptr + 0x10)" in s:   # w2b5's spelling of func_80046BF4
            s = rep(s, [("D_800A378C = (u32 *)((s32)g_gpu_ot_ptr + 0x10);", "D_800A378C = g_gpu_ot_ptr + 4;")])
        elif t == "368E4" and "s32 old_ptr = (s32)g_gpu_ot_ptr;" in s:   # before w2b5
            s = rep(s, [("        s32 old_ptr = (s32)g_gpu_ot_ptr;\n", "        u32 *old_ptr = g_gpu_ot_ptr;\n"),
                        ("        D_800A3808 = old_ptr;\n", "        D_800A3808 = (s32)old_ptr;\n"),
                        ("        D_800A378C = (u32 *)(old_ptr + 0x10);\n", "        D_800A378C = old_ptr + 4;\n")])
        # B4 sweep of the moved bodies (measured; scratch tmp/p2/ot/)
        if t == "51268":   # func_80068D88: the idx_s / entry temps are byte-identical as the direct read
            s = rep(s, [("                s32 idx_s = *p_idx;\n                u32 entry = zbuf[idx_s];\n"
                         "                D_800A34E4 = g_gpu_ot_ptr + entry;\n",
                         "                D_800A34E4 = g_gpu_ot_ptr + zbuf[*p_idx];\n")])
        if t == "5ED34":
            i = s.index("POLY_G4 *func_80072CD4(s32 arg0, POLY_G4 *arg1) {")
            j = s.index("\n}\n", i)
            b = rep(s[i:j], [("    int red;\n\n", ""), ("        red = 0xFC;\n", "")])
            s = s[:i] + b.replace("arg1->r0 = red;", "arg1->r0 = 0xFC;").replace("arg1->r1 = red;", "arg1->r1 = 0xFC;") + s[j:]
            s = rep(s, [("    int fc_const;\n",
                         "    int fc_const; /* FAKE: constant holder -- 0xFC held from before the branch; the literal at each"
                         " store rematerialises it at the r3 store and swaps v0 / v1: score 5 */\n")])
        out[t + ".c"] = s
    b = show("include/bb2.h")
    b = rep(b, [("/* The current frame's ordering table as a byte address (OT entry n is g_gpu_ot_ptr + n * 4);\n",
                 "/* The current frame's ordering table (OT entry n is g_gpu_ot_ptr + n);\n"),
                ("extern u8 *g_gpu_ot_ptr;", "extern u32 *g_gpu_ot_ptr;")])
    if "extern u8 *D_800A3808;" in b:   # after w2 b7: the OT base copy follows the OT pointer's type
        b = rep(b, [("extern u8 *D_800A3808;", "extern u32 *D_800A3808;")])
    out["bb2.h"] = b
    out["game.h"] = rep(show("include/game.h"), [(" * (g_gpu_ot_ptr + ot_idx * 4);", " * (g_gpu_ot_ptr + ot_idx);")])
    for k, v in out.items():
        open(OUT + k, "w", encoding="utf-8", newline=NL).write(v)
    for l in LEFT:
        print("LEFT", l)
    return out

if __name__ == "__main__":
    write()
    print("wrote fot1")
