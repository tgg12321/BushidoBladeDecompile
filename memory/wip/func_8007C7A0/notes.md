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
