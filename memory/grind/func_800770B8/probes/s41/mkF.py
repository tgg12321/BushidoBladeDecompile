"""mkF.py: ablation / split variants of F.c (= candidate.c) -> tmp/func_800770B8/F_<name>.c"""
import re
D = "tmp/func_800770B8/"
F = open(D + "F.c").read()


def cut_comment_and(stmt_regex, text):
    """drop a FAKE comment block immediately followed by the statement matching stmt_regex"""
    m = re.search(r"\n[ \t]*/\* FAKE:(?:(?!\*/).)*\*/\n[ \t]*" + stmt_regex + r"\n", text, re.S)
    assert m, stmt_regex
    return text[:m.start()] + "\n" + text[m.end():]


V = {}
V["norestore"] = cut_comment_and(r"work = list;", F)
V["nodowhile"] = cut_comment_and(r"do \{ \} while \(0\);", F)
V["noreset"] = cut_comment_and(r"row = D_800A35D0;", F)
# no row pointer at all (direct subscripts), re-set dropped
t = cut_comment_and(r"row = D_800A35D0;", F)
t = re.sub(r"\n[ \t]*/\* FAKE: typed row pointer(?:(?!\*/).)*\*/\n[ \t]*s16 \(\*row\)\[2\];\n", "\n", t, flags=re.S)
t = t.replace("        row = D_800A35D0;\n", "").replace("row[t0]", "D_800A35D0[t0]")
V["norow"] = t
# the split with F's exact statement list (one variable per value; only declarations / identifiers differ):
# `list` (s32 *) for the entry list pointer, `work` (void *) for the work area; no restore (nothing to restore)
s0 = F.replace("    void *work;\n", "    s32 *list;\n")
s0 = s0.replace("    work = (void *)(arg0 + 0x58);\n", "    list = (s32 *)(arg0 + 0x58);\n")
s0 = s0.replace("    func_8006E950(6, work);\n    r = func_80076FF8(work);\n",
                "    func_8006E950(6, list);\n    r = func_80076FF8(list);\n")
a = "        s32 *list = work;\n        work = (void *)func_8006E49C"
assert s0.count(a) == 1
s0 = s0.replace(a, "        void *work = (void *)func_8006E49C")
s0 = cut_comment_and(r"work = list;", s0)
V["split0"] = s0
# split with a SelWork-typed work pointer and the f04 store through it
s = s0.replace("        void *work = (void *)func_8006E49C(r, (s32 *)D_800A35D8);\n        D_800A36A0 = work;\n"
               "        SELWORK->f04 = list;\n",
               "        SelWork *work = (SelWork *)func_8006E49C(r, (s32 *)D_800A35D8);\n        D_800A36A0 = (u8 *)work;\n"
               "        work->f04 = list;\n")
assert s != s0
V["split"] = s
V["split_b"] = s.replace("        SelWork *work = (SelWork *)func_8006E49C(r, (s32 *)D_800A35D8);\n",
                         "        SelWork *work;\n        work = (SelWork *)func_8006E49C(r, (s32 *)D_800A35D8);\n")
# split where the clears go through work instead of re-reading the global
V["split_w"] = s.replace("        SELWORK->f30 = 0;\n        SELWORK->f34 = 0;\n    }\n    t0 = 0;",
                         "        work->f30 = 0;\n        work->f34 = 0;\n    }\n    t0 = 0;")
for k, v in V.items():
    assert v != F, k
    open(D + f"F_{k}.c", "w", newline="\n").write(v)
print(" ".join(D + f"F_{k}.c" for k in V))
