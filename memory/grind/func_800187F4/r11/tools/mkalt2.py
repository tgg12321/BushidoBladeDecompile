"""Ruling 11 (D)(4) structural respellings and the sanctioned-family (FAKE) probes, each built on
the landing chassis's per-value spelling of the variable under test (tmp/func_800187F4/r11pv_<var>.c,
generated from c5a by mkpv3.py). Writes tmp/func_800187F4/{st,fam}_<var>_<what>.c; prints tags."""
import re

D = "tmp/func_800187F4/"


def rd(var):
    return open(D + f"r11pv_{var}.c").read()


def once(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)


def drop_decl(s, name):
    s2, n = re.subn(rf"\n[ \t]*s32 {re.escape(name)};\n", "\n", s, count=1)
    assert n == 1, name
    return s2


def fn_scope(s, names):
    for n in names:
        s = drop_decl(s, n)
    return once(s, "    s32 lz[6];\n", "    s32 lz[6];\n" + "".join(f"    s32 {n};\n" for n in names))


def loop_body_end(s, head):
    i = s.index(head)
    depth, p = 0, i + len(head) - 1
    while True:
        c = s[p]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i, p
        p += 1


V = {}
# ---------------- structural respellings ----------------
s = rd("idx")
for nm in ("idx_add", "idx_sub"):
    head = f"for ({nm} = 0; {nm} < nforce; {nm}++) {{"
    i, p = loop_body_end(s, head)
    pad = s[s.rfind("\n", 0, i) + 1:i]
    body = s[i + len(head):p]
    s = s[:i] + f"{nm} = 0;\n{pad}while ({nm} < nforce) {{" + body + f"    {nm}++;\n{pad}}}" + s[p + 1:]
V["st_idx_while"] = s
V["st_idx_fscope"] = fn_scope(rd("idx"), ["idx_add", "idx_sub", "idx_sph"])

s = rd("nforce")
s = once(s, "        nforce_add = node[7];\n", "")
s = once(s, "        nforce_sub = node[8];\n", "")
s = once(s, "idx < nforce_add;", "idx < node[7];")
s = once(s, "idx < nforce_sub;", "idx < node[8];")
s = drop_decl(drop_decl(s, "nforce_add"), "nforce_sub")
V["st_nforce_inline"] = s
V["st_nforce_fscope"] = fn_scope(rd("nforce"), ["nforce_add", "nforce_sub"])

s = rd("temp")
s = once(s, "                lzc_in = sq1;\n", "")
s = once(s, "@gte_Lzc(lzc_in, &lz[0]);", "@gte_Lzc(sq1, &lz[0]);")
s = once(s, "                    lut1 = (&D_8008D118)[sq1 >> nbits];\n", "")
s = once(s, "(lut1 << 16) >> (0x13 - (nbits >> 1))", "((&D_8008D118)[sq1 >> nbits] << 16) >> (0x13 - (nbits >> 1))")
s = once(s, "                    lut2 = (&D_8008D118)[sq2 >> nbits2];\n", "")
s = once(s, "(lut2 << 16) >> (0x13 - (nbits2 >> 1))", "((&D_8008D118)[sq2 >> nbits2] << 16) >> (0x13 - (nbits2 >> 1))")
for n in ("lzc_in", "lut1", "lut2"):
    s = drop_decl(s, n)
V["st_temp_inline"] = s
V["st_temp_fscope"] = fn_scope(rd("temp"), ["lzc_in", "lut1", "lut2"])

for var, cnt, sh, lz in (("nbits", "lzcount", "shift", "lz[0]"), ("nbits2", "lzcount2", "shift2", "lz[1]")):
    s = rd(var)
    s = once(s, f"                    {cnt} = {lz};\n                    {sh} = 0x16 - ({cnt} & ~1);\n",
             f"                    {sh} = 0x16 - ({lz} & ~1);\n")
    V[f"st_{var}_onestmt"] = drop_decl(s, cnt)
    V[f"st_{var}_fscope"] = fn_scope(rd(var), [cnt, sh])

for tag, text in V.items():
    open(D + tag + ".c", "w", newline="\n").write(text)
print(" ".join(V))
