---
name: func-8007c7a0-named-intermediate-lever
description: func_8007C7A0 — sandbox 18 -> 16 via C97C-style named-intermediate `r_e3` for the OR const + drop new_var/new_var2 coercion vars (novel beyond prior session's v8 arg1-preload floor of 18); still NOT COMPLETED-C, missing park-merge insn structural ceiling persists
metadata:
  type: project
---

# func_8007C7A0 - named-intermediate lever lowers honest floor from 18 to 16

## Status

**INCOMPLETE - re-parked 2026-06-01 (second retry in same day).**

Prior sessions established:
- Session A (commit `bc65a49`, 2026-05-22): 19-22 floor across 23
  attempts + 52k+ permuter iters.
- Session B (memory `func-8007c7a0-arg1-preload-lever.md`,
  2026-06-01): 18 floor via `var_a1 = arg1` unconditional preload (v8).

This session (2026-06-01, second retry): 16 floor via the C97C-
breakthrough named-intermediate technique combined with cleanup of
the coercion variables that session B retained.

The 21 regfix `subst` rules in `regfix.txt:3037-3058` remain the
prescribed register-rename handling. Function stays matched-with-rules.

## The new combined lever set

Building on session B's v8 (`var_a1 = arg1` preload), this session
applied THREE additional simultaneous changes that drove score 18 -> 16:

1. **DROP `int new_var; int new_var2;` coercion variables.** These
   were retained in session B's v8. They're not classic dead-vars
   (each is used as RHS of an immediately-following assignment) but
   they look like coercion artifacts and inflate the frame from 16
   to 24 bytes. With them removed, GCC computes `arg0 >= 0` directly
   in the comparison and the frame drops to target's 16 bytes.

2. **REMOVE the `var_a1 = var_a1 & 0xFFF;` pre-dispatch line +
   restructure the third-if as inline `(var_a1 & MASK) << SHIFT`
   expressions.** The committed form had `var_a1 = var_a1 & 0xFFF;`
   before the dispatch then `var_v1 = (var_a1 << 1) << 11;` in the
   "else" branch. The cleaner form is `var_v1 = (var_a1 & 0xFFF) << 0xC;`
   directly. This shape matches m2c's reconstruction.

3. **ADD named intermediate `s32 r_e3 = var_v0 | 0xE3000000;`** at
   the return site (the C97C technique). Forces GCC to materialize
   the `var_v0 | const` accumulator FIRST in its own pseudo, then
   OR in `var_v1`. Without this, GCC reassociates `var_v1 | (var_v0 |
   const)` into `(var_v1 | const) | var_v0`, emitting the OR sequence
   in the wrong order vs target.

```c
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 var_v0_2;
    s16 var_a1;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;   /* session B's preload lever */
    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
    }
    if (var_a1 >= 0) {
        if ((D_8009BE7A - 1) < var_a1) {
            var_a1 = D_8009BE7A - 1;
        }
    } else {
        var_a1 = 0;
    }
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        var_v1 = (var_a1 & 0x3FF) << 0xA;
        var_v0 = var_v0_2 & 0x3FF;
    } else {
        var_v1 = (var_a1 & 0xFFF) << 0xC;
        var_v0 = var_v0_2 & 0xFFF;
    }
    {
        s32 r_e3 = var_v0 | 0xE3000000;   /* C97C-style named intermediate */
        return var_v1 | r_e3;
    }
}
```

`sandbox --disable all` baseline 20 -> 16. `build_insns` 51 -> 50
(still one short of target's 51 — the missing `move a3,v0` park insn
documented in session B's memory note).

## Progress journey (this session)

| Variant from baseline | Score | Build insns | Notes |
|---|---|---|---|
| baseline (HEAD committed form) | 20 | 51 | the m2c-style + new_var coercion |
| pure m2c-shape | 25 | 52 | extra sign-extend on `var_v0_2 = limit - 1` |
| s32 var_v0_2/var_a1 types | 27 | 45 | loses sign-extend ops but breaks ABI |
| cleanup (drop new_var/_2 only) | 17 | 50 | brought down by 3 |
| + flip operand order (arg0 > limit-1) | 17 | 50 | RTL canonicalizes; no effect |
| + r_e3 named intermediate | **16** | **50** | this session's best |
| + r_e3 + scoped tmp for clamp | 16 | 50 | coalesced |
| + r_e3 + decl order swap | 16 | 50 | no effect |
| + r_e3 + (s32) cast on var_v0_2 | 16 | 50 | folded |
| + r_e3 + if/else-if (early 0 path) | 20 | 50 | regression |
| + r_e3 + split sll statement | 18 | 50 | regression |
| + r_e3 + goto end + ret_val | 16 | 50 | inlined |
| + r_e3 + reversed arg aliases | 16 | 50 | coalesced |

## The remaining structural ceiling (UNCHANGED from session B)

The missing `move a3,v0` park insn at .L8007C7E0 persists. GCC keeps
`var_v0_2` in `$a0` throughout the function, while target moves it to
`$a3` at the clamp merge. The 16 residual diffs cascade from this
register-rotation:

| idx | ours (16-score) | target | gap |
|---|---|---|---|
| 0 | move a2,a0 | move a3,a0 | raw preserve reg ($a2 vs $a3) |
| 8 | move v1,v0 | move a2,v0 | maxX preserve reg |
| 12 | addiu a0,v1,-1 | addiu v0,a2,-1 | result reg |
| 14 | move a0,a2 | move v0,a3 | else result |
| 15 | move a0,zero | move v0,zero | neg path reg |
| 16 | (MISSING) | move a3,v0 | THE PARK MERGE |
| ... | ... | ... | cascade through 12 more insns |

## Why this is NOT committable (per persistence contract)

Same as session B: the source change does NOT retire any regfix rules.
The function still matches via its 21 register-rename substs. Committing
would require rewriting all 21 rules to handle the new cc1 emission
shape; net = lateral movement, no rule reduction, no completion progress.

The `park_src_guard` hook correctly blocks `park:` commits that modify
build files. Source reverted; commit only the memory note.

## Concrete next-step hypotheses for the next session

The structural ceiling is the missing `move a3,v0` park insn (= forcing
GCC's RA to allocate `var_v0_2` to `$a3` instead of `$a0`). Levers
tested this session that did NOT close the gap:

- Named intermediates (worked partially: 18 -> 16)
- Goto-end + ret_val (no effect)
- Decl order swap (no effect)
- Operand order flip (canonicalized away)
- Block-local tmp + outer storage (coalesced)
- (s32) casts (folded)
- Argument aliases (coalesced)
- If-else-if vs nested if (regression)

UNTRIED levers (genuinely new for resume):

1. **ALLOCDBG-instrumented session** to confirm WHY var_v0_2 lands in
   $a0 vs $a3. The instrumented cc1 (`tmp/gccdbg/cc1`) from prior
   sessions can dump allocno priorities for var_v0_2's pseudo vs raw
   arg0's pseudo. A specific livelen/refs balance would point at the
   structural change needed.

2. **Examine cc1's `.greg` dump** for the 16-score form (lever set
   applied) — find var_v0_2's hard reg assignment + conflict list.
   If $a3 is conflict-free at var_v0_2's live range, the choice is a
   tiebreaker that some specific lever may flip.

3. **Different OR association**: try `(var_v0 | 0xE3000000) | var_v1`
   as direct source (no named intermediate). The named intermediate
   approach worked but maybe a different chain shape can also retire
   the missing park insn.

4. **`tools/decomp-permuter` from the 16-score base** (clean single-
   function target as in [[difficult-is-not-impossible]] step 3).
   Permuter from a closer base might find a structural variant the
   manual search missed. Vet output against cheat catalog before
   proposing per [[no-new-park-categories]].

5. **Cross-reference matched `func_8007C748`** (sibling, simpler 3-arg
   variant). Read its `.greg` dump for how var_v0_2-analog got
   allocated to non-$a0. If that function's structure has a feature
   C7A0 lacks (e.g. a longer live-range competitor for $a0), that
   feature may be the missing C-level lever.

## Files / how to reproduce

- Baseline (committed, score 20): `src/display.c:523-562` (HEAD m2c body)
- This session's 16-score lever set:
  - Drop `int new_var; int new_var2;` decls
  - `var_a1 = arg1;` at top (preload)
  - Drop `var_a1 = var_a1 & 0xFFF;` line
  - Inline third-if as `(var_a1 & MASK) << SHIFT`
  - Add `s32 r_e3 = var_v0 | 0xE3000000;` block before return
- Verify: `& tools/eng.ps1 sandbox func_8007C7A0 --disable all`
  -> `"score": 16`

## Sibling impact

`func_8007C86C` (regfix.txt:3060-3081) has the IDENTICAL 21-rule cheat
pattern (0xE4 vs 0xE3 differs). Closing C7A0 closes C86C the same way.
Both stay parked until the structural ceiling lever is found.

## Related

- [[func-8007c7a0-arg1-preload-lever]] — session B's prior memory
  note; this session builds on its v8 lever.
- [[goto-end-prologue-delay-slot]] — C97C's breakthrough; NOT
  directly applicable here (no entry early-exit pattern), but the
  named-intermediate idea from C97C IS what brought score 18 -> 16.
- [[register-alloc-pure-c]] — the broader playbook; Levers A (block-
  local var split) + Lever B (narrow type) tested negative.
- [[param-local-alias-prologue-pair-flip]] — tested via reversed
  arg aliases, no effect.
- [[difficult-is-not-impossible]] — cardinal rule. 20 -> 16 is real
  progress; the matching C structure exists, just not in the levers
  tested so far.
- [[no-new-park-categories]] — register-rotation infrastructure is
  NOT an acceptable carve-out; function stays parked as unfinished
  work.
