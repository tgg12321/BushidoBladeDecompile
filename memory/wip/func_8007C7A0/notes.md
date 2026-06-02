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
