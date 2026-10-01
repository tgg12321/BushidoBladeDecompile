"""func_8002CD58 Ruling 11: the one-variable-per-value spellings and structural respellings of the
landing body memory/grind/func_8002CD58/candidate.c, written to r11/variants/.
usage: python r11gen.py   (paths are relative to this file)"""
import os, re

H = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(H, "..", "variants")
src = open(os.path.join(H, "..", "..", "candidate.c")).read()
body = src[src.index("s32 func_8002CD58(u8 *obj) {"):]
# drop the two Ruling 11 annotations (comments only)
body = re.sub(r"        /\* len holds two values.*?\*/\n", "", body, flags=re.S)
body = re.sub(r"    /\* temp holds two values.*?\*/\n", "", body, flags=re.S)
assert "Ruling 11" not in body
V = {"reuse": body}

GUARD = "        if ((u32)len < 0x4000) {\n"
i_guard = body.index(GUARD) + len(GUARD)
i_tail = body.index("    nyaw = ratan2(")


def split_len(b, name="xz_len", decl="block"):
    """len's second value (|a.xz|, written after the guard) in its own variable."""
    g = b.index(GUARD) + len(GUARD)
    t = b.index("    nyaw = ratan2(")
    mid = re.sub(r"\blen\b", name, b[g:t])
    b = b[:g] + mid + b[t:]
    if decl == "block":
        return b.replace("        s32 len;\n", "        s32 len;\n        s32 %s;\n" % name, 1)
    if decl == "block_rev":
        return b.replace("        s32 len;\n", "        s32 %s;\n        s32 len;\n" % name, 1)
    if decl == "inner":
        return b.replace(GUARD, GUARD + "            s32 %s;\n" % name, 1)
    if decl == "func":
        return b.replace("    s32 nxz_len;\n", "    s32 nxz_len;\n    s32 %s;\n" % name, 1)
    raise ValueError(decl)


TL = ("            temp = g_sqrt_table_u8[(u32)temp >> shift];\n"
      "            nxz_len = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));\n")
assert TL in body


def split_temp(b, form="tbl"):
    """temp's second value (the table byte) in its own variable; temp renamed nxz_sq."""
    forms = {
        "tbl": "            s32 tbl = g_sqrt_table_u8[(u32)temp >> shift];\n"
               "            nxz_len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n",
        "tbl_u32": "            u32 tbl = g_sqrt_table_u8[(u32)temp >> shift];\n"
                   "            nxz_len = (tbl << 16) >> (0x13 - ((u32)shift >> 1));\n",
        "tbl_u8": "            u8 tbl = g_sqrt_table_u8[(u32)temp >> shift];\n"
                  "            nxz_len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n",
        "inline": "            nxz_len = (u32)(g_sqrt_table_u8[(u32)temp >> shift] << 16) >> (0x13 - ((u32)shift >> 1));\n",
        "shift_first": "            s32 tbl = g_sqrt_table_u8[(u32)temp >> shift] << 16;\n"
                       "            nxz_len = (u32)tbl >> (0x13 - ((u32)shift >> 1));\n",
        "tbl_func": "            tbl = g_sqrt_table_u8[(u32)temp >> shift];\n"
                    "            nxz_len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n",
    }
    b = b.replace(TL, forms[form], 1)
    if form == "tbl_func":
        b = b.replace("    s32 temp;\n", "    s32 temp;\n    s32 tbl;\n", 1)
    return re.sub(r"\btemp\b", "nxz_sq", b)


# one-variable-per-value spellings
V["pv_both"] = split_temp(split_len(body))
V["pv_len"] = split_len(body)
V["pv_temp"] = split_temp(body)
# len split, structural / declaration respellings (temp reuse kept so only len's property is measured)
V["len_decl_rev"] = split_len(body, decl="block_rev")
V["len_decl_inner"] = split_len(body, decl="inner")
V["len_decl_func"] = split_len(body, decl="func")
b = split_len(body)
V["len_u32"] = b.replace("        s32 len;\n        s32 xz_len;\n", "        u32 len;\n        u32 xz_len;\n", 1)
V["len_le"] = b.replace("if ((u32)len < 0x4000)", "if ((u32)len <= 0x3FFF)")
L = b.split("\n")
gi = L.index(GUARD.rstrip("\n"))
ri = next(i for i in range(gi, len(L)) if L[i] == "            return 0;")
assert L[ri + 1] == "        }" and L[ri + 2] == "    }"
dd = [l[4:] if l.startswith("    ") else l for l in L[gi + 1:ri + 1]]
V["len_goto"] = "\n".join(L[:gi] + ["        if ((u32)len >= 0x4000) {", "            goto fallback;", "        }"]
                          + dd + L[ri + 2:ri + 3] + ["", "fallback:"] + L[ri + 3:])
V["len_inverted"] = "\n".join(L[:gi] + ["        if ((u32)len >= 0x4000) {", "        } else {"] + L[gi + 1:])
# len's FIRST value in its own variable instead (|a.xz| stays in len) -- same split, other naming
V["len_split_first"] = re.sub(r"\blen\b", "n_len", body[:i_guard]) + body[i_guard:]
V["len_split_first"] = V["len_split_first"].replace("        s32 n_len;\n", "        s32 n_len;\n        s32 len;\n", 1)
# temp split, respellings (len reuse kept)
for f in ("tbl_u32", "tbl_u8", "inline", "shift_first", "tbl_func"):
    V["temp_" + f] = split_temp(body, f)
# both split, with temp respellings
V["pv_both_inline"] = split_temp(split_len(body), "inline")
V["pv_both_goto"] = split_temp(V["len_goto"])
for k, s in V.items():
    if k != "reuse":
        assert s != body, k
    open(os.path.join(OUT, k + ".c"), "w", newline="\n").write(s)
print(",".join("memory/grind/func_8002CD58/r11/variants/%s.c" % k for k in V))
