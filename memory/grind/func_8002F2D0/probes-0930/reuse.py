"""det/sum reuse and m00 ablations on <dir>/w_none.c -> <dir>/r_*.c"""
import sys
from pathlib import Path

d = sys.argv[1]
t = Path(f"{d}/w_none.c").read_bytes().decode()


def must(s, a, b, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, b)


SUMOLD = """            sum = *(((u8 *)&g_sqrt_table_u8) + ((u32)sum >> shift));
            det = (u32)(sum << 16) >> (0x13 - ((u32)shift >> 1));"""
SUMNEW = """            det = (u32)(*(((u8 *)&g_sqrt_table_u8) + ((u32)sum >> shift)) << 16) >> (0x13 - ((u32)shift >> 1));"""
V = {}
V["nosum"] = must(t, SUMOLD, SUMNEW)


def nodet(s):
    s = must(s, "    s32 sum;\n", "    s32 sum;\n    s32 dist;\n")
    s = must(s, "        det = (u32)*(((u8 *)&g_sqrt_table_u8) + sum) >> 3;", "        dist = (u32)*(((u8 *)&g_sqrt_table_u8) + sum) >> 3;")
    s = s.replace("            det = (u32)(", "            dist = (u32)(")
    s = must(s, "ang_y = ratan2(i2, det);", "ang_y = ratan2(i2, dist);")
    return s


V["nodet"] = nodet(t)
V["nosum_nodet"] = nodet(V["nosum"])
if "m00" in t:
    for k in list(V) + ["base"]:
        s = t if k == "base" else V[k]
        s2 = s.replace("    s32 m00;\n", "").replace("    m00 = m->m[0][0];\n", "").replace("m00 *", "m->m[0][0] *").replace("- m00 * ", "- m->m[0][0] * ")
        assert "m00" not in s2
        V[k + "_nom00"] = s2
for k, v in V.items():
    Path(f"{d}/r_{k}.c").write_bytes(v.encode())
print(sorted(V))
