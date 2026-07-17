# Hypothesis ledger — motion_SetMotion

## [s2] F1 (flagship): the committed build's RTL contains a concrete, honest feature at the -1 sites that blocks find_cross_jump, absent at the 13 sites, and it is transplantable.
- mechanism: sched2 list scheduling hoists case-10's `set s0<--1` (insn 374) up beside `s1<-0` (insn 358) because the block has 6 independent store insns of slack; the pre-jump insn becomes `sb D_800A3350` (insn 371), so find_cross_jump iter1 mismatches set-reg vs set-mem -> 0 counted matches -> no merge either direction (jump.c:2532 needs last1!=0; label bonus cannot rescue a zero-match pair). reorg's fill_simple_delay_slots then pulls the hoisted li back into the j's delay slot, restoring the byte-identical [j; delay li -1] twins.
- probe: Regenerated -da dumps of the committed TU first-hand (dump_jump2.sh -> committed.i.sched2/.jump2, 2026-07-17 17:18); s2b_pairs.py printed the full prev/next-chain neighborhoods of all four blocks; cross-checked final layout in committed.s (line 296-300: sb; j .L244; delay li $16,-1).
- result: Discriminator identified and verified end-to-end: case-8 = [LABEL 288; set -1 (291); j 293], case-10 = [set s1,0 (358); set s0,-1 (374) HOISTED; ...4 stores...; sb (371); j 376]. Half CONFIRMED, half KILLED: the feature is real and honest, but it is NOT transplantable — it requires >=1 independent block-local insn, and target fixes the 13 blocks at exactly [jtbl-label; j; delay li13] / [beq; delay li15; j; delay li13], so any slack insn adds bytes. Byte-count closes the transplant.
- verdict: CONFIRMED

## [s2] Arm re-association (set13 hoisted above the compare: `sel = 0xD; if (v0 == 10) sel = 0xF;`) or branch-sense swap (`if (v0 == 10) sel = 0xF; else sel = 0xD;`) changes the arm's jump2 suffix enough to block the merge while keeping target bytes.
- mechanism: If the pre-jump insn on the arm side were the condjump or a different set, iter1 would mismatch.
- probe: s2b_probe.py r1 and r3 vs m0 baseline (label-normalized cc1 asm diff).
- result: KILLED: both canonicalize to the SAME shape — branch sense flips to bne with li13 in its delay (target has beq + li15) AND the case-9/11 block still merges (361 insns vs m0's 363; 122 diff lines each). Doubly wrong.
- verdict: KILLED

## [s2] Type narrowing sel to s16 (+ honest 0xD) perturbs the set13 RTL mode enough to break rtx_renumbered_equal_p at iter1.
- mechanism: Narrow-typed local could change set modes or insert extension insns between set and jump.
- probe: s2b_probe.py r2.
- result: KILLED: 364 insns, 658 diff lines vs m0 — massive whole-function divergence (extension insns, layout shift), nowhere near byte-neutral.
- verdict: KILLED

## [s2] Block-local const split at case 9/11 (`{ s32 d13 = 0xD; sel = d13; }`) survives to jump2 as a distinct pattern.
- mechanism: A separate pseudo staging the constant could make the pre-jump insn a reg-reg move.
- probe: s2b_probe.py r4.
- result: KILLED: cse/const-prop folds it to the plain set13 before jump2; merged shape (361 insns), identical to the honest-0xD control.
- verdict: KILLED
