"""Per-local ablation + ordinary respellings of func_80071C4C's two tail copy loops.
base.c = the landed body (extracted from src/text1b.c)."""
import os
base = open("tmp/cleanup/1c4c/base.c", encoding="utf-8").read()

LA = """        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
            s32 dst = i * 10;
            s32 ctx = i * 3;

            *(u8 *)(D_800A3568 + dst) = D_800A3560[ctx];
        }
"""
LB = """        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
            s32 dst = i * 10;
            s32 ctx = i * 3;

            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[ctx + 2];
        }
"""
assert LA in base and LB in base


def loopA(body, decls=""):
    return ("        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {\n" + decls + body + "        }\n")


def loopB(body, decls=""):
    return ("        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {\n" + decls + body + "        }\n")


D = "            s32 dst = i * 10;\n"
C = "            s32 ctx = i * 3;\n"
BL = "\n"
A_ = {  # (decls, store) for loop A with each local kept/inlined
    (1, 1): (D + C + BL, "            *(u8 *)(D_800A3568 + dst) = D_800A3560[ctx];\n"),
    (0, 1): (C + BL, "            *(u8 *)(D_800A3568 + i * 10) = D_800A3560[ctx];\n"),
    (1, 0): (D + BL, "            *(u8 *)(D_800A3568 + dst) = D_800A3560[i * 3];\n"),
    (0, 0): ("", "            *(u8 *)(D_800A3568 + i * 10) = D_800A3560[i * 3];\n"),
}
B_ = {
    (1, 1): (D + C + BL, "            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[ctx + 2];\n"),
    (0, 1): (C + BL, "            *(u8 *)(D_800A3568 + i * 10 + 1) = D_800A3560[ctx + 2];\n"),
    (1, 0): (D + BL, "            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[i * 3 + 2];\n"),
    (0, 0): ("", "            *(u8 *)(D_800A3568 + i * 10 + 1) = D_800A3560[i * 3 + 2];\n"),
}
variants = {}
for ka, (da, sa) in A_.items():
    for kb, (db, sb) in B_.items():
        name = "A%d%d_B%d%d" % (ka + kb)
        variants[name] = base.replace(LA, loopA(sa, da)).replace(LB, loopB(sb, db))

# ordinary respellings with no locals
def both(sa, sb):
    return base.replace(LA, loopA(sa)).replace(LB, loopB(sb))

variants["r_u8idx"] = both(
    "            ((u8 *)D_800A3568)[i * 10] = D_800A3560[i * 3];\n",
    "            ((u8 *)D_800A3568)[i * 10 + 1] = D_800A3560[i * 3 + 2];\n")
variants["r_u8idx_b1"] = both(
    "            ((u8 *)D_800A3568)[i * 10] = D_800A3560[i * 3];\n",
    "            ((u8 *)D_800A3568 + 1)[i * 10] = D_800A3560[i * 3 + 2];\n")
variants["r_sym2"] = both(
    "            *(u8 *)(D_800A3568 + i * 10) = D_800A3560[i * 3];\n",
    "            *(u8 *)(D_800A3568 + i * 10 + 1) = D_800A3562[i * 3];\n")
variants["r_3i"] = both(
    "            *(u8 *)(D_800A3568 + 10 * i) = D_800A3560[3 * i];\n",
    "            *(u8 *)(D_800A3568 + 10 * i + 1) = D_800A3560[3 * i + 2];\n")
variants["r_rec"] = both(
    "            ((u8 (*)[10])D_800A3568)[i][0] = ((u8 (*)[3])D_800A3560)[i][0];\n",
    "            ((u8 (*)[10])D_800A3568)[i][1] = ((u8 (*)[3])D_800A3560)[i][2];\n")
variants["r_rec_dstonly"] = both(
    "            ((u8 (*)[10])D_800A3568)[i][0] = D_800A3560[i * 3];\n",
    "            ((u8 (*)[10])D_800A3568)[i][1] = D_800A3560[i * 3 + 2];\n")

os.makedirs("tmp/cleanup/1c4c/v", exist_ok=True)
for k, t in variants.items():
    open(f"tmp/cleanup/1c4c/v/{k}.c", "w", newline="\n").write(t)
print(",".join(f"tmp/cleanup/1c4c/v/{k}.c" for k in variants))

# --- round 2: exhaustion probes ---
extra = {}
extra["x_swaporder"] = base.replace(LA, loopA(A_[(1, 1)][1], C + D + BL)).replace(LB, loopB(B_[(1, 1)][1], C + D + BL))
extra["x_embed_ctx"] = both(
    "            *(u8 *)(D_800A3568 + i * 10) = D_800A3560[ctx = i * 3];\n",
    "            *(u8 *)(D_800A3568 + i * 10 + 1) = D_800A3560[(ctx = i * 3) + 2];\n").replace(
    "        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {\n",
    "        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {\n            s32 ctx;\n\n").replace(
    "        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {\n            *(u8 *)(D_800A3568 + i * 10 + 1)",
    "        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {\n            s32 ctx;\n\n            *(u8 *)(D_800A3568 + i * 10 + 1)")
extra["x_recptr"] = base.replace(LA, loopA("            *rec = D_800A3560[ctx];\n", "            u8 *rec = (u8 *)(D_800A3568 + i * 10);\n" + C + BL)).replace(
    LB, loopB("            rec[1] = D_800A3560[ctx + 2];\n", "            u8 *rec = (u8 *)(D_800A3568 + i * 10);\n" + C + BL))
extra["x_rec2d_ctx"] = base.replace(LA, loopA("            ((u8 (*)[10])D_800A3568)[i][0] = D_800A3560[ctx];\n", C + BL)).replace(
    LB, loopB("            ((u8 (*)[10])D_800A3568)[i][1] = D_800A3560[ctx + 2];\n", C + BL))
extra["x_srcptr"] = both(
    "            *(u8 *)(D_800A3568 + i * 10) = *(D_800A3560 + i * 3);\n",
    "            *(u8 *)(D_800A3568 + i * 10 + 1) = *(D_800A3560 + i * 3 + 2);\n")
for k, t in extra.items():
    open(f"tmp/cleanup/1c4c/v/{k}.c", "w", newline="\n").write(t)
print(",".join(f"tmp/cleanup/1c4c/v/{k}.c" for k in extra))
