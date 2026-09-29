"""Phantom-slot producer census (phantom-slot-frame-lever.md) on the fully-written lz[2] form of c6:
does any ordinary-C spelling reserve the extra 16 bytes the original frame has (0x78 vs 0x68)?
Writes tmp/func_800187F4/fr_<name>.c; prints tags."""
import re

D = "tmp/func_800187F4/"
C6 = open(D + "c6.c").read()
B = C6.replace("    s32 lz[6];\n", "    s32 lz[2];\n")
V = {"fr_base_lz2": B, "fr_scalars": None}


def once(s, a, b):
    assert s.count(a) == 1, (a[:70], s.count(a))
    return s.replace(a, b)


# lz as two scalars
s = once(B, "    s32 lz[2];\n", "    s32 lz0, lz1;\n")
s = s.replace("&lz[0]", "&lz0").replace("&lz[1]", "&lz1").replace("= lz[0];", "= lz0;").replace("= lz[1];", "= lz1;")
V["fr_scalars"] = s
# producer 3: live named locals on multi-read fields (zero-cost if genuinely live)
V["fr_count_s16"] = once(B, "    s32 count;\n", "    s16 count;\n")
s = once(B, "        if (node[6] >= 0) {\n", "        state = node[6];\n        if (state >= 0) {\n")
s = s.replace("node[6]", "state").replace("        state = state;\n", "        state = node[6];\n")
s = once(s, "        s32 nforce;\n\n", "        s32 nforce;\n        s32 state;\n\n")
V["fr_state_s32"] = s
V["fr_state_s16"] = s.replace("        s32 state;\n", "        s16 state;\n")
s = once(B, "            for (idx = 0; idx < SCR->nsph; idx++) {\n", "            nsph = SCR->nsph;\n            for (idx = 0; idx < nsph; idx++) {\n")
V["fr_nsph_local"] = once(s, "            s32 depth;\n", "            s32 depth;\n            s32 nsph;\n")
V["fr_nsph_s16"] = V["fr_nsph_local"].replace("            s32 nsph;\n", "            s16 nsph;\n")
V["fr_nforce_s16"] = once(B, "        s32 nforce;\n", "        s16 nforce;\n")
V["fr_idx_s16"] = once(B, "        s32 idx;\n", "        s16 idx;\n")
V["fr_i_s16"] = once(B, "    s32 i;\n", "    s16 i;\n")
V["fr_bits_u8idx"] = B.replace("SCR->force[bits & 0xFF]", "SCR->force[(u8)bits]").replace("SCR->force[bits2 & 0xFF]", "SCR->force[(u8)bits2]")
# producer 1: loop-guard compares (rotated loops with an explicit guard on the loop's own exit test)
s = B
for cnt in ("nforce",):
    pass
s = once(B, "        for (idx = 0; idx < nforce; idx++) {\n            s32 *f_add;\n",
         "        idx = 0;\n        if (idx < nforce) do {\n            s32 *f_add;\n")
i = s.index("        if (idx < nforce) do {\n")
j = s.index("                bits >>= 8;\n            }\n        }\n", i) + len("                bits >>= 8;\n            }\n")
s = s[:j] + "        } while (++idx < nforce);\n" + s[j + len("        }\n"):]
V["fr_guard_forceloop1"] = s
s = once(B, "            for (idx = 0; idx < SCR->nsph; idx++) {\n", "            idx = 0;\n            if (idx < SCR->nsph) for (; idx < SCR->nsph; idx++) {\n")
V["fr_guard_sph"] = s
s = once(B, "    for (i = 0; i < count; i++, node += 16) {\n", "    i = 0;\n    if (i < count) for (; i < count; i++, node += 16) {\n")
V["fr_guard_node"] = s
s = once(B, "            if (depth > 0) {\n                if (depth > 0x3200) {\n",
         "            if (depth > 0) {\n                if (depth >= 0x3201) {\n")
V["fr_ground_ge"] = s
for tag, text in V.items():
    open(D + tag + ".c", "w", newline="\n").write(text)
print(" ".join(V))
