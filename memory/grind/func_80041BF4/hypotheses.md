# Hypothesis ledger — func_80041BF4

## Frontier after s1 (floor 22, rotation solved — see evidence.md [s1])
1. **Symbol-address form of the in-loop D_80094DF0 load** (residual item 1).
   Mechanism: under the new outer-loop notes, loop.c/cse treats the
   loop-invariant `symbol_ref D_80094DF0` differently and keeps the full
   address in a pseudo ($t0) instead of the %hi/%lo indexed form. Probe:
   dump.ps1 on the candidate form, read .loop (was the address a movable?)
   and .cse; compare against sibling func_80041AC8's RTL (same expression,
   emits target form outside a loop). Candidate levers: hoist the
   D_80094E08[sid] index load into a named local so the address arithmetic
   is no longer the loop-invariant candidate; or spell the read through the
   already-named sid expression per [[defeat-licm-hoist-var-reuse]]
   (reuse-a-scratch-variable recipe, but note we need the INVERSE — keep
   the recompute IN the loop).
2. **LoadImage arg-setup scheduling** (residual item 2) — re-measure only
   AFTER item 1: the a1 arg chain is the same symbol machinery, so fixing
   the address form likely reflows the sched1 order; do not tune this
   independently first.
3. **Caller-save renames (a1/v0, v1/t0)** — expected to collapse with 1/2;
   if any survives, read .lreg for the specific pseudo and check
   [[local-alloc-death-count-class-wall]] / rank-for-schedule ties before
   inventing levers.

Standing kills to respect (do NOT reopen): every outer-ref spelling
(fold/emit dichotomy), a2-param-reuse (rejected/), the do-while(0) yoff wrap
under the real outer loop (it inflates yoff to 5853 and flips the rotation
backwards), permuter random campaigns on the old basin.

## [s1] The 3-cycle s2/s3/s4 rotation is reachable in pure C via loop-depth-weighted allocno priorities, without any new outer reference
- mechanism: flow.c counts REG_N_REFS weighted by loop_depth (NOTE_INSN_LOOP from real loop constructs only); global.c pri = floor_log2(refs)*refs/live_length*10000. A real `do {...} while (outer < 2);` lifts outer's in-loop refs to depth 2 (pri 2000 -> ~5192 with the floor_log2 jump at 9 weighted refs), above b (~3870), giving allocation order outer > b > yoff > xoff = target's
- probe: rewrote the goto outer loop as do-while AND removed the leftover `do { yoff = 0; } while (0);` wrap (which put yoff's else-def at depth 3, pri ~5853, and had flipped the prior do-while attempt's rotation the wrong way); sandbox --disable all
- result: score 22 (from 29); objdump shows a2 home->s2, outer->s2, b->s3, yoff->s4, xoff->s5, g/r->s6/s7, fp_ptr->fp — target's full callee-save allocation; 135/135 insns
- verdict: CONFIRMED

## [s1] Reusing the a2 param variable as the outer counter reproduces target's s2 sharing
- mechanism: merged pseudo would inherit the param's low pseudo number and win priority ties
- probe: a2 = 0; ... a2++; while (a2 < 2) spelling; sandbox
- result: score 34: merged pseudo has refs 7 over live length ~86 (two disjoint phases) -> pri ~1628, allocates LAST, lands in s4 and perturbs prologue save order; banked in rejected/a2-param-reuse-as-outer.c
- verdict: KILLED
