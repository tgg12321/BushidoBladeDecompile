"""Per-value spellings that need a FAKE-annotated construct (Ruling 11 (D) Q30 set-aside), measured for
the record. usage: python3 gen_fake.py <r11v dir> <outdir>"""
import os
import sys

NL = chr(10)
D = sys.argv[1]
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)
SA = open(os.path.join(D, "split_all.c")).read()
V2 = open(os.path.join(D, "only_V2.c")).read()


def must(b, old, new):
    assert b.count(old) == 1, old
    return b.replace(old, new)


E6 = "    *(s16 *)(obj + 0x1E6) = *(s16 *)(obj + 0x1E6) + d1e6 / 8;\n"
E8 = "    *(s16 *)(obj + 0x1E8) = *(s16 *)(obj + 0x1E8) + d1e8 / 8;\n"
EA = "        *(s16 *)(obj + 0x1EA) = *(s16 *)(obj + 0x1EA) + delta / 8;\n"
V = {}
# M1: dead-store family self-assign after each division (extends the variable's last reference)
b = must(SA, E6, E6 + "    d1e6 = d1e6;\n")
b = must(b, E8, E8 + "    d1e8 = d1e8;\n")
V["M1_selfassign_all"] = must(b, EA, EA + "        delta = delta;\n")
V["M1b_selfassign_onlyV2"] = must(V2, E8, E8 + "    d1e8 = d1e8;\n")
# M2: cancellation pair after each division (no-new-park-categories 2026-08-18 F6 family)
b = must(SA, E6, E6 + "    d1e6++;\n    d1e6--;\n")
b = must(b, E8, E8 + "    d1e8++;\n    d1e8--;\n")
V["M2_cancelpair_all"] = must(b, EA, EA + "        delta++;\n        delta--;\n")
V["M2b_cancelpair_onlyV2"] = must(V2, E8, E8 + "    d1e8++;\n    d1e8--;\n")
# M3: do-while(0) around the two function-level easing steps of the per-value body
i = SA.index("    d1e6 = (tgt_z")
j = SA.index(E8) + len(E8)
body = "".join("    " + l + NL if l else NL for l in SA[i:j].rstrip(NL).split(NL))
V["M3_dowhile_ease"] = SA[:i] + "    do {\n" + body + "    } while (0);\n" + SA[j:]
for n, b in V.items():
    open(os.path.join(OUT, n + ".c"), "w", newline=NL).write(b)
print("wrote", sorted(V))
