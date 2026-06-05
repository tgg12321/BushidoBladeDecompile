# func_8007C7A0 — WIP resume notes

## TL;DR

- **Floor 12** (HEAD floor 16). 4-session lever stack: arg1-preload +
  r_e3 named const + drop coercion vars + **SOTN duplicate-read precompute
  (Y wide-mask)** — the last is the new lever from 2026-06-02 post-policy.
- Candidate body in `candidate.c`. Apply to `src/display.c`. Verify
  with `sandbox func_8007C7A0 --disable all` → expect score 12.
- Remaining gap: `$a2`-vs-`$a3` X-preserve register tiebreaker cascade.
  No legitimate (non-cheat) lever flips it yet.
- Cheat-reviewer NOT yet invoked on the score-12 form. Worker self-audit
  is PASS but independent verdict is owed before any Match: commit.

## How to resume in one read

1. Read `meta.json` — sessions[], reviewer verdict, rejected forms,
   next_hypotheses.
2. `cp` (or hand-copy) `candidate.c`'s body into `src/display.c`, replacing
   the existing func_8007C7A0 body.
3. `& tools/eng.ps1 sandbox func_8007C7A0 --disable all` — confirm score 12.
4. Pick a next_hypothesis from `meta.json` and iterate.
5. **DO NOT** add a `raw_arg0 = arg0` literal rename (drops to 10 but is
   forbidden — see `rejected_forms[0]` in `meta.json`).

## What was tried — deeper context

The full session-by-session ledger (~250 lines, all rejected variants, full
mechanism explanation for why the score-12 shape exists) is at
`memory/project/func-8007c7a0-sotn-duplicate-read-lever.md`. The earlier
sessions' notes are linked from there.

## The structural ceiling

cc1's default ascending allocation order picks `$a2` for the X preserve when
both `$a2` and `$a3` are free. Target picks `$a3`. Every legitimate C lever
tested leaves the choice unchanged:

| idx | mine (score 12)        | target              | nature |
|---|---|---|---|
| 0  | `move a2,a0`            | `move a3,a0`         | X preserve reg |
| 8  | `move v1,v0`            | `move a2,v0`         | X-limit save |
| 16 | (MISSING)               | `move a3,v0`         | THE PARK INSN |
| 22 | `move v1,v0`            | `move a0,v0`         | Y-limit save |
| 44 | `lui v0,0xe300`        | `lui a0,0xe300`      | const dest |

Same 10-12 diff cascade for the sibling `func_8007C86C` (constant 0xE4).

## Sibling

`func_8007C86C` is the constant-differs sibling (0xE4 vs 0xE3). Same 21-rule
pattern, same floor 12 under this lever stack. See
`memory/wip/func_8007C86C/`.

## Session 2026-06-02 (workflow round 1)

9-variant exploration of non-cheat structural levers from score-12 base, plus a
key UB-form proof. Notable finding: the C86C-sibling UB-form (`var_a1 = arg1`
placed ONLY in the negative-arg0 else branch, leaving var_a1 read uninitialized
on the positive-arg0 path) DOES emit target's `move a3, a0` X-preserve
allocation — proving the $a3 register match IS structurally C-reachable.
BUT that form (a) scores 18 (worse than 12 even with build_insns=51 matching
target), and (b) is FORBIDDEN per [[param-local-alias-prologue-pair-flip]] —
same coercion family as the previously-rejected raw_arg0 chain. So the search
space DOES contain a path to the right register allocation, but every known
path requires UB or a literal-rename coercion. The score-12 candidate remains
the legitimate pure-C floor. New top next_hypothesis: directed permuter from
score-12 base with cheat-reviewer gating on any saved candidate.

## Session 2026-06-02 (workflow round 2)

**Floor unchanged at 12.** Executed the round-1 top hypothesis: directed
permuter from score-12 CLEAN single-function offset-0 target. ~8254 random-
mutation iters in 4 wallclock minutes from base permuter score 185. Produced
11 saved candidates at permuter-weighted scores 90/95(x2)/150(x2)/175(x3)/180(x4)
— ALL failed self-vetting against the cheat catalog:

- **score-90**: `var_a1=0` in neg-arg0 else = dead-conditional-store + semantic change
- **score-95** (x2): synthetic `new_var`/reuse-`var_v0` for shared-zero routing; one
  variant MEASURED sandbox=13 (WORSE than 12) despite build_insns matching target
- **score-150** (x2): `var_a1=0` inside positive-arg0 X-clamp branches = semantic change
- **score-175** (x3): uninitialized var_v1 read (UB) + var_v0_2=arg0 clobbers (semantic change)
- **score-180** (x4): named-intermediate-for-boolean + var_v0_2=arg0 clobbers

KEY EVIDENCE: corroborates round-1 UB-form proof. The search space DOES contain
masked-Levenshtein-closer shapes, but every closer shape requires UB OR semantic
change OR coercion-only synthetic local. Permuter weight ≠ sandbox masked
Levenshtein ([[difficult-is-not-impossible]] § Metric gotchas) — empirically
confirmed.

**Engine bug surfaced** (filed as separate work item): `engine.inlineasm.write_stripped`
strips `#define PAD_NOPS_*` macros in display.c, leaving raw `PAD_NOPS_1;` references
that maspsx can't parse. Worked around with `--keep-cheat-asm` (no effect — func has
no cheat-asm pins). Blocks `sandbox --disable all` on display.c entirely.

New top next_hypothesis: PERM_GENERAL directed (not random) permuter ~30k iters,
OR ALLOCDBG/PRIODBG instrumented cc1 dump on score-12 form. ~60k+ cumulative
permuter iters + 25+ structural variants + UB-form proof = approaching the
escalation boundary for parking with documented evidence ledger.

## Session 2026-06-04 (workflow round 4)

**Floor unchanged at 12. HEAD drift observed.** Worker entered worktree at
main f9640bad. Candidate body applied cleanly to src/display.c → confirmed
floor 12 (build_insns 50, target 51, masked Lev. 12). Three small structural
variants tested at this base, ALL measured negative:

- **T1 (compare-operand-order flip + bltz-first):** rewrite X-clamp as
  `if (arg0 < 0) zero; else if (arg0 > D_8009BE78 - 1) limit-1; else arg0;`
  Score 16, regression. The compare-operand-order-register rule does NOT
  apply here (comparison is against a constant, not a global+register pair).
- **T2 (identity precompute):** `var_v0 = var_v0_2;` before dispatch + mask
  in branches. Score 22, build_insns 52 (+1 insn). Sibling of the round-2
  rejected synthetic-local family.
- **T3 (duplicated var_a1 = arg1):** put the preload into BOTH X-clamp
  branches instead of unconditionally. Score 12, GCC merges them via
  cross-jump — equivalent to the candidate.

**HEAD DRIFT (notable):** committed src/display.c func_8007C7A0 body NOW
scores 20 (not the head_floor=16 the meta.json was authored against). The
current HEAD uses synthetic `int new_var = arg0 >= 0;` + reused `int new_var2`
locals — these match the round-2 permuter-rejected forms (rejected_forms[12-14]).
The candidate's score-12 is now -8 below HEAD's 20, not -4. (Worker note: HEAD
regressions outside this WIP's scope; flagged for orchestrator awareness.)

**Source reverted; oracle confirmed green** (build SHA1 == 62efab4f).
Structural ceiling per remaining_gap stands. The genuinely-un-tried avenues
remaining are infrastructure-heavy:
1. PERM_GENERAL DIRECTED permuter (~30k iters, ~15 min budget) targeting the
   X-preserve register slot.
2. BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG instrumented cc1 dump on the score-12
   form to identify pseudo priorities of arg0/var_v0_2/var_a1/var_v1 (per
   the cpu_side_move_dir_4 / saEft00Add precedents — both used instrumented
   dumps to derive their levers).

## Session 2026-06-05 (workflow round 11 — instrumented-cc1 diagnostic)

**Floor unchanged at 12.** Instrumented-cc1 ALLOCDBG/SCHEDDBG/PRIODBG diagnostic
on the score-12 candidate body (per the campaign brief at
register-alloc-pure-c.md §6). Ran `tmp/gccdbg/cc1` (May 30 build, all three
debug hooks live; canonical cc1 SHA1 unchanged) on `permuter/c7a0_wf341/base.c`
with all three env vars set.

**Pseudo identification (the result the round-5 / round-7 escalations asked for):**

- Pseudo 72 = X-preserve var_v0_2 backup (line 23 `move $6,$4`). Gets $a2
  (target wants $a3). Allocno priority = 1818 (LOWEST in the 11-pseudo
  table, ord=10/10). nrefs=2, livelen=11.
- Pseudo 79 = Y-mask + Y-shift chain ($v1 across lines 81/85/86/95). Allocno
  priority = 10769 (HIGHEST). Already saturating $v1 — pushing it higher
  doesn't displace any other pseudo's allocation on the path between $a2
  and $a3 for pseudo 72.
- Pseudo 91 = sign-extended arg1 ($a2 at line 56). Allocno priority = 4285.
  Shares $a2 with pseudo 72 via non-overlapping live ranges.

**To flip pseudo 72 from $a2 to $a3:** some pseudo with priority > 1818 must
own $a2 across pseudo 72's livelen. EVERY way to manufacture that with pure-C
source falls into a forbidden cheat family (chain-extender, dead store,
synthetic local without semantic purpose, semantic change).

**Five candidate Lever A/B/C variants self-vetted against cheat-reviewer 6-test
checklist BEFORE coding** (per brief — `chain-extender forms inspired by the
ALLOCDBG output are FORBIDDEN`):

- (A.1) Block-local var_a1 wide-arm-only precompute — SKIP, duplicate of
  rejected_forms[25] (v2 Lever 2, score 18).
- (A.2) Block-local var_v0_2 inside wide-arm — FAIL test #1 semantic
  (X-clamp result used in BOTH arms).
- (B.1) u8 var_v0_2 / var_a1 — FAIL test #1 semantic (D_8009BE78/D_8009BE7A
  halfwords exceed 255, truncation).
- (B.2) u32 var_v0 / var_v1 — SKIP, permuter type-mutation exhausted.
- (C) loop-local precompute — N/A (no loop).
- (D) goto-form X-clamp — SKIP, rejected_forms[7] score 16.

No src edits applied; oracle remains green. Diagnosis content at
`tmp/gccdbg_func_8007C7A0/diagnosis.md` (gitignored; content folded into
`meta.json` round-11 sessions[] note).

PARK_CANDIDATE escalation reinforced. The dump corroborates the rounds-1-9
empirical evidence at the mechanism level: pseudo 72's priority is
quantitatively too low to flip without forbidden chain-extension.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 12 — but NO MEASUREMENT TAKEN.** Worker entered an isolated
worktree without prior WIP state visibility (memory/wip/func_8007C7A0 wasn't
propagated). Sandbox `func_8007C7A0 --disable all` returned non-scorable: the
display.c-wide cheat-asm-strip pipeline truncation issue (same gotcha as round
2's PAD_NOPS observation, blocking the C97C and CBB0 workers too) removes
func_8007C7A0 from the build output. Without a measurable baseline no candidate
edit was attempted.

**Two findings worth recording:**

1. **Sandbox truncation workaround discovered (by parallel C86C worker):**
   `--keep-cheat-asm` flag preserves indices when the function under test has
   no cheat-asm in its own body. Should be tried for C7A0/C97C/CBB0 next round.

2. **Existing HEAD src/display.c body carries a UB-conditional-init:** `var_a1`
   is read at line 547 (`if (var_a1 >= 0)`) but only assigned in the else branch
   at line 545 (`if (new_var) { ... } else { var_v0_2 = 0; var_a1 = arg1; }`).
   When new_var is true, var_a1 is uninitialized at the read. This is exactly
   the cheat-form catalogued in rejected_forms[1] of this WIP — must be replaced
   with a shared-end-label structure before any honest pure-C lever evaluation.

**Joint with C86C round-3 result:** the round-3 12-variant sweep on C86C lim_x
shape proved no middle ground exists — every variant that introduces a
separate s32 pseudo for lim_x costs +6 cascade; every variant that folds back
to no-separate-pseudo matches score 12. The same constraint applies here
verbatim via the sibling-constant relationship (0xE3 vs 0xE4 differs only).
The escalation boundary is approached: ~60k+ cumulative permuter iters + 25+
structural variants + UB-form proof + C86C 12-variant sweep result confirm
the structural ceiling.
