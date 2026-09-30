"""func_8001F2E4 Ruling 11 variants, generated from the EXACT landing body (candidate.c).

Every variant differs from the landing body ONLY in declarations and identifiers
(Ruling 11 (C)(2)): a split value is renamed inside its own source range and gets
its own declaration at the innermost scope enclosing that value's writes
(Ruling 11 (C)(1)); every other value stays in the shared variable.

usage: python3 gen2.py <landing body .c> <outdir>
"""
import os
import re
import sys

NL = chr(10)
BODY = open(sys.argv[1]).read()
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)

# ---- source-range markers (each must occur exactly once) ----
MK = {
    "E1": "    temp = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;\n",
    "E2": "    temp = (temp2 - *(s16 *)(obj + 0x1E8)) & 0xFFF;\n",
    "CALL": "    func_8002F770((s16 *)(a + 0x36),",
    "B3": "    t = *(s16 *)(obj + 0xC);\n",
    "B3IN": "*(s16 *)(obj + 0x8C) != 0) {\n",
    "B4": "    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U",
    "B4DECL": "        s32 dist_sq;\n        s32 dist;\n\n        if (*(s32 *)(obj + 0x268) == 0) {\n",
    "B4D": "        temp = (temp2 - *(s16 *)(obj + 0x1EA)) & 0xFFF;\n",
    "B5": "    if (*(s16 *)(obj + 0x26E) != 0",
    "B5IN": "*(s16 *)(obj + 0x96) == 0) {\n",
    "J2": "        temp = (rng_Next() & 0x3F) - 0x20;\n        *(u16 *)(a + 0x1E)",
    "B1ARM": "        } else {\n            s32 dist_sq;\n            s32 dist;\n",
    "END": "\n}\n",
}
for k, v in MK.items():
    n = BODY.count(v)
    assert n == 1 or (k == "END"), (k, n)


def pos(b, key):
    i = b.find(MK[key])
    assert i >= 0, key
    return i


def rename(b, lo, hi, old, new):
    """rename identifier `old` -> `new` in b[pos(lo) : pos(hi)] (hi None = end)."""
    i = pos(b, lo)
    j = len(b) if hi is None else pos(b, hi)
    part = re.sub(r"\b%s\b" % old, new, b[i:j])
    return b[:i] + part + b[j:]


def decl_after(b, key, text, where="after"):
    """insert declaration text right after the marker line(s) MK[key]."""
    i = pos(b, key) + (len(MK[key]) if where == "after" else 0)
    return b[:i] + text + b[i:]


FUNC_DECL_ANCHOR = "    s16 t;\n"


def func_decl(b, text):
    i = b.index(FUNC_DECL_ANCHOR)
    return b[:i] + text + b[i:]


def B1ARM_decl(b, text):
    # after the block-1 else-arm's own `s32 dist;` line
    i = pos(b, "B1ARM") + len(MK["B1ARM"])
    return b[:i] + text + b[i:]


def B4_decl(b, text):
    # after block 4's `s32 dist_sq; s32 dist;` (before the blank line)
    i = b.index("        s32 dist;\n", pos(b, "B4")) + len("        s32 dist;\n")
    return b[:i] + text + b[i:]


def _blk_decl(b, key, text):
    i = pos(b, key) + len(MK[key])
    sep = "" if b[i:].startswith("        s32 ") else NL
    return b[:i] + text + sep + b[i:]


def B3_decl(b, text):
    return _blk_decl(b, "B3IN", text)


def B5_decl(b, text):
    return _blk_decl(b, "B5IN", text)


def drop_func_decl(b, name):
    s = "    s32 %s;\n" % name
    assert b.count(s) == 1, name
    return b.replace(s, "", 1)


# ---- one op per value: split it out into its own fresh local ----
def V1(b):  # 0x1E6 easing delta (function scope: its writes are at function scope)
    return func_decl(rename(b, "E1", "E2", "temp", "d1e6"), "    s32 d1e6;\n")


def V2(b):  # 0x1E8 easing delta
    return func_decl(rename(b, "E2", "CALL", "temp", "d1e8"), "    s32 d1e8;\n")


def V3(b):  # block-3 twist
    return B3_decl(rename(b, "B3", "B4", "temp", "twist"), "        s32 twist;\n")


def V4(b):  # block-4 0x1EA easing delta
    return B4_decl(rename(b, "B4D", "B5", "temp", "delta"), "        s32 delta;\n")


def V5(b):  # jitter 1
    return B5_decl(rename(b, "B5", "J2", "temp", "jit1"), "        s32 jit1;\n")


def V6(b):  # jitter 2
    b = rename(b, "J2", None, "temp", "jit2")
    if "        s32 jit1;\n" in b:
        i = b.index("        s32 jit1;\n") + len("        s32 jit1;\n")
        return b[:i] + "        s32 jit2;\n" + b[i:]
    return B5_decl(b, "        s32 jit2;\n")


def V56(b):  # both jitters in one block-5 local (NOT a per-value spelling; diagnostic)
    return B5_decl(rename(b, "B5", None, "temp", "jitter"), "        s32 jitter;\n")


def W2(b):  # temp2's block-4 twist target
    return B4_decl(rename(b, "B4", "B5", "temp2", "twist_tgt"), "        s32 twist_tgt;\n")


def split_dx(b, which):
    """move one of dx/dz fully per-value: block-1 value into the block-1 else arm,
    block-4 value into block 4, both keeping the name (different scopes)."""
    b = drop_func_decl(b, which)
    b = B1ARM_decl(b, "            s32 %s;\n" % which)
    b = B4_decl(b, "        s32 %s;\n" % which)
    return b


def only_b4(b, which, new):
    """split only the block-4 value of dx/dz, under a new name, rest shared."""
    return B4_decl(rename(b, "B4", "B5", which, new), "        s32 %s;\n" % new)


def split_temp2(b):
    # W2 into block 4 as twist_tgt; W1 keeps a function-scope local named elev
    b = W2(b)
    i = b.index("    s32 temp2;\n")
    b = b[:i] + "    s32 elev;\n" + b[i + len("    s32 temp2;\n"):]
    return re.sub(r"\btemp2\b", "elev", b)


def split_temp(b):
    b = V6(V5(V4(V3(V2(V1(b))))))
    return drop_func_decl(b, "temp")


def strip_comments(b):
    """declaration comments only: variants keep every statement; comments are inert."""
    return re.sub(r"    /\* (?:temp2?:|dx / dz:).*?\*/\n", "", b, flags=re.S)


base = strip_comments(BODY)
V = {"R_base": base}
for n, f in [("V1", V1), ("V2", V2), ("V3", V3), ("V4", V4), ("V5", V5), ("V6", V6), ("V56", V56)]:
    V["only_" + n] = f(base)
V["only_W2"] = W2(base)
V["only_X2"] = only_b4(base, "dx", "dx2")
V["only_Z2"] = only_b4(base, "dz", "dz2")
V["only_XZ2"] = only_b4(only_b4(base, "dx", "dx2"), "dz", "dz2")
V["split_dxdz"] = split_dx(split_dx(base, "dz"), "dx")
V["split_temp"] = split_temp(base)
V["split_temp2"] = split_temp2(base)
V["split_all"] = split_temp2(split_temp(split_dx(split_dx(base, "dz"), "dx")))
# each variable kept shared, everything else per-value
V["keep_temp"] = split_temp2(split_dx(split_dx(base, "dz"), "dx"))
V["keep_temp2"] = split_temp(split_dx(split_dx(base, "dz"), "dx"))
V["keep_dxdz"] = split_temp2(split_temp(base))
V["R_base"] = BODY
for n, b in V.items():
    open(os.path.join(OUT, n + ".c"), "w", newline=NL).write(b)
print("wrote", len(V), "variants to", OUT)
