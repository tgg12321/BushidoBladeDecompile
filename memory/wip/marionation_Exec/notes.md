# marionation_Exec — HONEST SCORE 9; ALL EIGHT S-REGS + BLOCK REGS TARGET

## SESSION 6 — candidate.c assembled, score 9 (was 56→17→9)
**memory/wip/marionation_Exec/candidate.c** = the full function; apply
via tmp/mar_apply_candidate.py, test via tmp/mar_test_candidate.sh
(disp + honest sandbox + restore). ALL EIGHT s-regs target ✓ + all four
printf-block regs ✓. Composition: u12/u13 masks+block-local loop temps
+ w9 poll do-while + w24 printf (pp + 3-set t0 multiset BEFORE arg5 —
RAZOR: any reorder flips chain4/val5) + w25/26/27/28 arm shapes (arm-1
param-test, [i=7;dst=a1] interior; arm-2 [store;src;i;dst]) + w29
i1495-chain (UN-DOUBLES i1495 → 405 → s6; csmd4's committed idiom) +
w31 F19C0-store-chain via i1496 (+2 weighted refs → 7 → 933-TIE with
i1494, ascending-allocno breaks correctly → i1494 s2, i1496 s3, arg1
s4 at L=87/919). KEY EQUATIONS: saved 952 > i1494 933 = i1496 933 >
arg1 919 (L=87, NEEDS ≥86 — ONE clock of margin!) > tbl 657 (5 refs
doubled) > i1495 405 (undoubled) > arg0 256.
**Remaining 9 masked diffs + 1 insn (178 vs 179), 4 regions:**
(1) printf: lbu5 2-early + sll4 2-early [target: lbu4,lbu5,a1lw,sll5,
addu5,sll4,lw5]; (2) arm-1: sb-before-la + move-in-slot [target: sb,
la-pair, beqz-s4, move(slot), li v1]; ours [la, beqz, sb(slot), li,
move]; (3) check2: move/nop slot [+1 insn]; (4) coupled li/move order.
ALL statement-order fixes for 1-3 ROTATE the RA (arg1's L drops below
86 or the printf qty-birth tie flips): v2-v11 all measured 14-19.
tmp/mar_sweep_final.py = 40-combo permutation sweep (pp-pos × arm1i ×
arm1s × arm2 orders) — run in progress at session end; check results.
NEXT if sweep dry: (a) SLLDBG per-segment diff of v9-vs-v1 to find
WHERE arm-1's [dst;i] loses 2 clocks, then a neutral +2 lever;
(b) sched1-stall margin in do_timeout (clock-count vs bytes decouple);
(c) directed permuter with sandbox compile.sh on the candidate.
THEN: retire 42 + full SHA1 + declare new_var/new_var3 ✓ (already in
candidate) + dual review + queue done. csmd4's last 5 rules = same
machinery (transfer after).

## Composite recipe (other pieces, all probe-verified byte-neutral 142)
- u12/u13: masks (new_var/new_var3 opaque consts; fold via
  update_equiv_regs refs==2 UNWEIGHTED, local-alloc.c:1079) + csmd4
  arm shape + BLOCK-LOCAL `u8 bb` per copy loop → tail cascade
  (bb v0, src a0, i v1, dst a1, check a2).
- w9: poll as REAL `do { status=f(); if (status==0) break; {arms} }
  while (1);` → loop_depth weights (flow.c: depth PLUS ONE) → status
  8 refs, i1494 909→933 (under saved 952), i1495 → s6. **Outer cycle
  MUST stay goto-loop** (real loop → mask refs 4 ≠ 2 → fold dies).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5,
  i1495 s6, arg0 s7; tail check a2, src a0, i v1, b v0.
- Printf block (718FC-7194C): lbu a0(idx0), lbu v0(idx1), lui/lw a1,
  sll v0, addu v0+s5, sll a0, lw v1 0(v0), lbu v0 11D5, addu a0+s5,
  sll v0, sw v1 0x10(sp), lw a2 11DC(at), lw a3 0(a0), la a0 fmt, jal.
  tslTm2 slot = NOP (lbu a0 arg-conflicts, lbu v0 return-conflicts —
  both follow from the coloring).
- after_blocks: `beqz s7 → loop`, `move v0,zero` delay. The two
  null-path [j; move v0,a2] sites stay unmerged (ours too ✓).

## Known gotchas
- `new_var` UNDECLARED at :555 (pipe swallows cc1 error-recovery exit;
  bytes fine). Declare new_var/new_var3 only in the FINAL form.
- 42 rules are index-anchored: mid-derivation full builds meaningless;
  end gate = retire-all-42 + full SHA1. Sandbox rotation-blind. Twin
  csmd4 (~:399) shares text — anchors must be marionation-unique
  (`_2` suffix / partition at "s32 marionation_Exec").
- csmd4 payoff: its last 5 rules are THIS block; answer transfers.

## Pointers
- tmp/probe_mar.py (tags t0..w17) + probe_mar_{disp,sched}.py +
  mar_{sched_section2,rtl_order,block_regs}.py + sweep_mar_printf.py
  (all restore src). tmp/gccdbg/cc1 = instrumented (env-gated
  SCHED/PRIO debug; build/cc1 = canonical May-18).
- memory/wip/cpu_side_move_dir_4/notes.md; [[register-alloc-pure-c]].
