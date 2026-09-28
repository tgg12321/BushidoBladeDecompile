"""Build tmp/func_8001CE60/struct_tu.c: the landing code6cac.c with the score/tiebreak pairs declared as the banked struct form
(struct score_pair { u8 p1; u8 p2; }, as probes/calib_800340A0/A340_s.c) and func_8001CE60 spelled with member selects."""
import re
from pathlib import Path

sel = Path("tmp/func_8001CE60/sel_tu.c").read_text(encoding="utf-8")
decl = "extern u8 D_800A3898;\nextern u8 D_800A3899;\nextern u8 D_800A38AA;\nextern u8 D_800A38AB;\n"
assert sel.count(decl) == 1
sel = sel.replace(decl, "struct score_pair { u8 p1; u8 p2; };\nextern struct score_pair D_800A3898;\nextern struct score_pair D_800A38AA;\n")
fs = sel.index("void func_8001CE60(void) {")
fe = sel.index("\n}\n", fs) + 3
body = sel[fs:fe]
body = re.sub(r"\bD_800A3899\b", "D_800A3898.p2", body)
body = re.sub(r"\bD_800A38AB\b", "D_800A38AA.p2", body)
body = re.sub(r"\bD_800A3898\b(?!\.p2)", "D_800A3898.p1", body)
body = re.sub(r"\bD_800A38AA\b(?!\.p2)", "D_800A38AA.p1", body)
out = sel[:fs] + body + sel[fe:]
Path("tmp/func_8001CE60/struct_tu.c").write_bytes(out.encode("utf-8"))
Path("tmp/func_8001CE60/struct_body.c").write_bytes(body.encode("utf-8"))
print("ok", body.count(".p1"), body.count(".p2"))
