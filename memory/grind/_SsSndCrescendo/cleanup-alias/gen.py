import re
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
src = open(ROOT + "/src/main.c").read()
s = src.index("void _SsSndCrescendo(s16 a0, s16 a1) {")
e = src.index("\n}\n", s) + 3
body = src[s:e]
i = body.index("    /* FAKE: named intermediate")
j = body.index("    u8 *base = ")
k = body.index("\n", j) + 1
nb = body[:i] + "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n" + body[k:]
nb = nb.replace("*(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98)", "SS_SCORE_FLAG(a0, a1)")
nb = nb.replace("*(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98)", "SS_SCORE_FLAG(a0, a1)")
assert "bank" not in nb, nb
open(ROOT + "/tmp/ff-worker/cres/v_macro.c", "w").write(nb)
open(ROOT + "/tmp/ff-worker/cres/base.c", "w").write(body)
# preamble-only variants on the landed Crescendo body (clear sites untouched)
PRE_S = body.index("    /* FAKE: named intermediate")
PRE_E = body.index("    u8 *base = ")
pre_body = lambda pre: body[:PRE_S] + pre + body[PRE_E:]
V = {
    "c_plus": "    s32 *bank = (s32 *)&_ss_score + a0;\n",
    "c_sub": "    s32 *bank = &((s32 *)&_ss_score)[a0];\n",
    "c_idxonly": "    s32 bank_no = a0;\n    s32 *bank = (s32 *)&_ss_score + bank_no;\n",
    "c_aliasonly": "    s32 *score_tbl = (s32 *)&_ss_score;\n    s32 *bank = score_tbl + a0;\n",
}
for n, p in V.items():
    open(ROOT + f"/tmp/ff-worker/cres/{n}.c", "w").write(pre_body(p))
# no bank, clears kept in their landed spellings
nb = body[:PRE_S] + "    u8 *base = (u8 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0);\n" + body[body.index("\n", PRE_E) + 1:]
nb = nb.replace("*bank", "((s32 *)&_ss_score)[a0]")
assert "bank" not in nb
open(ROOT + "/tmp/ff-worker/cres/c_nobank.c", "w").write(nb)
nb2 = nb.replace("*(s32 *)(((s16)a1 * 0xB0) + ((s32 *)&_ss_score)[a0] + 0x98)", "*(s32 *)(base + 0x98)")
open(ROOT + "/tmp/ff-worker/cres/c_nobank_base.c", "w").write(nb2)
