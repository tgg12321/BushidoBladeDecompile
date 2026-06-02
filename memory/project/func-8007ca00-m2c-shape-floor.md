---
name: func-8007ca00-m2c-shape-floor
description: func_8007CA00 (display.c switch-dispatch with shared block_5) — m2c-shape rewrite + var_a0/var_v1 split + 2-statement subtract drops honest sandbox floor 13 → 10 (prior parked at 11). NOT committable (rules calibrated for old shape).
metadata:
  type: project
---

# func_8007CA00 — m2c-shape body drops sandbox 13 → 10 (prior session: 11)

## Status

**INCOMPLETE — re-parked 2026-06-01.** Prior park (commit `2b3ccfe`)
documented floor of sandbox 11 via the "v0_tmp temp split" lever. This
session pushed it to **sandbox 10** via three combined levers below,
all pure-C. Source change reverted (the 13 existing regfix rules are
calibrated against the OLD body and cannot retire on the new shape).

Same fundamental wall persists: 10 residual diffs are reorg.c
delay-slot-fill decisions and case 2 BE77==0 chain register allocation
($v0 vs $v1) that no explored C-source lever closes within the
remaining budget.

## The score-10 form (NOT committable as-is)

```c
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 var_v1;
    s32 var_a0;

    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            var_v1 = arg0[2];
            var_a0 = arg0[0];
block_5:
            {
                s32 tmp = 0x400 - var_v1;
                tmp -= var_a0;
                return tmp;
            }
        }
        return arg0[0];
    case 2:
        if (D_8009BE77 != 0) {
            var_v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            var_a0 = arg0[0];
            goto block_5;
        }
        return ((s32)arg0[0]) / 2;
    default:
        return arg0[0];
    }
}
```

## Levers found this session (13 → 10)

1. **m2c-shape adoption** (13 → 14 initially with temp_v0, then 12-10 after
   refinement). m2c reconstructs the target as a 3-case switch with case 1
   containing the shared `block_5` label, case 2 doing `goto block_5`, and
   case 2 BE77==0 / default returning independently. Adopting this shape
   restores the proper switch dispatch (default-at-end so it merges with
   the shared tail).

2. **`s32 var_a0` split** (12 → 10). Case 1 BE77!=0 loads `arg0[0]` into
   `var_a0` BEFORE the `goto block_5`. Case 2 BE77!=0 does the same. Then
   `block_5: tmp = 0x400 - var_v1; tmp -= var_a0; return tmp;` uses the
   pre-loaded `var_a0` instead of dereferencing `arg0[0]` at block_5 entry.
   This matches target's structure where both case 1 and case 2 paths
   PRE-LOAD `arg0[0]` into $a0 before jumping into the shared tail
   (`subu v0,v0,a0` at .L8007CA40 trusts $a0 is pre-loaded).

3. **2-statement subtract split** (10 vs 11 with single expression).
   `{ s32 tmp = 0x400 - var_v1; tmp -= var_a0; return tmp; }` (block-local
   `tmp` initialized as `0x400 - var_v1`, then `tmp -= var_a0`) gives the
   right operand order for the subus: `subu v0,v0,v1` first, `subu v0,v0,a0`
   in j delay. Writing it as `return (0x400 - var_v1) - var_a0` produces
   the same masked count but reverses the subu order. The 2-statement form
   also keeps `tmp` allocated to $v0 (single-expression form caused
   `partial` to go to $a1 with the prior hoisted-`s32 partial;` variant).

## What remains (10 residual diffs)

Diff structure (target=44 insns, sandbox=44 insns, both match count):

1. **Case 1 BE77!=0 beqz delay slot fill (~5 cascade diffs)**: Target
   fills the `beqz v0,.L_BE77eq0` delay slot with `li v0,1024` (copied
   from block_5 start via reorg.c's `fill_slots_from_thread`). Sandbox
   leaves it as `nop`. The `li v0,1024` exists in sandbox at idx 16
   (start of block_5 body) but reorg doesn't copy it into the delay
   slot.

2. **Case 2 BE77!=0 j delay slot fill (~2 cascade diffs)**: Target's
   `j .L8007CA40` delay slot is `li v0,1024` (also copied from
   block_5 start). Sandbox's j delay is `sra v1,v1,1` (pulled from
   case 2 body — the final `/2` insn). Net: sandbox is structurally
   1 insn shorter in case 2 (because sra fills the slot) and 1 insn
   longer in case 1 (because nop replaces the would-be li).

3. **Case 2 BE77==0 register chain ($v1 vs $v0, ~3 diffs)**: Target
   uses $v0 for the `arg0[0]/2` branchless-divide chain (`lhu v0,0(a0);
   sll v0; sra v1,v0; srl v0; addu v1,v1,v0; sra v0,v1,1`). Sandbox
   uses $v1 as the initial load destination (`lhu v1,0(a0)`) and the
   chain inverts v0/v1 roles. Same insn count, different registers.

## Why reorg.c declines the delay-slot copy

`mostly_true_jump` predicts the beqz/j based on heuristics. For the case
1 beqz, target's path predicts BE77!=0 as "more likely" → fills delay
from fall-through body's first insn (`li v0,1024` after block_5
reordering by sched.c). Sandbox's prediction or eligibility check
declines.

The "fill from target" trick where reorg copies the FIRST insn of the
branch target into the delay slot AND advances the branch past it
requires `eligible_for_delay` to return true on that first insn. For
case 1, that first insn (block_5 start) is `li v0,1024`. For sandbox to
also do this, GCC's sched.c must place `li v0,1024` AT THE TOP of
block_5 — which requires `tmp = 0x400` to be the first computation in
block_5's RTL. The 2-statement split (`s32 tmp = 0x400 - var_v1;`)
fuses these into one `subu` insn that depends on `var_v1`, putting the
constant load adjacent to the subtraction. So `li 1024; subu` is
emitted together as block_5 entry — and reorg should copy the `li
1024` if eligible. It's not, in this codegen.

Likely root cause: `li 1024` writes $v0 which is the BRANCH CONDITION
REGISTER of the beqz. While the branch was already evaluated, reorg may
treat $v0 as "in use" for the branch and decline to fill the delay
slot with an insn writing $v0. Target's $v0 might have been considered
dead by reorg differently due to upstream insn arrangement.

## What was tried this session and failed

| Lever | Score | Notes |
|---|---|---|
| baseline (committed shape) | 13 | reference |
| m2c-shape with `u16 temp_v0` for case 2 BE77==0 | 12 (45 insns) | u16 local adds stack frame (-8/+8 prologue/epilogue) |
| m2c-shape `arg0[0]/2` simple form | 14-12 depending on default placement | default-at-end fixed it |
| 2-statement subtract `s32 tmp = 0x400 - var_v1; tmp -= var_a0;` | 10 | **best — adopted** |
| Hoisted function-scope `s32 partial;` initialized first | 11-12 | partial → $a1 instead of $v0 |
| `partial = 0x400 - var_v1; return partial - var_a0;` single expr | 11 (43 insns) | partial → $a1, +1 less insn |
| `var_a0` declared/loaded BEFORE `var_v1` in case 1 | 13 | regresses |
| `block_5` OUTSIDE the switch, after default | 15 (47 insns) | explicit j to block_5 needs frame |
| Separate `tmp1`/`tmp2` per case (no goto, full inlining) | 21 (46 insns) | breaks cross-jump merge |
| Case 2 BE77==0 as m2c form `(s32)((s16)temp + ((u32)(temp<<16)>>31))>>1` with `u16 temp` | 12 (45 insns) | stack frame again |
| Case 2 BE77==0 as `(s32)((s16)*(u16*)arg0 + ((u32)(*(u16*)arg0<<16)>>31))>>1` (no temp) | 12 | regresses |
| Reorder case 2: `raw = arg0[2]; var_a0 = arg0[0]; var_v1 = raw/2;` | 10 | same — scheduler still puts loads first |
| `var_v1 = ... >> 1` (vs `/ 2`) | semantics differ for negative s16 | not equivalent |

## Resume avenues (untested this session)

1. **Directed permuter from sandbox-10 base.** `permuter/ca00/` exists
   with target.s; needs base.c + permuter run. Would explore mutations
   on the 2-statement-tmp form to flip the case 2 BE77==0 register or
   land the delay-slot fills.
2. **Instrumented `tmp/gccdbg/cc1`** (BB2_SCHED_DEBUG / BB2_FILL_DEBUG /
   BB2_PRIO_DEBUG already built May 30). Dump reorg.c's
   `eligible_for_delay` decision for the case 1 beqz to find what's
   blocking the `li v0,1024` copy. If $v0 dead-bit tracking is the
   culprit, find a C lever that makes it dead before the beqz reaches
   reorg.
3. **Rule update for the new shape.** The 13 existing rules are
   calibrated against the OLD body (where `s32 v1 = arg0[2]; return
   (0x400 - v1) - arg0[0]` is inlined and case 2 has `return ((s16)x)/2`
   without m2c form). The new shape with score 10 needs different rules
   to close the residual. Specifically: 1-2 rules to fill the case 1
   beqz delay slot with `li v0,1024`, 1 rule to swap case 2 BE77!=0's
   delay slot (sra → li 1024), 2-3 rules to swap case 2 BE77==0
   register chain v0↔v1.
4. **Source-file re-attribution** ([[no-new-park-categories]]
   evidence-based re-attribution). Not investigated; would require
   evidence that func_8007CA00 was in a different .c file in the
   original source — no signals from the function's behaviour.

## Related

- [[cross-jump-call-merge]] — sibling pattern (cross-jump merge of CALL
  suffixes); ca00 is the STORE-tail / SHARED-LABEL merge variant.
- [[cross-jump-store-tail-merge]] — store-merge case; ca00's structure
  is different (single shared computation block, not a per-arm store
  pattern).
- [[goto-end-prologue-delay-slot]] — sibling lever from func_8007C97C
  (just COMPLETED-C 2026-06-01); ca00 doesn't have the early-NULL
  pattern but the m2c-shape adoption is analogous.
- [[register-alloc-pure-c]] — the playbook for register allocation
  levers; ca00's case 2 BE77==0 chain $v0/$v1 swap may be reachable
  via a Lever A (block-local split) or Lever B (narrow type) probe
  not yet exhausted.
- [[difficult-is-not-impossible]] — the standing rule; matching C
  exists, more search required.
