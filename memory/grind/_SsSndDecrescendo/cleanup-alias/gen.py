import os, re
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT = ROOT + "/tmp/ff-worker/decres"
src = open(ROOT + "/src/main.c").read().split("\n")
# lines 371..428 (1-based) = the function
body = "\n".join(src[370:428]) + "\n"
PRE = """    s32 bank_no = a0;
    s32 *score_tbl = (s32 *)&_ss_score;
    s32 *bank = score_tbl + bank_no;
"""
assert PRE in body, "preamble not found"
variants = {
    "base": PRE,
    "v_plus": "    s32 *bank = (s32 *)&_ss_score + a0;\n",
    "v_sub": "    s32 *bank = &((s32 *)&_ss_score)[a0];\n",
    "v_bytes": "    s32 *bank = (s32 *)((u8 *)&_ss_score + a0 * 4);\n",
    "v_s32cast": "    s32 *bank = (s32 *)&_ss_score + (s32)a0;\n",
    "v_idxonly": "    s32 bank_no = a0;\n    s32 *bank = (s32 *)&_ss_score + bank_no;\n",
    "v_aliasonly": "    s32 *score_tbl = (s32 *)&_ss_score;\n    s32 *bank = score_tbl + a0;\n",
    "v_alias_first": "    s32 *score_tbl = (s32 *)&_ss_score;\n    s32 bank_no = a0;\n    s32 *bank = score_tbl + bank_no;\n",
}
for name, pre in variants.items():
    open(f"{OUT}/{name}.c", "w").write(body.replace(PRE, pre))
B2 = "    u8 *base = (u8 *)(*bank + (s16)a1 * 0xB0);\n    u16 voll, volr;\n"
assert PRE + B2 in body
DECL = "    s32 *bank;\n    u8 *base;\n    u16 voll, volr;\n\n"
TAIL = "    base = (u8 *)(*bank + (s16)a1 * 0xB0);\n"
two = {
    "v_twostmt": DECL + "    bank = (s32 *)&_ss_score;\n    bank += a0;\n" + TAIL,
    "v_twostmt_idx": "    s32 bank_no = a0;\n" + DECL + "    bank = (s32 *)&_ss_score;\n    bank += bank_no;\n" + TAIL,
    "v_assign_order": "    s32 bank_no;\n" + DECL + "    bank_no = a0;\n    bank = (s32 *)&_ss_score + bank_no;\n" + TAIL,
    "v_sotn_score": "    s32 *bank;\n    u8 *base;\n    u16 voll, volr;\n\n    bank = &((s32 *)&_ss_score)[a0];\n" + TAIL,
    "v_base_direct": "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n    s32 *bank = (s32 *)&_ss_score + a0;\n    u16 voll, volr;\n",
}
two["v_base_direct_amp"] = "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n    s32 *bank = &((s32 *)&_ss_score)[a0];\n    u16 voll, volr;\n"
two["v_base_direct_nocast"] = "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + a1 * 0xB0);\n    s32 *bank = (s32 *)&_ss_score + a0;\n    u16 voll, volr;\n"
for name, pre in two.items():
    open(f"{OUT}/{name}.c", "w").write(body.replace(PRE + B2, pre))
variants.update(two)
# no bank local at all: every *bank read spelled as the table element
nb = body.replace(PRE + B2, "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n    u16 voll, volr;\n")
nb = nb.replace("*bank", "((s32 *)&_ss_score)[a0]")
open(f"{OUT}/v_nobank.c", "w").write(nb)
nb2 = body.replace(PRE + B2, "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n    u16 voll, volr;\n")
nb2 = nb2.replace("(((s16)a1 * 0xB0) + *bank + 0x98)", "(base + 0x98)")
open(f"{OUT}/v_nobank_base.c", "w").write(nb2)
print("ok", len(variants))
mac = nb.replace("*(s32 *)(((s16)a1 * 0xB0) + ((s32 *)&_ss_score)[a0] + 0x98)", "SS_SCORE_FLAG(a0, a1)")
mac = mac.replace("*(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98)", "SS_SCORE_FLAG(a0, a1)")
open(f"{OUT}/v_macro.c", "w").write(mac)
print("macro sites", mac.count("SS_SCORE_FLAG"))
