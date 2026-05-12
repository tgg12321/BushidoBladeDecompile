#!/usr/bin/env python3
"""Class-aware subagent prompt generator. Smaller, more focused
prompts than the default template — typically 40-80% fewer tokens
per spawn.

The default `tools/gen_subagent_prompt.py` emits the same ~150-line
prompt regardless of function shape. This generator detects the
function class from the asm and includes only the gotchas/recipes
relevant to that class.

Classes:
  - gte_wrapper       small (< 30 insn), heavy cop2 (mtc2/lwc2/...)
                      ops. Recipe: clone latest GTE-wrapper commit,
                      use .word for cop2 ops, watch the audit traps.
  - state_dispatch    standard, has multiple branches on the same
                      register. Recipe: explicit-goto dispatch.
  - leaf_arith        small (< 30 insn), no jal, no cop2. Recipe:
                      direct pure-C, try `dc.sh attempt` first.
  - branch_heavy      large + many branches. Recipe: read asm, write
                      goto-skeleton, iterate regfix.
  - inline_asm_dom    the source file is already mostly inline-asm
                      blocks (e.g., text1b.c, code6cac_*). Recipe:
                      acceptable to retire to file-scope __asm__ if
                      C cannot express the custom ABI.
  - generic           fallback when no class signal is strong.

Usage:
    python3 tools/gen_subagent_prompt_smart.py <func>
    bash tools/dc.sh subagent-prompt-smart <func>
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC = ROOT / "src"

COP2_OPS = re.compile(r"\b(mfc2|cfc2|mtc2|ctc2|lwc2|swc2|rtps|rtpt|gpf|gpl|"
                      r"mvmva|avsz3|avsz4|sqr|op|dpct|dpcl|intpl|nclip|"
                      r"ncds|nccs|cdp|dcpl|nce|ncs|ncds|ncct)\b")
JAL_RE = re.compile(r"\bjal\b")
BRANCH_RE = re.compile(r"\b(beq|bne|beqz|bnez|bltz|bgtz|blez|bgez)\b")


def classify_function(func: str, asm_text: str, src_path: Path | None) -> str:
    """Return the class string."""
    if not asm_text:
        return "generic"
    insns = sum(1 for line in asm_text.splitlines() if "/* " in line and " */" in line)
    n_cop2 = len(COP2_OPS.findall(asm_text))
    n_jal = len(JAL_RE.findall(asm_text))
    n_branches = len(BRANCH_RE.findall(asm_text))

    # GTE wrapper heuristic: small + cop2-heavy
    if insns <= 40 and n_cop2 >= 3:
        return "gte_wrapper"

    # Check inline-asm-dominant file
    if src_path is not None and src_path.exists():
        try:
            text = src_path.read_text(encoding="utf-8", errors="ignore")
            asm_block_count = text.count("__asm__(")
            if asm_block_count >= 30:
                return "inline_asm_dom"
        except Exception:
            pass

    # State dispatch: many branches on similar comparisons in a small frame
    if 30 <= insns <= 100 and n_branches >= 6 and n_jal <= 2:
        return "state_dispatch"

    if insns < 30 and n_jal == 0:
        return "leaf_arith"

    if insns >= 60 and n_branches >= 10:
        return "branch_heavy"

    return "generic"


def find_src_for_func(func: str) -> Path | None:
    func_def_re = re.compile(
        rf"^[A-Za-z_][\w *]*?\s\**{re.escape(func)}\s*\([^)]*\)\s*\{{", re.MULTILINE)
    glabel_re = re.compile(rf"glabel\s+{re.escape(func)}\b")
    for p in sorted(SRC.glob("*.c")):
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        if func_def_re.search(text) or glabel_re.search(text):
            return p
    return None


PROMPT_HEAD = """You are working {func} to byte-match in 100% pure C.

# Active marker / commit gate
The harness has set `.bb2_active_func={func}`. The hook blocks `git
commit` until `dc.sh verify-c {func}` (bridge-aware) reports MATCH on
a clean rebuild. No re-bridging. No inline asm unless original was
hand-written assembly.

# Class: {cls}

"""

CLASS_BODIES = {
    "gte_wrapper": """\
This is a small GTE-coprocessor wrapper. Common recipe:
  1. `dc.sh preflight {func}` — confirm bridge state + size + sibling commits
  2. `dc.sh retire {func}` — comment the bridge
  3. Find the most recent GTE-wrapper commit (from preflight's sibling
     list) and CLONE its C body, swapping opcodes/operands.
  4. Build with `dc.sh build-active {func}` (~30s incremental).
  5. `dc.sh verify-c {func}` — must say MATCH.
  6. Commit.

GTE wrapper gotchas (see memory/feedback_retirement_recipes.md):
  - Encode cop2 ops as `.word 0xXXXXXXXX` inside `__asm__ volatile`.
    Use the mnemonic form ONLY for ops where you need register-asm
    bindings honored — `.word` ignores `"=r"` operand placement.
  - For placement-control register moves, use `move %0, %1` not
    `addu %0, %1, $zero`. The audit's SUSPECT regex catches `addu`
    and reclassifies the function as inline_asm_debt.
  - In display.c specifically, avoid MATRIX/SVECTOR/VECTOR typedefs
    in signatures — they parse-error silently before the relevant
    typedef is in scope. Use bare s32*/s16*/u8*.
""",
    "state_dispatch": """\
Multi-branch dispatcher (state machine, validate-and-call, etc.).
GCC's natural `if (cond) {{ body }}` emits `bne !cond, skip; body;
skip:` — i.e., body falls through, branch skips. But many target
asms instead use `beq cond, label; ... fall through to default; label:
body;` (jump TO the body if cond, fall through to default).

Recipe — explicit-goto dispatch:
  ```
  if (state == 1) goto handler_1;
  if (state == 2) goto handler_2;
  goto end;       /* explicit; do NOT just `return` here */
  handler_1: ... goto end;
  handler_2: ... goto end;
  end: return;
  ```
This produces target's `beq state, K, .Lhandler / nop / j .Lend / nop`
pattern naturally.

Workflow:
  1. `dc.sh preflight {func}` — bridge state + similar siblings
  2. `dc.sh retire {func}`
  3. Write C using goto-dispatch
  4. `dc.sh build-active {func}` → `dc.sh diff-summary {func}`
  5. Iterate (regfix for register/scheduling, edit C for structural)
  6. `dc.sh verify-c {func}` → commit
""",
    "leaf_arith": """\
Small arithmetic/data-movement leaf. Almost always matches via direct
pure-C transcription, sometimes with a single regfix subst.

Workflow:
  1. `dc.sh preflight {func}`
  2. `dc.sh retire {func}`
  3. `dc.sh attempt {func}` (auto-pipeline) — often MATCHES immediately
  4. If not: write C body manually, `dc.sh build-active`,
     `dc.sh diff-summary`, fix the remaining 1-3 diffs.
  5. `dc.sh verify-c {func}` → commit
""",
    "branch_heavy": """\
Substantial decomp (many branches, possibly nested loops). Plan for
30-90 min of work.

Workflow:
  1. `dc.sh preflight {func}` — note kengo equivalent if present
  2. Read the full asm carefully. Sketch the C control flow on paper
     (or in a comment block in the file).
  3. `dc.sh retire {func}`
  4. Write C with explicit gotos matching the asm's branch structure.
  5. `dc.sh build-active {func}` → `dc.sh diff-summary {func}`
  6. Most likely first miss: register allocation differs. Read
     `dc.sh diff {func}` carefully, classify each diff (opcode, reg,
     immediate, scheduling), apply targeted regfix rules.
  7. `dc.sh verify-c {func}` → commit
""",
    "inline_asm_dom": """\
This function lives in a file dominated by inline-asm blocks (e.g.
text1b.c, code6cac_*). The original was hand-written assembly. The
89-sibling convention in this file is **in-file __asm__()
retirement** — per project policy, inline asm is acceptable when the
canonical source was assembly.

If pure-C decomp is tractable, do that. If you hit a wall (custom
ABI like `$s0`-as-unsaved-input, register pressure GCC can't express,
etc.), do NOT grind for hours. Inline-asm retirement (file-scope
`__asm__()` block replacing the bridge) is the established fallback.
Pattern is identical to func_8004A76C (commit bc04b47).

Workflow:
  1. `dc.sh preflight {func}`
  2. `dc.sh retire {func}`
  3. Try pure C first (build-active + diff-summary loop, 30 min max)
  4. If genuinely stuck (custom ABI, etc.), STOP. Either:
     a) `git commit` with file-scope __asm__() body (project policy
        allows this for original-asm functions) AND add the function
        to inline_asm_canonical.txt
     b) Return STUCK to coordinator if uncertain.
""",
    "generic": """\
No specific recipe class detected. Use the general retirement
workflow:

  1. `dc.sh preflight {func}`
  2. `dc.sh retire {func}`
  3. Write or iterate C body
  4. `dc.sh build-active {func}` → `dc.sh diff-summary {func}`
  5. `dc.sh verify-c {func}` → commit

Read memory/feedback_retirement_recipes.md for the 7 known gotchas
before starting — most failures are repeats of those.
""",
}

PROMPT_TAIL = """

# Build/verify discipline
  - Iterate with `dc.sh build-active {func}` (incremental, ~30s).
  - `dc.sh verify-c {func}` is bridge-aware and REFUSES if the bridge
    is still active. Use it, NOT `dc.sh verify {func}`.
  - The commit hook does a clean rebuild + bridge check automatically.
    If it refuses, fix the underlying issue — never `--no-verify`.

# Communication
Work silently. One-line final report:

  MATCHED `{func}` at commit <sha>. Recipe: <one-line>. Retro: NONE or <sha>.
  STUCK `{func}` after <techniques>. Best diff: <description>. Suggested: <next move>.
"""


def main() -> int:
    args = sys.argv[1:]
    if not args or args[0] in ("-h", "--help"):
        print(__doc__, file=sys.stderr)
        return 1
    func = args[0]
    asm_path = ASM_FUNCS / f"{func}.s"
    asm_text = asm_path.read_text(encoding="utf-8", errors="ignore") if asm_path.exists() else ""
    src_path = find_src_for_func(func)
    cls = classify_function(func, asm_text, src_path)

    head = PROMPT_HEAD.format(func=func, cls=cls)
    body = CLASS_BODIES.get(cls, CLASS_BODIES["generic"]).format(func=func)
    tail = PROMPT_TAIL.format(func=func)
    print(head + body + tail)
    return 0


if __name__ == "__main__":
    sys.exit(main())
