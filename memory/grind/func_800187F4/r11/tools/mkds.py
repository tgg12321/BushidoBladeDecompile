"""Dead-store placement sweep for the remaining Ruling 11 locals on the c6 chassis.

For each variable (idx, nforce, temp, nbits, nbits2): its one-variable-per-value spelling with
every fresh local declared at FUNCTION scope (a scope respelling, so a dead store can sit anywhere),
then, for each fresh local and each anchor, `X = 0;` inserted there (a FAKE dead store). Also the
per-value spelling itself (function scope, no dead store). Tags ds_<var>_<local>_<anchor>.
Writes tmp/func_800187F4/<tag>.c; prints tags."""
import re

D = "tmp/func_800187F4/"
C6 = open(D + "c6.c").read()


def once(s, a, b):
    assert s.count(a) == 1, (a[:70], s.count(a))
    return s.replace(a, b)


def strip_decl(s, name):
    s2, n = re.subn(rf"\n([ \t]*)s32 {re.escape(name)};\n", "\n", s, count=1)
    assert n == 1, name
    return s2


FUNC_DECL = "    s32 lz[6];\n"


def to_fscope(s, names):
    return once(s, FUNC_DECL, FUNC_DECL + "".join(f"    s32 {n};\n" for n in names))


def pv_idx(s):
    parts = s.split("for (idx = 0; idx < ")
    assert len(parts) == 4
    names = ["idx_add", "idx_sub", "idx_sph"]
    out = parts[0]
    for k in range(3):
        out += f"for ({names[k]} = 0; {names[k]} < " + re.sub(r"\bidx\b", names[k], parts[k + 1])
    out = strip_decl(out, "idx")
    return to_fscope(out, names), names


def pv_nforce(s):
    s = once(s, "        nforce = node[7];\n", "        nforce_add = node[7];\n")
    s = once(s, "        nforce = node[8];\n", "        nforce_sub = node[8];\n")
    hdr = re.compile(r"(for \(idx = 0; idx < )nforce(; idx\+\+\) \{)")
    s = hdr.sub(lambda m: m.group(1) + "nforce_add" + m.group(2), s, count=1)
    s = hdr.sub(lambda m: m.group(1) + "nforce_sub" + m.group(2), s, count=1)
    s = strip_decl(s, "nforce")
    return to_fscope(s, ["nforce_add", "nforce_sub"]), ["nforce_add", "nforce_sub"]


def pv_temp(s):
    s = once(s, "                temp = sq1;\n", "                lzc_in = sq1;\n")
    s = once(s, "@gte_Lzc(temp, &lz[0]);", "@gte_Lzc(lzc_in, &lz[0]);")
    s = once(s, "                    temp = (&D_8008D118)[sq1 >> nbits];\n", "                    lut1 = (&D_8008D118)[sq1 >> nbits];\n")
    s = once(s, "(temp << 16) >> (0x13 - (nbits >> 1))", "(lut1 << 16) >> (0x13 - (nbits >> 1))")
    s = once(s, "                    temp = (&D_8008D118)[sq2 >> nbits2];\n", "                    lut2 = (&D_8008D118)[sq2 >> nbits2];\n")
    s = once(s, "(temp << 16) >> (0x13 - (nbits2 >> 1))", "(lut2 << 16) >> (0x13 - (nbits2 >> 1))")
    s = strip_decl(s, "temp")
    return to_fscope(s, ["lzc_in", "lut1", "lut2"]), ["lzc_in", "lut1", "lut2"]


def pv_nb(s, var, lz, cnt, sh):
    s = once(s, f"                    {var} = {lz};\n", f"                    {cnt} = {lz};\n")
    s = once(s, f"                    {var} = 0x16 - ({var} & ~1);\n", f"                    {sh} = 0x16 - ({cnt} & ~1);\n")
    head, tail = s.split(f"                    {sh} = 0x16 - ({cnt} & ~1);\n", 1)
    end = tail.index("                }\n")
    s = head + f"                    {sh} = 0x16 - ({cnt} & ~1);\n" + re.sub(rf"\b{var}\b", sh, tail[:end]) + tail[end:]
    s = strip_decl(s, var)
    return to_fscope(s, [cnt, sh]), [cnt, sh]


ANCHORS = {
    "afterforce": "        SCR->vel[0] = vx;\n",
    "sphtop": "                r = SCR->rad[idx];\n",
    "beforetot": "                tot = dist1 + dist2;\n",
    "sphend": "                @gte_stlvl(SCR->vel);\n            }\n",
    "collend": "            depth = 0;\n",
    "nodeend": "        node[3] = (SCR->vel[0] * 7) >> 3;\n",
}


def insert(s, anchor_key, stmt):
    a = ANCHORS[anchor_key]
    if anchor_key == "sphtop" and a not in s:        # idx renamed in the idx spelling
        a = "                r = SCR->rad[idx_sph];\n"
    if anchor_key == "sphend":
        return once(s, a, a.replace("            }\n", "") + "                " + stmt + "\n            }\n")
    ind = re.match(r"[ ]*", a).group(0)
    return once(s, a, ind + stmt + "\n" + a)


GENS = {"idx": lambda s: pv_idx(s), "nforce": pv_nforce, "temp": pv_temp,
        "nbits": lambda s: pv_nb(s, "nbits", "lz[0]", "lzcount", "shift"),
        "nbits2": lambda s: pv_nb(s, "nbits2", "lz[1]", "lzcount2", "shift2")}
tags = []
for var, g in GENS.items():
    base, names = g(C6)
    open(D + f"ds_{var}_pv.c", "w", newline="\n").write(base)
    tags.append(f"ds_{var}_pv")
    for n in names:
        for ak in ANCHORS:
            t = f"ds_{var}_{n}_{ak}"
            open(D + t + ".c", "w", newline="\n").write(insert(base, ak, f"{n} = 0; /* FAKE */"))
            tags.append(t)
print(" ".join(tags))
