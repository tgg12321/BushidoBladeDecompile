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
s = once(s, "                lzc_in = work;\n", "")
s = once(s, "@gte_Lzc(lzc_in, &lz[0]);", "@gte_Lzc(work, &lz[0]);")
s = once(s, "                    lut1 = (&D_8008D118)[work >> nbits];\n", "")
s = once(s, "(lut1 << 16) >> (0x13 - (nbits >> 1))", "((&D_8008D118)[work >> nbits] << 16) >> (0x13 - (nbits >> 1))")
s = once(s, "                    lut2 = (&D_8008D118)[sq2 >> nbits2];\n", "")
s = once(s, "(lut2 << 16) >> (0x13 - (nbits2 >> 1))", "((&D_8008D118)[sq2 >> nbits2] << 16) >> (0x13 - (nbits2 >> 1))")
for n in ("lzc_in", "lut1", "lut2"):
    s = drop_decl(s, n)
V["st_temp_inline"] = s
V["st_temp_fscope"] = fn_scope(rd("temp"), ["lzc_in", "lut1", "lut2"])

s = rd("work")
s = once(s, "                sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                temp = sq1;\n                if (sq1 < 0x400) {\n",
         "                temp = sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                if (sq1 < 0x400) {\n")
V["st_work_chainassign"] = s
V["st_work_fscope"] = fn_scope(rd("work"), ["sq1", "dist1"])

s = rd("delta")
s = once(s, "            dg = SCR->pos[1] - SCR->ground;\n", "")
s = once(s, "            if (dg > 0) {\n", "            if (SCR->pos[1] - SCR->ground > 0) {\n")
s = once(s, "                if (dg > 0x3200) {\n", "                if (SCR->pos[1] - SCR->ground > 0x3200) {\n")
s = once(s, "vy - dg / 8;", "vy - (SCR->pos[1] - SCR->ground) / 8;")
V["st_delta_inline"] = drop_decl(s, "dg")
V["st_delta_fscope"] = fn_scope(rd("delta"), ["dg", "dy0"])

for var, cnt, sh, lz in (("nbits", "lzcount", "shift", "lz[0]"), ("nbits2", "lzcount2", "shift2", "lz[1]")):
    s = rd(var)
    s = once(s, f"                    {cnt} = {lz};\n                    {sh} = 0x16 - ({cnt} & ~1);\n",
             f"                    {sh} = 0x16 - ({lz} & ~1);\n")
    V[f"st_{var}_onestmt"] = drop_decl(s, cnt)
    V[f"st_{var}_fscope"] = fn_scope(rd(var), [cnt, sh])

# ---------------- sanctioned-family (FAKE) probes on the per-value spellings ----------------
END_SPH = "                @gte_stlvl(SCR->vel);\n            }\n"
s = rd("delta")
V["fam_delta_selfassign_end"] = once(s, END_SPH, END_SPH + "            dg = dg;\n")
V["fam_delta_deadstore_end"] = once(s, END_SPH, END_SPH + "            dg = 0;\n")
V["fam_delta_chain_dy0"] = once(s, "                dy0 = SCR->cpos[1] - SCR->sph[idx][1];\n",
                                "                dy0 = SCR->cpos[1] - SCR->sph[idx][1] + dg - dg;\n")
V["fam_delta_chain_store"] = once(s, "                SCR->vel[1] = vy_new;\n", "                SCR->vel[1] = vy_new + dg - dg;\n")
s = rd("temp")
V["fam_temp_selfassign"] = once(s, "                lzc_in = work;\n", "                lzc_in = work;\n                lzc_in = lzc_in;\n")
V["fam_temp_chain_lut1"] = once(s, "                    lut1 = (&D_8008D118)[work >> nbits];\n",
                                "                    lut1 = (&D_8008D118)[work >> nbits] + lzc_in - lzc_in;\n")
V["fam_temp_dead"] = once(s, "                if (work >= r) {\n", "                lzc_in = 0;\n                if (work >= r) {\n")
s = rd("nforce")
V["fam_nforce_selfassign"] = once(s, "        nforce_sub = node[8];\n", "        nforce_add = nforce_add;\n        nforce_sub = node[8];\n")
V["fam_nforce_chain"] = once(s, "idx < nforce_sub;", "idx < nforce_sub + nforce_add - nforce_add;")
V["fam_nforce_dead"] = once(s, "        SCR->vel[0] = vx;\n", "        nforce_add = 0;\n        SCR->vel[0] = vx;\n")
s = rd("work")
V["fam_work_chain"] = once(s, "                if (dist1 >= r) {\n", "                if (dist1 + sq1 - sq1 >= r) {\n")
V["fam_work_dead"] = once(s, "                if (dist1 >= r) {\n", "                sq1 = 0;\n                if (dist1 >= r) {\n")
s = rd("idx")
V["fam_idx_chain_sph"] = once(s, "                r = SCR->rad[idx_sph];\n",
                              "                r = SCR->rad[idx_sph + idx_add - idx_add + idx_sub - idx_sub];\n")
V["fam_idx_chain_loop2"] = once(s, "idx_sub < nforce;", "idx_sub < nforce + idx_add - idx_add;")
V["fam_idx_dead"] = once(s, "        node[3] = (SCR->vel[0] * 7) >> 3;\n",
                         "        idx_add = 0;\n        idx_sub = 0;\n        node[3] = (SCR->vel[0] * 7) >> 3;\n")
s = rd("nbits")
V["fam_nbits_dead"] = once(s, "                    temp = (&D_8008D118)[work >> shift];\n",
                           "                    temp = (&D_8008D118)[work >> shift];\n                    lzcount = 0;\n")
V["fam_nbits_chain"] = once(s, "                    temp = (&D_8008D118)[work >> shift];\n",
                            "                    temp = (&D_8008D118)[work >> (shift + lzcount - lzcount)];\n")
for tag, text in V.items():
    open(D + tag + ".c", "w", newline="\n").write(text)
print(" ".join(V))
