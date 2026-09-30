"""Structural respellings of the one-variable-per-value body (split_all.c) for Ruling 11 (D)(4) / Q31.
None uses a FAKE construct. usage: python3 gen_struct.py <split_all.c> <outdir>"""
import os
import re
import sys

NL = chr(10)
S = open(sys.argv[1]).read()
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)
V = {}


def must(b, old, new, count=1):
    assert b.count(old) == count, (old, b.count(old))
    return b.replace(old, new)


# S1: every per-value local at function scope (declaration placement only)
b = S
for d in ["            s32 dx;\n", "            s32 dz;\n", "        s32 twist;\n\n", "        s32 twist_tgt;\n",
          "        s32 delta;\n", "        s32 dx;\n", "        s32 dz;\n", "        s32 jit1;\n", "        s32 jit2;\n\n"]:
    b = must(b, d, "", 1)
b = must(b, "    s16 t;\n", "    s32 dx;\n    s32 dz;\n    s32 twist;\n    s32 twist_tgt;\n    s32 delta;\n"
         "    s32 dx2;\n    s32 dz2;\n    s32 jit1;\n    s32 jit2;\n    s16 t;\n")
i = b.index("    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U")
j = b.index("    if (*(s16 *)(obj + 0x26E) != 0")
b = b[:i] + re.sub(r"\bdx\b", "dx2", re.sub(r"\bdz\b", "dz2", b[i:j])) + b[j:]
V["S1_funcscope"] = b

# S2: compound-assignment easing stores (`+= d / 8`)
b = S
for f, v in [("0x1E6", "d1e6"), ("0x1E8", "d1e8")]:
    b = must(b, "    *(s16 *)(obj + %s) = *(s16 *)(obj + %s) + %s / 8;\n" % (f, f, v),
             "    *(s16 *)(obj + %s) += %s / 8;\n" % (f, v))
b = must(b, "        *(s16 *)(obj + 0x1EA) = *(s16 *)(obj + 0x1EA) + delta / 8;\n",
         "        *(s16 *)(obj + 0x1EA) += delta / 8;\n")
V["S2_compound"] = b

# S3: no dx/dz locals -- the squared distance written from the loads
b = S
for sfx in ["0x180", "0xF4"]:
    pass
b = must(b, "            s32 dx;\n            s32 dz;\n", "")
b = must(b, "        s32 dx;\n        s32 dz;\n", "")
b = must(b, "            dx = *(s32 *)(*(u8 **)obj + 0x180) - *(s32 *)(obj + 0x180);\n"
            "            dz = *(s32 *)(*(u8 **)obj + 0x188) - *(s32 *)(obj + 0x188);\n"
            "            dist_sq = dx * dx + dz * dz;\n",
         "            dist_sq = (*(s32 *)(*(u8 **)obj + 0x180) - *(s32 *)(obj + 0x180)) * (*(s32 *)(*(u8 **)obj + 0x180) - *(s32 *)(obj + 0x180))\n"
         "                    + (*(s32 *)(*(u8 **)obj + 0x188) - *(s32 *)(obj + 0x188)) * (*(s32 *)(*(u8 **)obj + 0x188) - *(s32 *)(obj + 0x188));\n")
b = must(b, "        dx = *(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C);\n"
            "        dz = *(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264);\n"
            "        dist_sq = dx * dx + dz * dz;\n",
         "        dist_sq = (*(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C)) * (*(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C))\n"
         "                + (*(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264)) * (*(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264));\n")
V["S3_no_dxdz"] = b

# S4: the three wrap-and-ease steps through one static inline helper (per-value by construction)
b = S
HELP = ("static inline s32 wrap12(s32 d) {\n    d &= 0xFFF;\n    if (d >= 0x800) {\n        d -= 0x1000;\n    }\n"
        "    return d;\n}\n")
b = HELP + b
for v, e in [("d1e6", "tgt_z - *(s16 *)(obj + 0x1E6)"), ("d1e8", "elev - *(s16 *)(obj + 0x1E8)")]:
    b = must(b, "    %s = (%s) & 0xFFF;\n    if (%s >= 0x800) {\n        %s -= 0x1000;\n    }\n" % (v, e, v, v),
             "    %s = wrap12(%s);\n" % (v, e))
b = must(b, "        delta = (twist_tgt - *(s16 *)(obj + 0x1EA)) & 0xFFF;\n        if (delta >= 0x800) {\n"
            "            delta -= 0x1000;\n        }\n",
         "        delta = wrap12(twist_tgt - *(s16 *)(obj + 0x1EA));\n")
V["S4_inline_wrap"] = b

# S5: reverse declaration order of the function-scope locals
b = S
b = must(b, "    s32 tgt_z;\n    s32 elev;\n", "")
b = must(b, "    s32 d1e6;\n    s32 d1e8;\n", "    s32 d1e8;\n    s32 d1e6;\n    s32 elev;\n    s32 tgt_z;\n")
V["S5_decl_reverse"] = b

# S6: jitters each in its own nested block, same name
b = S
b = must(b, "        s32 jit1;\n        s32 jit2;\n\n", "")
i = b.index("        jit1 = (rng_Next()")
j = b.index("        jit2 = (rng_Next()")
k = b.index("    }\n}\n", j)
p1 = b[i:j].replace("jit1", "jitter")
p2 = b[j:k].replace("jit2", "jitter")
ind = lambda s: "".join("    " + l + NL if l else NL for l in s.rstrip(NL).split(NL))
b = b[:i] + "        {\n            s32 jitter;\n\n" + ind(p1) + "        }\n        {\n            s32 jitter;\n\n" + ind(p2) + "        }\n" + b[k:]
V["S6_jitter_blocks"] = b

for n, b in V.items():
    open(os.path.join(OUT, n + ".c"), "w", newline=NL).write(b)
print("wrote", sorted(V))
