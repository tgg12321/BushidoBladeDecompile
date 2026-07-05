# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3: TARGET SHAPE + NOP reproduced in minis (2026-07-04 s6/s6b)

## TL;DR
Floor unchanged (masked 4 = region-1 pair 2 + region-3 2; candidate.c = mh5).
SESSION-6b (round-5, tmp/mar_trigger_gen5.py r5c/r5d): the EXACT target shape
[beqz→after; NOP; sb; la; move; beqz a1; li…] falls out of natural C when the
arm's first statement carries a label with a SECOND referencer — the label
(zero bytes) un-owns check2's fall-through thread and the steal is refused in
BOTH passes. THE WHOLE REMAINING PROBLEM = the referencer's bytes: it must
survive pass-2 fill_eager, die by end of pass-2 relax, and its CONDITION
insns must also cost nothing (use an already-live reg, e.g. `check`/a2 —
runtime semantics are IRRELEVANT since it is deleted before final; it only
needs to dodge pre-flow folding). Relax deleter catalog now COMPLETE (below).
Pass-2 retry activity confirmed real in this TU (tmp/mar_pass_trace.py: 56
branches multi-attempted, 36 wins). All single-mechanism referencer
lifecycles derived so far collapse into a recursion (each scaffold needs its
own scaffold) — the open construction is a pass-2-ONLY adjacency/steal that
lets one of the deleters fire in pass-2 relax specifically.

## Region-3 — mechanism map (s6/s6b, source-verified)
- dbr runs {fill_simple ×2; fill_eager; relax} TWICE. Scaffolding that dies
  in pass-1 relax is UNDONE by the pass-2 fill retry (retries confirmed live
  in this TU); it must survive pass-2 fill_eager, die by end of pass-2 relax.
- CONFIRMED SHAPE (r5d/r5c, _gen5.py): give the arm's first stmt a label
  with a second referencer (`if (G3) goto arm;` + `if (check) goto arm; goto
  after; arm: sb; …`) → compiles to the EXACT target shape [beqz→after;
  BARE SLOT; sb; la; move; beqz a1; li 7…]. The zero-width label un-owns the
  fall-through thread → steal refused BOTH passes. Only cost = the
  referencer (+its condition insns — use an already-live reg (check/a2):
  runtime semantics irrelevant, it dies pre-final; must dodge pre-flow folds).
- everything-live ⇔ find_basic_block==-1 ⇔ target label minted POST-FLOW
  (flow.c: every flow-era label is its own basic_block_head — retarget/
  deletion tricks on flow-known labels can never give -1). Minters: jump2
  do_cross_jump (mid-block ⇒ walk-up finds b_far + blind SET-scan, not -1);
  jump2 USE-hoist (jump.c 679, wrong side); relax redundant-first-insn chain
  (3992-4006, mint 4002): re-computed `check = *(idx_1496-1) & new_var3;` at
  after_blocks = lbu+andi redundant along check2 → chain-mints to exactly
  .L812C4; orphans cleaned byte-free by delete_insn's label-cascade (jump.c
  3509: barrier-preceded deleted label deletes unreachable code after it;
  user labels → NOTE). LOAD-BEARING GUARD: the 3992 chain is in relax's
  FILLED-SEQ-ONLY section (3946) — an unfilled beqz never reaches it, so
  label-refusal (unfilled) and the mint CANNOT compose naively; mint route
  needs check2 FILLED + slot EMPTIED via 3953-61 (slot insn must become
  redundant — the stolen move isn't; arm-1's move is on the untaken arm).
  4003's reorg_redirect_jump has NO delay-slot safety gate.
- RELAX DELETER CATALOG (complete): [unfilled cond] 3866 delete if
  next_active(insn)==next_active(target); 3876 invert-around-bare-j (deletes
  the j); 3913 cond+j SWAP (mostly_true-gated, redirect only). [filled SEQ]
  3956 drop redundant slot insn; 3992/4002 redundancy redirect+MINT; 4031
  branch-to-next → delete jump + re-emit slots inline; 4067 j-around-
  identical-insn; 4080 filled invert-around-bare-j (flips FROM_TARGET).
  Labels at 0 refs → NOTE at every redirect_jump. DBR START: all cond jumps
  re-pointed to LAST of consecutive labels (kills double-label tricks).
- REFERENCER LIFECYCLE constraint set: every deleter's precondition either
  held already in pass-1 (fires too early) or needs an insn X between
  referencer and label to vanish pass-2-ONLY. X=sb unstealable (memory);
  X=la ineligible (length 2); X=move blocked (a1 read by guard → live at
  arm). Inversion deleters RETARGET their cond branch onto the label (keeps
  it alive, real bytes). Open: a pass-2-only steal/adjacency (pass-1 relax
  transforms + bb_ticks invalidation are the available asymmetries — the
  SWAP exists explicitly to enable pass-2 fills).
- MEASURED DEAD: &&label dead-take (tree-eliminated, k-battery); rounds 1-2
  topology batteries (27 variants at after_blocks) all steal; loop-top
  labels die pre-flow; h2 recheck-alone steals (pass-1 fill precedes relax);
  round-6 (_gen6.py s1-s5): NEAR-label referencers of ANY condition
  (check==0 / check!=0 / a0==0, label 0-2 stmts away, before/after sb) all
  fold pre-flow to the exact baseline — referencer survival REQUIRES
  distance from the label (r5d's far guard survived).
- NEXT: (i) minis where a pass-1 SWAP/inversion sets up a pass-2 fill that
  creates the referencer's branch-to-next deletion; (ii) filled+slot-
  emptying: pass-1 steal an insn that a pass-1 relax case-4031 inline
  re-emission makes redundant → 3956 empties → 3992 mints → pass-2
  everything-live; (iii) read fill_simple_delay_slots fully for missed
  unfilled-branch transforms; (iv) far-referencer variants whose branch
  bytes COINCIDE with a real function branch (audit every real branch in
  the fn for retarget-to-arm compatibility during reorg).

## Arm-2 transposition (2 masked lines): unchanged from s5 — git history
(d1/d2 tie at 952; ref-bump program CLOSED: exact s-alloc, no byte-clean form).
## Region-2 SOLVED — recipe unchanged (in candidate.c)

## Region-1 pair — permuter verdicts (session-6)
- perm_mar6 output-200-1/2 (arg5 value-staging via status): honest masked 8 —
  staging breaks the register story (lbu v1/lw s0 vs lbu a0/lw v1). REJECTED.
- output-200-3 (goto-loop moved into the arm): masked 5, registers in region-1
  all correct but the pair becomes a 3-insn rotation {sll a0 | sll v0, addu}
  and region-3 unchanged. REJECTED (worse than mh5's 4).
- csmd4 output-40-2: masked 6 vs floor 2. REJECTED.
- Conclusion: the g3-family residual (v1/a0 qty exchange) remains the
  region-1 frontier per 5b; the blind search keeps confirming mh5's basin.

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495 s6,
  arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot = NOP.
- .L812C4: beqz s7→.L810A4 slot=move v0,0; falls into epilogue (.L812CC).
- Tail has TWO identical [j .L812CC; slot move v0,a2] (arm-1 end + .L812BC):
  not cross-jumped because jump-vs-jump find_cross_jump needs minimum=2
  matching insns and only 1 matches. Arm-1's beqz s4 DID steal its move.
- Base ALLOCDBG s-order: p82 952 / p76 933 / p78 933 / p73 930 / p81 657 /
  p77 405 / p72 256.

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin csmd4
  (:388) shares text — marionation-unique anchors. Declare new_var/new_var3.
- d1/d2 stmt reorders flip s-regs via the 952 tie (do NOT re-derive).
- knob uids shift with any C change — tmp/mar_alllive_full.py re-discovers.

## Tools (all local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive: BB2_DBR_DEBUG, BB2_ALLLIVE_LABEL, ALLOC/QTY/
  SCHED dumps. Source: tools/gcc-2.7.2 (read-only reference).
- Session-6 empirical harness: tmp/mar_trigger_gen.py (topology battery 1),
  _gen2.py (loop-note battery), _gen3.py (h1 mid-thread label CONFIRMED NOP;
  h2 recheck steals), _gen4.py (&&label DEAD), _gen5.py (r5c/r5d TARGET
  SHAPE + NOP via second-referencer arm label), tmp/mar_trace_labels.sh
  (label lifecycle per pass), tmp/mar_pass_trace.py (real-candidate DBRDBG
  fill trace; pass-2 retries confirmed). Minis in tmp/mar_trig/.
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh. Round-6 minis: tmp/mar_trigger_gen6.py.
