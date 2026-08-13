# Hypothesis ledger — func_8006288C

## Session 1 (recon, 2026-08-13) — floor 23 -> 2

### CONFIRMED
**H1 — the whole distance-23 gap is the global.c allocno-priority sort, and the
loop's C spelling is the lever that reorders it.**
Mechanism: GCC 2.7.2 `global.c:allocno_compare` sorts allocnos by
`floor_log2(nrefs)*nrefs/live_length`; MIPS `reg_alloc_order` then hands out
$3,$4,$5,$6,$7,$8,$9,$10,$11 down that order, so the sort IS the register
assignment. The loop spelling moves the latch block and the loop notes, which
changes every pseudo's `reg_live_length`, which permutes the sort.
Probe: instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_ALLOC_DEBUG=1`) priority
table + a 20-variant standalone sweep (`tmp/grind/func_8006288C/s1/sweep.py`).
Result: `do { mask = 1 << i; if (!(D_800A3460 & mask)) { ...; goto out; }
<latch> } while (i < 6);` gives **9/9 target registers**; sandbox distance
**23 -> 2**. `while (1) { ... break; ... }` is codegen-identical (9/9);
`for (i = 0; i < 6; i++)` is 7/9 (swaps `i`/`off_s32` between $3 and $4);
the old label + `goto loop_top` + else-latch form is 3/9.

### KILLED
**H2 — the destination writes are array indexing (`(&SYM)[i*3+k]`) and GCC's
strength reduction manufactures the +12/+8 induction variables.**
Probe: `tmp/grind/func_8006288C/s1/probe_b.c` compiled with the project cc1
flags (strength reduction is ON — `NO_SR_FILES` is empty in the Makefile).
Result: **92 instructions**, a 16-byte stack frame, and per-store index
recomputation (`sll`/`addu` chains, repeated `la` of each base). The target's
`lui $at,%hi(SYM); addu $at,$at,<off>; sw $v0,%lo(SYM)($at)` shape only comes
from an explicit byte-offset accumulator. KILLED; banked as
`rejected/array-index-form-explodes-to-92-insns.c`.

**H3 — the if-body statement order is a free lever for live lengths.**
Probe: sweep set 1 (`variants.py`), 6 permutations of the word-store group /
halfword-store group / `D_800A3460 |= mask` / `*flag_p = 0`.
Result: every permutation changes the cc1 instruction COUNT (43, 43, 43, 45)
away from the target's 42 — the body order is byte-load-bearing, not a free
allocation lever. KILLED; banked as
`rejected/ifbody-statement-reorder-breaks-42-insn-shape.c`.

**H4 — value-naming variants inside the loop move the allocation.**
Probe: sweep set 1 — explicit named flag-word local (`w = D_800A3460; ...
D_800A3460 = w | mask;`), no-mask-variable (inline `1 << i` at both uses),
and a staged temp for the third halfword.
Result: all three are **codegen-identical** to the base form (same 42
instructions, same registers). CSE canonicalises them to the same RTL.
KILLED as an allocation lever.

**H5 — the init-block statement order moves the allocation.**
Probe: sweep set 2 — 6 permutations (src pointers first / last, flag_p last,
off_s16 late, the `D_800F1138 = 1` store first).
Result: no permutation that keeps 42 instructions changes any register; the
two that did change registers (`g_flagp_last`, `g_off16_late`) moved `i` and
`off_s32` the WRONG way (1/9). KILLED.

**H6 — a `new_var = 1;` constant holder placed early gives the target's
prologue-slot-1 `li $t3,1`.**
Probe: sweep set 3 — `new_var = 1;` at each of the 8 init-block positions.
Result: every position **before** the `D_800F1138 = 1;` store makes cse fold
the two constant-1 materialisations (`sw $t3,D_800F1138`), dropping the
function to **41** instructions — the target has 42 with two separate `li`s.
Only the after-the-store position keeps 42, and there the scheduler parks
`li $t3,1` at prologue slot 5, not slot 1. KILLED; banked as
`rejected/const1-before-store-cse-folds-to-41-insns.c`.
(Note the current best form drops the holder entirely — `mask = 1 << i;` — so
the `li` is created by loop.c invariant hoisting after cse2 and no fold
happens; same 42 instructions, same slot-5 placement.)

## Live frontier for session 2

1. **The residual-2 is the prologue slot of `li $t3,1`.** Target emits it
   immediately after `move $a0,$zero`; we emit it after the four
   `la`/`move` init insns. Everything else — all 52 instructions, all 9
   register assignments — already matches. Mechanism to attack: the constant
   is created by loop.c's invariant hoist into the preheader (post-cse2, which
   is why it is not folded with the store's `1`), and cc1's first-pass
   scheduler (`sched.c`, `-fschedule-insns`) then places it at slot 5. Next
   probe: dump `probe.i.sched` / `probe.i.loop` for the current best form and
   read where insn for `(set reg 1)` is inserted and what its `INSN_PRIORITY`
   is relative to the `la`/`move` insns; then look for a C form whose init
   block gives the constant a longer dependence chain to the `sllv`.
2. **Cheap unexplored axis: the types of the offset/index locals.**
   `off_s16`/`off_s32`/`i` are all `s32`; `u32`/`s16` spellings change
   `allocno_size`/live ranges and cost nothing to sweep with the existing rig
   (add a set to `variants*.py`).
3. **Permuter.** At distance 2 with a 52/52 instruction-for-instruction shape
   this is an ideal short permuter run — build `target.o` from
   `asm/funcs/func_8006288C.s` + the permuter prelude so the function sits at
   offset 0 (see [[difficult-is-not-impossible]] §3), seed from the current
   best form, and run with `--stop-on-zero`.

## [s1] The entire distance-23 gap is register assignment, and the loop's C spelling is the lever: GCC 2.7.2 global.c allocno_compare sorts allocnos by floor_log2(nrefs)*nrefs/live_length and MIPS reg_alloc_order then hands out $3,$4,$5,$6,$7,$8,$9,$10,$11 down that order, so the sort IS the assignment; the loop spelling moves the latch block and loop notes and therefore every pseudo's reg_live_length.
- mechanism: global.c allocno_compare priority sort + MIPS reg_alloc_order; live_length is set by flow.c from the RTL block layout the loop spelling produces
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) priority table for the baseline, plus a 20-variant standalone cc1 sweep (tmp/grind/func_8006288C/s1/sweep.py) scoring each variant's variable->hard-register map against the target map extracted from asm/funcs/func_8006288C.s
- result: do { mask = 1 << i; if (!(D_800A3460 & mask)) { ...; goto out; } <latch> } while (i < 6); gives 9/9 target registers and sandbox --disable all distance 23 -> 2. while(1){...break;} is codegen-identical (9/9); for(i=0;i<6;i++) is 7/9; the previous label + goto loop_top + else-latch form was 3/9.
- verdict: CONFIRMED

## [s1] The destination writes are array indexing ((&SYM)[i*3+k]) and GCC's strength reduction manufactures the +12 / +8 induction variables that the target increments in the latch.
- mechanism: loop.c strength reduction creating givs from an i-scaled index
- probe: tmp/grind/func_8006288C/s1/probe_b.c compiled with the exact project cc1 flags (strength reduction is ON; NO_SR_FILES is empty in the Makefile)
- result: 92 instructions vs the target's 52, a 16-byte stack frame, and per-store index recomputation (sll/addu chains plus a repeated la of each base). The target's lui $at,%hi(SYM); addu $at,$at,<off>; sw $v0,%lo(SYM)($at) shape only comes from an explicit byte-offset accumulator, which is what the in-tree C already had.
- verdict: KILLED

## [s1] The statement order inside the taken-slot if-body is a free lever for shifting allocno live lengths.
- mechanism: moving a store changes each pseudo's last-use point and hence reg_live_length
- probe: Sweep set 1 (tmp/grind/func_8006288C/s1/variants.py): 6 permutations moving the word-store group, the halfword-store group, D_800A3460 |= mask, and *flag_p = 0
- result: Every permutation changes the cc1 instruction COUNT away from the target's 42 (43, 43, 43, 45). The body order is byte-load-bearing, not a free allocation lever.
- verdict: KILLED

## [s1] Renaming/staging values inside the loop (explicit flag-word local, no mask variable, staged temp for the third halfword) moves the allocation.
- mechanism: extra named intermediates changing allocno nrefs
- probe: Sweep set 1, variants v1_named_w / v5_no_mask / v6_stage_c
- result: All three are codegen-identical to the base form — same 42 instructions, same 9 registers. CSE canonicalises them to identical RTL, so nrefs never changes.
- verdict: KILLED

## [s1] The init-block statement order moves the allocation toward the target.
- mechanism: def points set the start of each live range
- probe: Sweep set 2: 6 init-order permutations (src pointers first / last, flag_p last, off_s16 late, D_800F1138 store first) with the loop held fixed
- result: No 42-instruction permutation changes any register; the two permutations that did change registers moved i and off_s32 the wrong way (1/9).
- verdict: KILLED

## [s1] A constant-holder local (new_var = 1;) placed early in the init block produces the target's prologue-slot-1 li $t3,1.
- mechanism: source position of the constant assignment setting its RTL emission slot
- probe: Sweep set 3: new_var = 1; at each of the 8 init-block positions, under the confirmed do/while shape
- result: Every position before the D_800F1138 = 1; store makes cse fold the two constant-1 materialisations (sw $t3,D_800F1138) and drops the function to 41 instructions; the target has 42 with two separate li insns. Only the after-the-store position keeps 42, and there the scheduler parks li $t3,1 at prologue slot 5, not slot 1. The best form therefore drops the holder entirely (mask = 1 << i), letting loop.c hoist the constant post-cse2 — same 42 instructions, same slot-5 placement.
- verdict: KILLED
