# func_8007C86C — WIP resume notes

## TL;DR

- **Floor 12** (HEAD floor 20). Identical structural shape to the sibling
  `func_8007C7A0` (constant 0xE4 vs 0xE3 differs).
- 4-session lever stack: clean v8 base + r_e4 named const + drop padding +
  **SOTN duplicate-read precompute (Y wide-mask)** (the last is the
  2026-06-02 SOTN-aligned policy unlock).
- Candidate body in `candidate.c`. Apply to `src/display.c`. Verify with
  `sandbox func_8007C86C --disable all` → expect score 12.
- Remaining gap: same `$a2`-vs-`$a3` X-preserve cascade as C7A0.
- Cheat-reviewer NOT yet invoked. Worker self-audit PASS; independent
  verdict owed.

## Joint progress with C7A0

This function and `func_8007C7A0` are siblings with identical 21-rule
patterns differing only in the GPU command word constant. The SOTN
duplicate-read lever that lowered C7A0's floor 16 → 12 ports directly here
and lowers this function's floor 20 → 12 with the same result. A lever
that closes one is highly likely to close the other; iterate them together.

See `memory/wip/func_8007C7A0/notes.md` for the structural cascade table.

## Sessions ledger

The full session-by-session evidence (~210 lines, all rejected variants,
mechanism explanation) is at:

- `memory/project/func-8007c86c-sotn-duplicate-read-lever.md` (session 4)
- `memory/project/func-8007c86c-clean-form-floor.md` (session 2 baseline)
- `memory/project/func-8007c86c-permuter-ub-cheat-findings.md` (session 3:
  ~5400 directed-permuter iters found only UB-conditional-init cheats — DO
  NOT re-run from the score-17 base; if running permuter, start from the
  score-12 base instead)

## How to resume in one read

1. Read `meta.json` — sessions[], reviewer verdict, rejected_forms,
   next_hypotheses.
2. Apply `candidate.c`'s body to `src/display.c`, replacing the existing
   func_8007C86C body.
3. `& tools/eng.ps1 sandbox func_8007C86C --disable all` → confirm score 12.
4. Pick a next_hypothesis. Consider iterating with C7A0 together since they
   share the structural ceiling.

## Session 2026-06-02 (workflow round 1)

7 fresh structural variants from the score-12 base + var_a1=arg1 preload
position-shift + OR-chain associativity reorderings. All confirm the score-12
floor (perturbations either match or regress). Direct disassembly diff vs
target re-confirms the structural-ceiling claim: idx 0 `move a2,a0` (mine) vs
`move a3,a0` (target), cascading through ~12 register-rotation diffs including
the missing idx-16 park-merge. cc1 ascending-allocator picks $a2; only UB or
literal-rename forms flip to $a3 in the explored space. New top next_hypothesis:
joint BB2_ALLOC_DEBUG dump on C86C + sibling C748 (3-arg matched) to compare
the pseudo's livelen/refs profile.

## Session 2026-06-02 (workflow round 2)

**Floor unchanged at 12, but MECHANISM FULLY CHARACTERIZED via BB2_ALLOC_DEBUG.**

Ran instrumented cc1 (tmp/gccdbg/cc1) on the score-12 form + matched siblings
C748 (3-arg) and C938 (2-arg dispatch). Tested 13 fresh structural variants.

**KEY FINDING — variant v6 (`s32 lim_x = D_8009BE78` block-local) is the FIRST
KNOWN LEVER that flips the C86C X-preserve pseudo to target's $a3:**

- Score-12 candidate: pseudo #72 (nrefs=2 livelen=11 pri=1818, ord=10 LAST) → $a2
- v6: new pseudo #91 (nrefs=3 livelen=7 pri=4285) → $a2 at ord=6; pseudo #76
  (X-preserve role, ord=8) flips to $a3 — **TARGET'S ALLOCATION ACHIEVED**
- v6 also produces target's missing idx-16 `move a3, v0` park-merge

BUT v6 scores 18 (regression) because lim_x's lifetime forces:
- (a) extra early `move $3, $4` preserve at idx 0 (lim_x's load forces arg0
  preservation earlier)
- (b) pseudo #77 shift from $a2 (target) to $a1
- Net: +6 cascade diffs

This is the score-12 → $a3-allocation pathway's existence proof. The mechanism
for flipping X-preserve to $a3 is now characterized: **introduce a pseudo with
priority > pseudo-72 but lifetime/refs that DON'T also bias pseudo 76/77's
allocation.**

**Sibling ALLOCDBG corroboration:**
- C748 has only 5 pseudos; args take $a0/$a1/$a2 naturally — fundamentally
  different shape
- C938 has only 4 pseudos and uses `new_var2 = arg0` as SOTN-allowed
  named-intermediate (works because no X-clamp clobbers $a0)
- C86C's X-preserve is structurally necessary: target's idx-12-14 reuses $a0
  for `lim_x - 1` then re-reads preserve in branch delay slot at idx 14
  `addu v0, a3, $zero`

New top next_hypothesis: PERMUTER FROM v6 BASE (score-18 lim_x form) with
PERM_GENERAL directed on the prologue region, ~30k iters. Vet output strictly
against the cheat catalog. Alternatively, micro-variant sweep on lim_x SHAPE
(outer-scope, s16 retype, explicit cast, lim_x-1 directly, lim_x read-once for
comparison only). Joint with C7A0: v6 lever should produce the same ALLOCDBG
flip there (constant 0xE3 vs 0xE4 is the only difference).
