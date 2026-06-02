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
