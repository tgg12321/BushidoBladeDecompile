# func_80078EC0 (text1b_b.c) — WIP, floor 3 (HEAD 10), BLOCKED

## TL;DR (updated 2026-06-15)
Tiny 16-insn predicate: loads `p = D_8009BD88`, returns 1 iff `(p[1]&1)&&(p[0]&1)`.
Best clean form is the **three-return form** (candidate.c) at floor 3.
**Permuter confirmed byte-matching is possible** (found 0-score forms in <120s) — but
every 0-score form is a dead-code cheat. The cheat-reviewer gave FAIL on the cleanest
one (`p++; p--;` pointer no-op). 19 structural variants tested: all tie at floor 3.
The residual 3 is GCC's boolean fold — NOT a register or frame issue.

## The residual 3 (the whole gap)
GCC folds `if (p[0]&1) return 1; return 0;` → `return p[0]&1;` (boolean
if-to-return fold, jump.c `thread_jumps()`), emitting `andi; jr; nop` (13 insns).
Target keeps the verbose `bnez; addiu v0,0,1; addu v0,0,0` (16 insns).
p→$v1, ret→$v0, frame=none already MATCH. The ONLY gap is the fold.

## Resume
1. Apply candidate.c (three-return form); confirm sandbox score 3.
2. Try cc1 RTL dump (`-dr/-dR`) to identify the exact pass doing the fold.
3. Try cc1psx calibration to check if original compiler also folds.
4. Search for a sibling fn in text1b_b.c with a matched boolean-return pattern.

## Ruled out (do not re-derive)
- `ret=1; ret=0;` dead-store trick: FORBIDDEN.
- `p++; p--;` pointer no-op: CHEAT — cheat-reviewer FAIL 2026-06-15.
- Permuter dead-computation forms (uninitialized vars, `(double)` cast, `long long`).
- shared-end-label goto forms: still folds (floor 3).
- All 19 tested structural variants: named-tmp-reuse, goto forms, double-mask,
  separate-load, unsigned-cmp, sub-check, nested-if, precompute-both, ternary.
  Best: tie at floor 3. Worse: nested-if=4, ternary=4, precompute-both=11.

## Pointers
- rejected/ptr-noop-cheat.c — the cleanest 0-score form (p++/p--), FAIL
- rejected/dead-computation-cheat.c — permuter garbage forms
- `.claude/rules/shared-end-label.md` (tried — no help for this fold)
