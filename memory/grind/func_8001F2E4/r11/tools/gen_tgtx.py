"""tgt_x constant-holder alternatives on the landing body (named-local-fake-exception prerequisite 1).
usage: python3 gen_tgtx.py <landing body .c> <outdir>"""
import os
import re
import sys

NL = chr(10)
B = open(sys.argv[1]).read()
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)
# drop the declaration comment (inert) so the variants read cleanly
B = re.sub(r"    /\* FAKE: constant-holder.*?\*/\n", "", B, flags=re.S)
arms = re.compile(r"\n *tgt_x = 0;")
assert len(arms.findall(B)) == 3
CALL1 = "    func_8002F770((s16 *)(a + 0x36),"
E1 = "    temp = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;\n"
V = {}
b = arms.sub("", B).replace("    s32 tgt_x;\n", "")
V["T1_literal0"] = b.replace("*(s16 *)(obj + 0x1E8), tgt_x);", "*(s16 *)(obj + 0x1E8), 0);")
V["T2_init_once"] = arms.sub("", B).replace("    s32 tgt_x;\n", "    s32 tgt_x = 0;\n")
V["T3_after_join"] = arms.sub("", B).replace(E1, "    tgt_x = 0;\n" + E1, 1)
V["T4_before_calls"] = arms.sub("", B).replace(CALL1, "    tgt_x = 0;\n" + CALL1, 1)
for n, b in V.items():
    assert b.count("tgt_x") in (0, 4, 3), (n, b.count("tgt_x"))
    open(os.path.join(OUT, n + ".c"), "w", newline=NL).write(b)
print("wrote", sorted(V))
