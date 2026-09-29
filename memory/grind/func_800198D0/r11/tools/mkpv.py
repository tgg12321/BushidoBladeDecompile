#!/usr/bin/env python3
"""mkpv.py LANDING OUTDIR -- generate Ruling 11 one-variable-per-value spellings,
ablations and structural respellings of func_800198D0 from the landing body.

Every variant differs from the landing ONLY in declarations and identifiers
(Ruling 11 (C)(2)); each new local is declared at the innermost block that
encloses all of its writes, unless the variant name says `fs` (function scope,
a structural respelling). Every edit is an exact-text replacement that must
hit the stated number of occurrences, so a drifted landing body fails loudly.
"""
import os, sys

LAND, OUT = sys.argv[1], sys.argv[2]
L = open(LAND, encoding="utf-8").read()
os.makedirs(OUT, exist_ok=True)


def sub(s, old, new, count=1, nth=None):
    n = s.count(old)
    if nth is not None:
        assert n >= nth, (old[:70], n, nth)
        k = -1
        for _ in range(nth):
            k = s.index(old, k + 1)
        return s[:k] + new + s[k + len(old):]
    assert n == count, (old[:70], n, count)
    return s.replace(old, new)


I8, I12, I16, I20, I24 = " " * 8, " " * 12, " " * 16, " " * 20, " " * 24
ELSE_TOP = "            slot = prev;\n        } else {\n"   # full-decode block start
DEC_TOP = "    for (; idx2 < sub; idx2++) {\n"                 # sub-frame loop body start
KF_FOR = "            for (idx = 0; idx < 63; idx++) {\n"      # keyframe loop head
C1_TOP = "                s32 nbits;\n"
C2_TOP = "            case 2: {\n"
C3_IF = "                    s32 nbits2;\n"
CH_TOP = "        for (ch = 0; ch < 63; ch++) {\n            s16 temp;\n"


# ---------------------------------------------------------------- idx
def pv_idx(s, fs=False):
    """kidx: its writes and reads are in the keyframe block -> declared there;
    col: its writes and reads are in the post-pass row-loop body -> declared
    there. fs=True (structural respelling): both at function scope."""
    s = sub(s, "    s32 idx;\n", ("    s32 col;\n    s32 kidx;\n" if fs else ""))
    if not fs:
        s = sub(s, ELSE_TOP, ELSE_TOP + "            s32 kidx;\n")
    s = sub(s, KF_FOR, "            for (kidx = 0; kidx < 63; kidx++) {\n")
    s = sub(s, "GETBITS(work[idx + 3], 12);", "GETBITS(work[kidx + 3], 12);")
    if fs:
        s = sub(s, "        for (idx = 0; idx < 3; idx++) {\n", "        for (col = 0; col < 3; col++) {\n")
    else:
        s = sub(s, "        for (idx = 0; idx < 3; idx++) {\n",
                "        s32 col;\n\n        for (col = 0; col < 3; col++) {\n")
    return s


# ---------------------------------------------------------------- idx2
def pv_idx2(s):
    s = sub(s, "    s32 idx2;\n", "    s32 step;\n    s32 row;\n")
    s = sub(s, "            idx2 = 0;\n            goto decode;", "            step = 0;\n            goto decode;")
    s = sub(s, "    idx2 = sub - 1;\n", "    step = sub - 1;\n")
    s = sub(s, DEC_TOP, "    for (; step < sub; step++) {\n")
    s = sub(s, "            if (idx2 == 0) {\n", "            if (step == 0) {\n")
    s = sub(s, "    for (idx2 = 0; idx2 < 2; idx2++) {\n", "    for (row = 0; row < 2; row++) {\n")
    return s


# ---------------------------------------------------------------- field
HDR = ("GETBITS(field, 1);\n{i}if (field) {{\n{i}    GETBITS(field, 16);\n{i}}}\n{i}work[{k}] = field;")


def hdr_old(ind, k):
    return HDR.format(i=ind, k=k)


def hdr_new(ind, k, nm):
    return hdr_old(ind, k).replace("field", nm)


KFLAG_OLD = "                GETBITS(field, 1);\n                if (field) {\n                    GETBITS(work["
LO_OLD = ["GETBITS(field, 4);", "temp = ((temp << 3) | (field & 7)) + 1;", "if (field & 8) {"]


def field_split(s, which, fs=False):
    """which: subset of h0..h5, kflag, lo -> own locals; the rest keep `field`."""
    decl_fs = []
    for k in range(3):
        nm = f"h{k}"
        if nm in which:
            s = sub(s, hdr_old(I12, k), hdr_new(I12, k, nm))
            if fs:
                decl_fs.append(nm)
            else:
                s = sub(s, ELSE_TOP, ELSE_TOP + f"            u32 {nm};\n")
    if "kflag" in which:
        if fs:
            s = sub(s, KFLAG_OLD, KFLAG_OLD.replace("field", "kflag"))
            decl_fs.append("kflag")
        else:
            s = sub(s, KFLAG_OLD, "                u32 kflag;\n\n" + KFLAG_OLD.replace("field", "kflag"))
    for k in range(3):
        nm = f"h{k + 3}"
        if nm in which:
            s = sub(s, hdr_old(I8, k), hdr_new(I8, k, nm))
            if fs:
                decl_fs.append(nm)
            else:
                s = sub(s, DEC_TOP, DEC_TOP + f"        u32 {nm};\n\n")
    if "lo" in which:
        for o in LO_OLD:
            s = sub(s, o, o.replace("field", "lo"))
        if fs:
            decl_fs.append("lo")
        else:
            s = sub(s, C3_IF, C3_IF + "                    u32 lo;\n")
    allv = {"h0", "h1", "h2", "h3", "h4", "h5", "kflag", "lo"}
    if set(which) == allv:
        s = sub(s, "    u32 field;\n", "".join(f"    u32 {n};\n" for n in decl_fs))
    else:
        s = sub(s, "    u32 field;\n", "    u32 field;\n" + "".join(f"    u32 {n};\n" for n in decl_fs))
    return s


# ---------------------------------------------------------------- temp
MAG1 = [("GETBITS_PRE(temp, 11, 0x800);", "GETBITS_PRE(mag1, 11, 0x800);"),
        ("GETBITS_PRE(temp, nbits, 1 << nbits);", "GETBITS_PRE(mag1, nbits, 1 << nbits);"),
        ("                    temp = nbits;\n", "                    mag1 = nbits;\n")]
ZZ = "temp = (temp & 1) ? -(temp / 2) - 1 : temp / 2;"
FLAG2 = "                GETBITS(temp, 1);\n                if (temp) {\n                    temp = 0;\n                } else {\n                    GETBITS(temp, 12);\n"
MAG3 = [("GETBITS_PRE(temp, 7, 0x80);", "GETBITS_PRE(mag3, 7, 0x80);"),
        ("GETBITS_PRE(temp, nbits2, 1 << nbits2);", "GETBITS_PRE(mag3, nbits2, 1 << nbits2);"),
        ("                        temp = nbits2;\n", "                        mag3 = nbits2;\n")]
C3FLAG = "                GETBITS(temp, 1);\n                if (temp) {\n                    /* Ruling 11"
C3VAL = ["temp = ((temp << 3) | (", "                        temp = -temp;\n"]
ACC = ["                work[ch + 0x45] = temp;\n                work[ch + 3] += temp;\n",
       "                work[ch + 3] += work[ch + 0x45] + temp;\n                work[ch + 0x45] += temp;\n"]


def temp_split(s, which, fs=False):
    """which: subset of final, mag1, flag, mag3 -> own locals (final -> `delta`)."""
    final = "delta" if "final" in which else "temp"
    m1 = "mag1" if "mag1" in which else "temp"
    fl = "flag" if "flag" in which else "temp"
    m3 = "mag3" if "mag3" in which else "temp"
    for o, n in MAG1:
        s = sub(s, o, n.replace("mag1", m1))
    s = sub(s, ZZ, f"{final} = ({m1} & 1) ? -({m1} / 2) - 1 : {m1} / 2;")
    s = sub(s, FLAG2, FLAG2.replace("GETBITS(temp, 1);\n                if (temp)", f"GETBITS({fl}, 1);\n                if ({fl})")
            .replace("temp = 0;", f"{final} = 0;").replace("GETBITS(temp, 12);", f"GETBITS({final}, 12);"))
    for o, n in MAG3:
        s = sub(s, o, n.replace("mag3", m3))
    s = sub(s, C3FLAG, C3FLAG.replace("temp", final))
    s = sub(s, C3VAL[0], f"{final} = (({m3} << 3) | (")
    s = sub(s, C3VAL[1], f"                        {final} = -{final};\n")
    for a in ACC:
        s = sub(s, a, a.replace("temp", final))
    decl = []
    for nm, top in (("delta", None), ("mag1", C1_TOP), ("flag", C2_TOP), ("mag3", C3_IF)):
        key = {"delta": "final"}.get(nm, nm)
        if key not in which:
            continue
        if fs or top is None:
            decl.append(nm)
        else:
            s = sub(s, top, top + f"{I16 if top != C3_IF else I20}s16 {nm};\n" + ("\n" if top == C2_TOP else ""))
    keep_temp = set(which) != {"final", "mag1", "flag", "mag3"}
    if fs:
        s = sub(s, "            s16 temp;\n\n", "" if not keep_temp else "            s16 temp;\n\n")
        s = sub(s, "    s16 x;\n", "    s16 x;\n" + "".join(f"    s16 {n};\n" for n in decl))
    else:
        ch_decl = ("            s16 temp;\n" if keep_temp else "") + "".join(f"            s16 {n};\n" for n in decl)
        s = sub(s, "            s16 temp;\n", ch_decl)
    return s


# ---------------------------------------------------------------- nbits / nbits2
def nbits_split(s, v2=False, fs=False):
    nb, zz, ln, lim = ("nbits2", "zeros2", "len2", 8) if v2 else ("nbits", "zeros", "len", 12)
    ind = I24 if v2 else I20
    pairs = [(f"{nb} = 0;\n", f"{zz} = 0;\n"), (f"{nb}++;\n", f"{zz}++;\n"),
             (f"while ({nb} < {lim});", f"while ({zz} < {lim});"),
             (f"if ({nb} == {lim}) {{", f"if ({zz} == {lim}) {{"),
             (f"}} else if ({nb} >= 2) {{\n{ind}{nb} = {nb} - 1;\n",
              f"}} else if ({zz} >= 2) {{\n" + ("" if fs else f"{ind}s32 {ln};\n\n") + f"{ind}{ln} = {zz} - 1;\n"),
             (f", {nb}, 1 << {nb});", f", {ln}, 1 << {ln});"),
             (f"= {nb};\n", f"= {zz};\n")]
    for o, n in pairs:
        s = sub(s, o, n)
    if fs:
        s = sub(s, f"s32 {nb};\n", "")
        s = sub(s, "    s16 x;\n", f"    s16 x;\n    s32 {zz};\n    s32 {ln};\n")
    else:
        s = sub(s, f"s32 {nb};\n", f"s32 {zz};\n")
    return s


FIELD_ALL = ["h0", "h1", "h2", "kflag", "h3", "h4", "h5", "lo"]
TEMP_ALL = ["final", "mag1", "flag", "mag3"]
V = {}
V["landing"] = L
V["pv_idx"] = pv_idx(L)
V["pv_idx2"] = pv_idx2(L)
V["pv_field"] = field_split(L, FIELD_ALL)
V["pv_temp"] = temp_split(L, TEMP_ALL)
V["pv_nbits"] = nbits_split(L)
V["pv_nbits2"] = nbits_split(L, v2=True)
a = L
a = field_split(a, FIELD_ALL); a = temp_split(a, TEMP_ALL); a = pv_idx(a); a = pv_idx2(a)
a = nbits_split(a); a = nbits_split(a, v2=True)
V["pv_all"] = a
for nm in FIELD_ALL:
    V[f"abl_field_{nm}"] = field_split(L, [nm])
for nm in TEMP_ALL:
    V[f"abl_temp_{nm}"] = temp_split(L, [nm])
# structural / scope respellings (function-scope per-value locals)
V["fs_idx"] = pv_idx(L, fs=True)
V["fs_field"] = field_split(L, FIELD_ALL, fs=True)
V["fs_temp"] = temp_split(L, TEMP_ALL, fs=True)
V["fs_nbits"] = nbits_split(L, fs=True)
V["fs_nbits2"] = nbits_split(L, v2=True, fs=True)
for n, t in V.items():
    with open(os.path.join(OUT, n + ".c"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(t)
print(len(V), "variants ->", OUT)
