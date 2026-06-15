# func_800600C8 — WIP (text1b digit/texture setup; 6 rules + source cheats)

## TL;DR
Honest distance (rules disabled) = **13** (sandbox --disable all). HEAD
"matches" via SIX regfix `subst` rules PLUS two source-level cheat constructs.
121-insn function with multiple COUPLED register + scheduling diffs — this is
orchestrator/multi-session grade, not a single-lever close.

## The cheats HEAD carries
1. **6 regfix-subst rules** (regfix.txt) doing instruction swaps:
   - `addu $4,$sp,24` <-> `sw $2,48($sp)` at idx 25/26 and 78/79 (scheduling
     swap around the func_8007352C call arg setup, both call sites).
   - `sh $2,72($sp)` <-> `sh $2,74($sp)` at idx 48/49 (s.d0/s.d1 store order).
2. **Source cheats** (src/text1b.c):
   - `if ((!hi) && (!hi)) {}` — empty-if dead-read of UNINITIALISED `hi`
     (hi is only assigned later at `hi = (s16)arg0 / 10`). Reserves hi's
     register early to bias allocation. Empty-if dead-read family — FAIL.
   - `s.zero1C = 0 & 0xFFFFu;` — masked-zero coercion (just `= 0`).

## Honest diff (rules + nothing else disabled), 119 vs target 121
Multiple clusters, coupled:
- target keeps `move t1,s2` (a copy of arg1 for the first `s.in_tex = cur_tex`
  store, `sw t1,32(sp)`); ours folds cur_tex==arg1==s2 and stores s2 directly
  (-1 insn). Needs cur_tex to be a distinct live value without a coercion.
- `width` value: target in v0 (`li v0,147/163`, `sw v0,48(sp)`); ours in v1.
- `sh` store order (72 vs 74) for s.d0/s.d1 — the regfix sh-swap target.
- second call-site width `(((1-i)<<2)<<3)+0x54` -> register v0 vs v1 again.

## Why this is blocked (not a quick close)
~8 distinct coupled diff clusters spanning register allocation (v0/v1 width,
the t1 copy) AND scheduling (the addu/sw swaps) AND store ordering. No single
structural lever addresses them; the empty-if dead-read is load-bearing for the
current cheated allocation. Removing the source cheats changes codegen
non-trivially. This needs sustained multi-pass work (orchestrator modality),
likely a from-scratch re-derivation of the digit-rendering loop structure.

## Avenues for next session
- Re-derive the function structure cleanly (digit extraction: d0=arg0%10,
  hi=arg0/10, d1=hi%10; two-iteration render loop), WITHOUT the empty-if and
  masked-zero, then attack the residual width-register (v0/v1) and the t1 copy.
- decomp-permuter on the full 121-insn body once a clean cheat-free base exists.

## Floor
- HEAD: distance 13 carrying 6 regfix rules + 2 source-cheat constructs.
- No cheat-free candidate at <13 found this pass (not attempted to completion —
  flagged as multi-session).
