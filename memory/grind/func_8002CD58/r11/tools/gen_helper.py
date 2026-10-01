# inline sqrt-helper spellings (no reused local at all)
import os, re
D = os.path.dirname(os.path.abspath(__file__)) + "/v"
src = open(D + "/pv_all.c").read()
def block_end(s, i):
    j = s.index("{", i); depth = 0
    while True:
        c = s[j]
        if c == "{": depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0: break
        j += 1
    # include the else block
    k = j + 1
    if s[k:k+7] == " else {":
        return block_end(s, k)
    return k
def replace_sqrt(s, x, dst, expr_fmt="{dst} = isqrt({x});"):
    i = s.index("if ((u32)%s < 0x400) {" % x)
    e = block_end(s, i)
    return s[:i] + expr_fmt.format(dst=dst, x=x) + s[e:]
HELPER = '''static inline s32 isqrt(s32 x) {
    s32 slot;
    s32 lzcr;
    s32 shift;
    s32 tbl;
    if ((u32)x < 0x400) {
        return (u32)g_sqrt_table_u8[x] >> 3;
    }
    lzcr = 0;
    if (x >= 0) {
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            "nop\n"
            "nop\n"
            :: "r"(x) : "$12");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            :: "r"(&slot) : "$12", "memory");
        lzcr = slot;
    }
    shift = 0x16 - (lzcr & ~1);
    tbl = g_sqrt_table_u8[(u32)x >> shift];
    return (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
}
'''
HELPER2 = HELPER.replace("    s32 tbl;\n", "").replace("tbl = g_sqrt", "x = g_sqrt").replace("(tbl << 16)", "(x << 16)")
s = replace_sqrt(src, "len_sq", "dist")
s = replace_sqrt(s, "xz_sq", "xz_dist")
s = replace_sqrt(s, "nxz_sq", "nxz_dist")
for d in ("sp_tmp", "sp_tmp2", "sp_tmp3"):
    s = s.replace("    s32 %s;\n" % d, "", 1)
out = {"H1": HELPER + s, "H2": HELPER2 + s}
# H3: no dist locals at the ratan2 sites
s3 = s.replace("ratan2(*(s32 *)(obj + 0xAC), xz_dist)", "ratan2(*(s32 *)(obj + 0xAC), isqrt(xz_sq))")
s3 = s3.replace("xz_dist = isqrt(xz_sq);", "").replace("ratan2(*(s32 *)(obj + 0xCC), nxz_dist)", "ratan2(*(s32 *)(obj + 0xCC), isqrt(nxz_sq))").replace("nxz_dist = isqrt(nxz_sq);", "")
s3 = s3.replace("    s32 xz_dist;\n", "").replace("    s32 nxz_dist;\n", "")
out["H3"] = HELPER2 + s3
out["H4"] = HELPER + s3
for k, v in out.items():
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
