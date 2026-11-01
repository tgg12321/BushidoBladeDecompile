"""wave117 rows (func-second-pass-25): func_80046E8C, revived from NO_NAME (w49,w61,w70,w86,w116) on var-globals-q
(7e7b2f947: D_800A3790 -> g_gpu_draw_option_mask). Research input tmp/codex/func-second-pass-25-research.md (untrusted);
every code claim re-derived at HEAD 26f5d2d5f.

Re-derived code facts:
- func_80046E8C (src/main/368E4.c:543; asm/funcs/func_80046E8C.s:1-7): `g_gpu_draw_option_mask = 0x23;` and return. No argument.
- No caller: git grep 80046E8C over src, include, asm hits only the definition and its glabel / endlabel.
- func_80046B44 (368E4.c:429-448) stores the same 0x23 at :446 among its (re)init stores; it is called at boot (6CF8.c:300) and
  from the post-movie reinit (2B344.c:691). These two are the mask's only writers, and 0x23 is the only value stored.
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 1/1 (fresh default-FAIL reviewer, lane naming-a 2026-11-01)"
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


def IA(fn, f):
    """src/main/<file>:<line> of the INCLUDE_ASM line of fn."""
    for k, l in enumerate(_SRC[f]):
        if l.startswith("INCLUDE_ASM") and re.search(r'\b%s\)' % fn, l):
            return f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no INCLUDE_ASM of {fn}")


_RESET = (
    f"BODY: {B('func_80046E8C', '368E4.c')}: `g_gpu_draw_option_mask = 0x23;` (asm/funcs/func_80046E8C.s:2-4: addiu v0,0x23; sw; jr ra), "
    "no argument, no return value. "
    f"OTHER WRITER: func_80046B44 ({B('func_80046B44', '368E4.c')}) stores the identical 0x23 among its (re)init stores (368E4.c:446), "
    "called at boot (src/main/6CF8.c:300) and from the post-movie reinit (src/main/2B344.c:691); no other store to the mask exists "
    "(every other access is a load), so 0x23 is its init and only value. "
    "USES: none (no jal / j / .word / table entry; git grep 80046E8C over src, include, asm: definition only). "
    "WHY: Reset = writes back the (re)init value func_80046B44 sets, with no argument (precedent menu_ResetRowBitCursors 596CC.c:2246-2249, "
    "a single store of a nonzero default); DrawOptionMask = the landed noun of g_gpu_draw_option_mask (var-globals-q); gpu_ = the global's "
    "prefix and every reader's (gpu_PrepareViewAndFlushDrawQueue, gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets, the draw-queue walker). "
    "Not Init: func_80046B44 is the (re)init, and this is one store of it. "
    "COUNTER-EVIDENCE: uncalled dead code (never hit in any capture), so its purpose is unshown; the name states only the store (code-only, "
    "no trace words, Q124). Bit 5 of 0x23 has no reader; the name makes no per-bit claim. The retired named_syms note "
    "'game_ResetTimer' (named_syms.txt:159, a wrong timer reading) is not evidence and the name is not reused."
)

FUNCS = [
    ("0x80046E8C", "gpu_ResetDrawOptionMask", "RENAME", _RESET),
]

DATA = []
