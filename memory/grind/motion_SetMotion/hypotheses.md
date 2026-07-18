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

## [s3] Some structural spelling outside r1-r4 (goto-sharing, physical case order, jump-thunk, sanctioned split-init, hoisted-common-set) survives to jump2 as a distinct pattern or blocks the merge byte-neutrally.
- mechanism: Each candidate perturbs a different canonicalization surface: r5 block duplication, r6 emission order, r7 jump_chain shape (m7's sibling), r8 cse const-fold timing, r9 counted-match count (last1==0).
- probe: tmp/grind/motion_SetMotion/s3/s3_probe.py — label-normalized cc1 asm diff vs m0 for r5-r9.
- result: r5/r6/r7/r8 all canonicalize to the identical merged 361-insn shape (86 diff lines each — same as honest-0xD); r9 does block the merge (block vanishes, last1==0 confirmed from the zero-insn side) but diverges 619 diff lines. The sanctioned split-init family is specifically measured dead here (cse folds before jump2).
- verdict: KILLED

## [s3b] A residual structural spelling class outside r1-r9 (duplicate case-9/case-11 blocks, cond-expr arm, unsigned selector type, pre-switch if-extraction of the jtbl label) survives to jump2 unmerged or blocks the merge byte-neutrally.
- mechanism: Each attacks a distinct closure assumption: r10 the "one shared C block" premise (two source-level duplicates could change jump_chain pairing order), r11 the cond-expr expansion surface (vs if/else statement forms r1/r3/m8), r12 the set13 RTL mode/guard shape via unsignedness (r2 only measured s16), r13 the stream-1 label bonus itself (no jtbl CODE_LABEL heading the block if 9/11 branch in from an if).
- probe: tmp/grind/motion_SetMotion/s3/s3b_probe.py — label-normalized cc1 asm diff vs m0 (363 insns) with honest-0xD control h0 (361 insns / 86 diff lines).
- result: r10 and r11 = byte-identical to the merged h0 signature (361 insns, same 86 diff lines); r12 = 149 diff lines (unsigned vacuates `sel >= 0`, dispatch tail restructured, still merged); r13 = 369 insns / 126 diff lines (2 extra compare pairs + jtbl rewrite — the label bonus is only avoidable at real byte cost). Fourth independent confirmation of the closure theorem.
- verdict: KILLED

## [s3] A residual structural spelling class outside r1-r9 (duplicate case-9/case-11 blocks, cond-expr arm, unsigned selector type, pre-switch if-extraction of the jtbl label) survives to jump2 unmerged or blocks the 13-pair merge byte-neutrally.
- mechanism: r10 attacks the one-shared-C-block premise (source-level duplicates could change jump_chain pairing order); r11 the cond-expr expansion surface (vs if/else forms r1/r3/m8); r12 the set13 RTL mode/guard shape via unsignedness (r2 only measured s16); r13 the stream-1 label bonus itself (no jtbl CODE_LABEL heading the block when 9/11 branch in from a pre-switch if).
- probe: tmp/grind/motion_SetMotion/s3/s3b_probe.py - label-normalized cc1 asm diff vs committed baseline m0 (363 insns) with honest-0xD control h0 (361 insns / 86 diff lines).
- result: r10 and r11 byte-identical to the merged h0 signature (361 insns, same 86 diff lines - cross-jump collapses the 9/11 duplicates first, survivor still merges with the ==3 arm); r12 = 362 insns / 149 diff lines (unsigned vacuates the sel>=0 dispatch guard, new j+move tail, still merged); r13 = 369 insns / 126 diff lines (2 extra beq/li compare pairs + rewritten jtbl entries - the label bonus is only avoidable at real byte cost).
- verdict: KILLED

## [s4] The discarded prior s4 attempt's perm_a output-0-1 (permuter score 0 with --stack-diffs, honest-0xD chassis) is a true byte-match closing form for the 13-pair wall.
- mechanism: Mutation moved load_sel2 (sel2 = D_800A3350) from case 13/17 into the ==3 arm; at jump2 the arm suffix [set13; lbu; j] cross-jumps its [lbu; j] tail into the shared lbu block, redirecting the arm's jump to a different CODE_LABEL than case-9/11's -> the 13-pair merge cannot fire (both li 13 survive, 402 insns).
- probe: Recompiled output-0-1/source.c through the exact build pipeline (cc1|prologue_fix|maspsx|as) and raw instruction-word diffed vs target.o (tmp/grind/motion_SetMotion/s4/verify_find.sh); calibrated noise floor with committed.c (exactly 1 li word + 3 jtbl-reloc lws).
- result: 6 real word diffs: the ==3 arm's beq+j target the lbu block (0x164) instead of sel_dispatch (0x26c) and case-9/11's j targets 0x26c instead of 0x164 — jump destinations SWAPPED vs target. Also semantically wrong both ways (arm dispatches sel2=lbu where target dispatches -1; case-13/17 loses its sel2 load). The permuter scorer's label normalization cannot see swapped branch targets: score 0 != bytes 0 on this function.
- verdict: KILLED

## [s4] A permuter basin (random honest-0xD chassis A / directed PERM_GENERAL chassis B) contains an honest byte-neutral spelling that keeps both 13-sites unmerged.
- mechanism: Random mutation explores CFG/statement geometries outside the hand-enumerated r1-r13/m1-m9 space; any iter1-breaking real insn between set13 and j blocks find_cross_jump.
- probe: Adopted, harvested and stopped both campaigns from the discarded 2026-07-18 attempt (perm_a: 112,618 iterations / 13.1h / 12 finds; perm_b directed: 2,049 iterations / 13 finds, best 100); raw word-diffed ALL 12 perm_a finds vs target (rawdiff_all.sh).
- result: Every find classified: score 100-200 forms = merged-shape layout divergence (119-125 real word diffs); the only sub-100 wins are score-0 (jump-target swap, above) and score-60/120 (SEMANTIC THEFT: a live store moved from case-10's D_800A3207=5 path into case 9/11 — D_800A3350=0 resp. D_800A334C=0x5A — real sb between set13 and j blocks the merge, but the store executes on the wrong path and sits 11 slots from target's position, 6-8 real word diffs). Zero honest finds. The scorer's floor is occupied by false matches, so scorer-guided search on this wall is metric-invalid by construction.
- verdict: KILLED

## [s4] The 'two different CODE_LABELs at jump2 time' unmerge family (s1's original WIP conjecture) is honestly reachable in a byte-matching compile.
- mechanism: If the two set13 jumps target different labels they sit in different jump_chains and find_cross_jump never pairs them; output-0-1 is the first measured instance of this family actually firing.
- probe: Measured via output-0-1's raw diff + source-level analysis against the s1-mapped do_cross_jump (jump.c:2536-2583): the redirect label is placed BEFORE the partner's matched insns, so the redirected jump's target word can only equal target's 0x26c if the matched insns are byte-free.
- result: The family fires only by (a) routing one 13-site through the shared lbu block — semantically forbidden on both 13-paths since both must dispatch with sel2=-1 (target words differ, measured) — or (b) byte-free matched insns, i.e. the USE/CLOBBER manufacture family the Judge ruled unsanctioned (decisions.md 2026-07-17 17:09). Closure theorem now confirmed from a fifth independent direction, this time by blind search.
- verdict: KILLED
