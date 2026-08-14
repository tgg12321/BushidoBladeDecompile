# Hypothesis ledger — CdControl

## Session 1 (recon, 2026-08-14)

### CONFIRMED
- **H1 — CdControl is the same function family as the already-matched siblings
  CdControlF and CdControlB, and their solved spelling ports directly.**
  Mechanism: identical body, identical target register ordering (count, a1,
  [a2], idx, a0, saved, elem, result). Probe: apply CdControlF's pin-free,
  `base`/`elem` two-step, do-while(0)-wrapped shape plus its init order.
  Result: floor 25 → 4 in four measured steps. **CONFIRMED.**
- **H2 — The residual is 100% register-assignment priority, driven by
  `flow.c` loop-depth ref weighting feeding `global.c allocno_compare`.**
  Probe: adding the `do { ... } while (0);` loop-note wrap to the goto-loop
  body, with nothing else changed, moved 20 → 6 and put ALL EIGHT callee-saved
  values on their target registers at once. Instruction count and control flow
  never changed (78 = 78) at any point. **CONFIRMED.**
- **H3 — Init-statement order is a live but now-exhausted lever at this body
  shape.** Probe: two complete 240-permutation sweeps (before and after the
  wrap). Range 20…31 without the wrap; six distinct orders tie at 4 with it,
  none below. **CONFIRMED (and exhausted for this body shape).**

### KILLED
- **H4 — Writing the retry loop as an honest real `do { ... } while (count !=
  -1);` loop gets the loop notes for free and removes the need for any FAKE
  construct.** Mechanism: a loop.c-visible loop emits `NOTE_INSN_LOOP_BEG`
  natively. Probe: full restructure, sandboxed. Result: **13**, at 82
  instructions vs target's 78 — loop.c's LICM hoists the invariant `-1` into a
  ninth callee-save (s8) with its own save/restore pair. Target materialises
  `-1` inside the loop each iteration (`asm/funcs/CdControl.s:65`), so the
  original was NOT a loop.c-visible loop. **KILLED.**
- **H5 — Respelling the exit test as `} while (count >= 0);` denies loop.c an
  invariant to hoist and rescues H4.** Probe: sandboxed. Result: **13** at 79
  instructions. The s8 hoist is gone as predicted, but `bgez` is one
  instruction where target spends two (`addiu $v0,$zero,-1` + `bne`). The exit
  test must stay the two-instruction `!= -1` compare. **KILLED.**

### Frontier for session 2+

1. **Get a loop.c-visible loop WITHOUT the LICM cost.** H4/H5's real-loop form
   is the ONLY form measured this session that seats `a0` in s4 and `saved` in
   s5 — exactly the pairwise inversion the score-4 candidate still carries. The
   question is not whether loop visibility fixes it (it does), but how to pay
   for it at 78 instructions with a two-instruction `!= -1` exit test. Probes
   worth running: keep the `!= -1` test but make `-1` non-invariant or
   non-hoistable (e.g. compare against a value derived inside the loop);
   `while (1) { ...; if (count == -1) break; }` and `for (;;)` spellings (NB:
   these are NOT sanctioned by the do-while(0) carve-out and must be judged on
   their own merits — but as MEASUREMENTS they are free and they map the
   space); a real loop that encloses only part of the body.
2. **Attack the a0-vs-`saved` priority inversion directly at the goto-form's
   score 4.** `a0` has 2 refs (one inside the retry body), `saved` has 3 (one
   inside). Under the wrap that ties them closely enough that `saved` wins.
   Probes: shorten `saved`'s live range or lengthen `a0`'s (`live_length` is
   the denominator in `allocno_compare`) by moving where `saved` is read
   relative to the `a0` uses; spell the second CD_cw argument as `(u8)a0`
   vs `a0 & 0xFF` vs `idx` and measure each (changing `a0`'s ref count is the
   most direct handle on its priority); read `g_cd_callback_a` into `saved`
   later, immediately before the loop.
3. **Diff the score-4 build against the matched CdControlB at the RTL level.**
   CdControlB is 3-param, same body, same target seating, and matched WITHOUT
   any do-while(0) wrap — it is a working counter-example that the seating is
   reachable from a plain goto-loop. The structural differences are: no
   `result` local (it uses `status`, assigned inside BOTH exit arms), and a
   trailing CD_sync block after the shared `done:` label. Probe: `cc1 -da` greg
   dumps of CdControl-at-4 and CdControlB side by side
   (`;; Register dispositions:` + conflict lists) to see exactly why `saved`
   out-prioritises the `a0` copy in one and not the other.

## [s1] CdControl is the same function family as the already-matched siblings CdControlF and CdControlB, and their solved spelling ports directly.
- mechanism: Identical body and identical target register ordering (count, a1, [a2], idx, a0, saved, elem, result); CdControlB is 3-param with byte-identical seating for the six shared values.
- probe: Delete the register s7 pin and the new_var/new_var2 DImode chain; adopt CdControlF/CdControlB's two-step 'base = g_cd_sector_buf; elem = base + idx;'; then CdControlF's do-while(0) wrap and init order.
- result: sandbox 25 -> 21 (port) -> 20 (init sweep) -> 6 (wrap) -> 4 (re-sweep). Instruction count 78 == 78 throughout.
- verdict: CONFIRMED

## [s1] The entire residual is register-assignment priority, driven by flow.c loop-depth ref weighting feeding global.c allocno_compare; a backward goto emits no loop notes so every retry-body reference is weighted at depth 0.
- mechanism: flow.c life analysis does REG_N_REFS(regno) += loop_depth per reference; global.c allocno_compare orders allocnos by ~floor_log2(n_refs)*n_refs*size/live_length and global_alloc hands out callee-saves in that order. A do {...} while (0); wrap emits NOTE_INSN_LOOP_BEG, re-weighting exactly the references inside the retry body (a2 +2, a1 +2, count +2, idx +1, elem +1, saved +1, a0 +1, result +0).
- probe: Add the wrap around the goto-loop body with nothing else changed; sandbox and disassemble tmp/sandbox/CdControl/system.o.
- result: 20 -> 6, and all eight callee-saved values landed on their target registers simultaneously (s0=count, s1=a1, s2=a2, s3=idx, s4=a0, s5=saved, s6=elem, s7=result).
- verdict: CONFIRMED

## [s1] Init-statement order is a live lever but is exhausted at this body shape.
- mechanism: GCC 2.7.2 creates pseudos at first use, so the order of the six initialiser statements sets allocno creation order and live ranges.
- probe: Two complete 240-permutation sweeps of the six init statements (constraints: idx before elem, base before elem) — one without the wrap, one with it. tmp/grind/CdControl/s1/sweep.py.
- result: Without the wrap: range 20..31, best 20 (idx|base|result|saved|count|elem). With the wrap: six orders tie at 4, none below; every one of them ends with elem then result. Best is byte-for-byte CdControlF's own init order.
- verdict: CONFIRMED

## [s1] Writing the retry loop as an honest real 'do { ... } while (count != -1);' loop gets the loop notes natively and removes the need for any FAKE construct.
- mechanism: A loop.c-visible loop emits NOTE_INSN_LOOP_BEG without a wrapper, so the ref re-weighting comes from ordinary program structure.
- probe: Full restructure of the goto-loop into a real do-while; sandbox + disassemble.
- result: score 13 at 82 instructions vs target's 78 — loop.c's LICM hoists the invariant -1 into a NINTH callee-save (s8) with its own save/restore pair. Target materialises -1 inside the loop each iteration (asm/funcs/CdControl.s:65), so the original was not a loop.c-visible loop.
- verdict: KILLED

## [s1] Respelling the exit test as '} while (count >= 0);' denies loop.c an invariant to hoist and rescues the honest real-loop form.
- mechanism: bgez needs no materialised constant, so there is nothing for LICM to lift out of the loop.
- probe: Real-loop form with the bgez exit test; sandbox + disassemble.
- result: score 13 at 79 instructions. The s8 hoist is gone as predicted, but target spends TWO instructions on the exit test (addiu $v0,$zero,-1 + bne $s0,$v0) where bgez spends one. The exit test must stay the two-instruction != -1 compare.
- verdict: KILLED
