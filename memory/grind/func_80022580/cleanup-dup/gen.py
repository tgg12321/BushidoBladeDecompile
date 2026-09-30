"""Variants of func_80022580's spawn-position dispatch; everything else is the src body verbatim."""
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
import os, re
OUT = os.path.dirname(os.path.abspath(__file__))
src = open(ROOT + "/src/code6cac_tu2.c").read()
s = src.index("void func_80022580(s32 idx, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {")
e = src.index("\n}\n", s) + 3
body = src[s:e]
# the landed body carries a FAKE comment on case 0's copy; strip it to get the plain dispatch
body = re.sub(r"\n +/\* FAKE: default's two func_80021D10 calls.*?\*/", "", body, flags=re.S)
SW = """    switch (D_800A38DC) {
    case 0:
        if (arg4 != 0) {
            other = p->unk_00->unk_D8;
            func_80022224(idx, &p->unk_D8.x, &other.x);
        } else {
            func_80021D10(idx, &p->unk_D8.x, slot);
            func_80021D10(idx == 0, &other.x, slot);
        }
        break;
    case 2:
    case 3:
        if (arg4 != 0) {
            other = p->unk_00->unk_D8;
            func_80021DB0(idx, &p->unk_D8, &other.x);
        } else {
            func_80021D10(idx, &p->unk_D8.x, D_800A38E0);
            func_80021D10(idx == 0, &other.x, D_800A38E0);
        }
        break;
    default:
        func_80021D10(idx, &p->unk_D8.x, slot);
        func_80021D10(idx == 0, &other.x, slot);
        break;
    }
"""
assert SW in body
C0W = """            other = p->unk_00->unk_D8;
            func_80022224(idx, &p->unk_D8.x, &other.x);
"""
C23 = """        if (arg4 != 0) {
            other = p->unk_00->unk_D8;
            func_80021DB0(idx, &p->unk_D8, &other.x);
        } else {
            func_80021D10(idx, &p->unk_D8.x, D_800A38E0);
            func_80021D10(idx == 0, &other.x, D_800A38E0);
        }
"""
DEF = """        func_80021D10(idx, &p->unk_D8.x, slot);
        func_80021D10(idx == 0, &other.x, slot);
"""
V = {}
V["base"] = SW
V["fallthrough"] = ("    switch (D_800A38DC) {\n    case 2:\n    case 3:\n" + C23 + "        break;\n"
    "    case 0:\n        if (arg4 != 0) {\n" + C0W + "            break;\n        }\n    default:\n" + DEF + "        break;\n    }\n")
V["fallthrough_c0first"] = ("    switch (D_800A38DC) {\n    case 0:\n        if (arg4 != 0) {\n" + C0W + "            break;\n        }\n"
    "        goto dflt;\n    case 2:\n    case 3:\n" + C23 + "        break;\n    default:\n    dflt:\n" + DEF + "        break;\n    }\n")
V["ifchain_then_switch"] = ("    if (D_800A38DC == 0 && arg4 != 0) {\n" + C0W.replace("            ", "        ") + "    } else {\n"
    "        switch (D_800A38DC) {\n        case 2:\n        case 3:\n" + C23.replace("\n        ", "\n            ").replace("        if", "            if", 1) +
    "            break;\n        default:\n" + DEF.replace("        ", "            ") + "            break;\n        }\n    }\n")
V["ifchain"] = ("    if (D_800A38DC == 0 && arg4 != 0) {\n" + C0W.replace("            ", "        ") +
    "    } else if (D_800A38DC == 2 || D_800A38DC == 3) {\n" + C23.replace("\n        ", "\n    ").replace("        if", "        if", 1).replace("    }\n", "    }\n") +
    "    } else {\n" + DEF + "    }\n")
V["cond_break"] = ("    switch (D_800A38DC) {\n    case 0:\n        if (arg4 == 0) {\n            goto dflt;\n        }\n" + C0W.replace("            ", "        ") + "        break;\n"
    "    case 2:\n    case 3:\n" + C23 + "        break;\n    default:\n    dflt:\n" + DEF + "        break;\n    }\n")
for n, sw in V.items():
    open(f"{OUT}/{n}.c", "w").write(body.replace(SW, sw))
print("ok", list(V))
ANN = """        } else {
            /* FAKE: default's two func_80021D10 calls repeated in this arm
             * (duplicated-statement-into-arms; calls byte-identical, owner
             * Q47): cross-jump merges the copies (no extra jal). Mechanism:
             * block layout. With the copy here the dispatch tests mode == 0
             * first, places case 0's block after the range tests and reaches
             * case 2/3 through the target's `j` (asm/funcs/func_80022580.s
             * :250-251); every shared form measured lays the dispatch out
             * in another order, lever-exhaustion: memory/grind/func_80022580/
             * evidence.md "Cleanup 2026-09-30" (16-21). */
            func_80021D10(idx, &p->unk_D8.x, slot);
            func_80021D10(idx == 0, &other.x, slot);
        }
        break;
    case 2:"""
PLAIN = """        } else {
            func_80021D10(idx, &p->unk_D8.x, slot);
            func_80021D10(idx == 0, &other.x, slot);
        }
        break;
    case 2:"""
assert body.count(PLAIN) == 1
open(f"{OUT}/annotated.c", "w").write(body.replace(PLAIN, ANN))
adj = ("    switch (D_800A38DC) {\n    case 0:\n        if (arg4 != 0) {\n" + C0W + "            break;\n        }\n"
    "    default:\n" + DEF + "        break;\n    case 2:\n    case 3:\n" + C23 + "        break;\n    }\n")
open(f"{OUT}/c0_default_adjacent.c", "w").write(body.replace(SW, adj))
# mirror label placement: default jumps into case 0's copy (the only copy)
mir = ("    switch (D_800A38DC) {\n    case 0:\n        if (arg4 != 0) {\n" + C0W + "        } else {\n        c0_calls:\n" +
    DEF.replace("        ", "            ") + "        }\n        break;\n    case 2:\n    case 3:\n" + C23 + "        break;\n"
    "    default:\n        goto c0_calls;\n    }\n")
open(f"{OUT}/default_goto_case0.c", "w").write(body.replace(SW, mir))
