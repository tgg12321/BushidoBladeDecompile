# Evidence bank — func_80045294

## s1 (recon, 2026-07-18)

- **Floor: 2** (sandbox --disable all; 83/83 insns; 0 rules, 78 cheat-asm-stripped lines are file-wide, not this func).
- **Exact residual diff** (tmp/grind/func_80045294/s1/diff_baseline.txt, target extracted from
  disc/SLUS_006.63 @ file offset 0x35A94, 83 insns): the ONLY divergence is one prologue cluster —
  - target: `sw $16,16($sp) ; move $16,$18 ; sll $3,$18,0x4`
  - build:  `sll $3,$18,0x4 ; sw $16,16($sp) ; move $16,$18`
  Every register allocation, every branch, both loops, the epilogue: byte-identical. The residual is
  purely the placement of `sll $3,$18,4` (v1 = a0<<4) relative to the `$16` save/copy (i = a0) —
  a sched2 ordering of two independent insns that both depend only on `move $18,$4`.
- **RA fingerprint of the matching form**: a0→$18(s2), a1→$19(s3), sum→$17(s1), i→$16(s0),
  s4(table val)→$20, s5(s4+a1)→$21, v1(shift)→$3, count→$4. Init order in C must be
  sum, v1, s4, i, count, s5 (v1 BEFORE i — see H1 kill).
- **H1 kill detail** (i = a0 moved before v1 = a0<<4): RA rotates a0→$21, s4→$18, s5→$20 AND cse
  copy-propagates the shift operand to `sll $3,$16` (target reads $18). So the original source
  provably computes `v1 = a0 << 4` textually before `i = a0`; the move-before-sll order in target
  is a SCHEDULER outcome, not source order.
- **Tie is robust to downstream init reordering**: swapping count/i/s5 init order among themselves
  (keeping v1-before-i) = still 2, same shape (H2, H3 neutral).
- **CSE-reuse spelling adds an insn**: folding the shift into s4's initializer and re-deriving
  `v1 = a0 << 4` after `i = a0` builds 84 insns (extra copy for the reused sll temp) — H4 killed.
- Function already incorporates the documented loop-counter-fills-load-delay recipe (see
  .claude/rules/ index entry for func_80045294, 2026-06-14) — the loop body is fully matched;
  do not disturb `val = *(p+off); off += 0x10; i += 1; sum += val;`.
- No duplicate/sibling leads (tmp/duplicates_leads.txt has no entry for 80045294).
- Scheduling mechanics to beat: in block0 the chain sll→addu $1→lw $20→addu $21(beqz delay) is
  longer than move16→slt→beqz, so GCC 2.7.2 sched gives sll higher priority and emits it first.
  Target emits move16 first despite this. Lever must either shorten sll's downstream chain,
  lengthen move16's, or change the tie some other way — WITHOUT changing pseudo creation order
  (RA fingerprint above) and without moving addu $21 out of block0 (it must stay for the
  beqz delay-slot fill).

- [s1] Honest floor = 2 with 0 regfix/asmfix rules and no non-canonical cheat-asm in func_80045294 (sandbox --disable all this session).

- [s1] 83/83 insn count parity; residual = 2-insn sched2 ordering of `sll $3,$18,0x4` vs `sw $16,16($sp) ; move $16,$18` in the prologue cluster.

- [s1] RA fingerprint of the matching form: a0->$18(s2), a1->$19(s3), sum->$17(s1), i->$16(s0), s4(table val)->$20, s5(s4+a1)->$21, v1(shift)->$3, count->$4. Confirmed identical in build.

- [s1] Original source provably has textual order `v1 = a0<<4` BEFORE `i = a0` (H1 kill). The target's move-before-sll emission is a scheduler outcome, not source order.

- [s1] Neutral reorderings of count/i/s5 init (keeping v1-before-i) do not perturb the score or shape (H2, H3 neutral).

- [s1] Loop body already implements the documented [[loop-counter-fills-load-delay]] recipe (val = *(p+off); off+=0x10; i+=1; sum+=val;) — do not disturb.

- [s1] No sibling/duplicate leads: tmp/duplicates_leads.txt has no entry for 80045294.

- [s1] Scheduling mechanics to beat in block0: chain sll->addu $1->lw $20->addu $21(beqz delay) is longer than move16->slt->beqz, so sched2 elevates sll. A closing lever must either shorten sll's downstream chain, lengthen move16's, or change the tie some other way — WITHOUT altering pseudo creation order and WITHOUT moving addu $21 out of block0 (delay-slot fill).

- [s2] s2 baseline reconfirmed: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 (candidate.c from s1 remains the current in-src form).

- [s2] Residual UNCHANGED: 2-insn sched2 ordering of `sll $3,$18,0x4` vs `sw $16,16 ; move $16,$18` in prologue positions 8-11; all other 82 insns byte-identical (diff still matches tmp/grind/func_80045294/s1/diff_baseline.txt).

- [s2] Guard-form isomorphism: `while(cond) body` and `if(cond) do body while(cond)` lower to the identical CFG in GCC 2.7.2 for this function (probe A neutral).

- [s2] Delay-slot fill dependence: reorg.c requires addu $21,$20,$19 to be BLOCK0-scheduled so it can be pulled into the beqz delay slot; if `s5 = s4 + a1` is hoisted into if(sum!=0), the delay slot fills with `move $4,$0` (jal arg), the block0 addu $21 disappears, and 4 fewer insns emit (probe B, score 13/79 insns).

- [s2] Operand-order sensitivity: swapping `s4 + a1` -> `a1 + s4` produces a single extra register-choice diff (probe C, score 3), evidence that GCC 2.7.2 does NOT commutatively canonicalize addu RHS operands at expand time.

- [s2] Structural-init-order axis is EXHAUSTED for hand-derivation: v1-before-i is required (H1 kill s1), s5 init position is fixed by delay-slot constraint (H2/H3 neutral s1, B killed s2), guard form is neutral (A s2). The sll-vs-move16 tie survives every pure-C structural rewrite tried.

- [s3] [s3] MEASURED via cc1 -da RTL dump (base.i.sched2 line 20111-20155): block 0 insns sll(14) and move16(22) BOTH have INSN_PRIORITY = 1. The s1/s2 CONFIRMED chain-length hypothesis (sll chain 4 > move16 chain 3) is falsified — priorities are equal.

- [s3] [s3] sched2 tiebreak read directly from tools/gcc-2.7.2/sched.c:2398-2456: rank_for_schedule falls through priority tie, then hazard-class tie, to `INSN_LUID(tmp) - INSN_LUID(tmp2)`. qsort direction + backward-list-sched of block 0 means: HIGHER LUID is picked FIRST at ready list and thus emitted LATER in forward stream.

- [s3] [s3] Ready-list dump at T-9 for baseline candidate shows `22 14 12 6` (LUID descending) with insn 22 (move16) at front — picked first, scheduled at position 10 (emitted last of the sll/sw/move16 cluster). Insn 14 (sll) picked at T-11 → scheduled at position 8 (emitted first). Matches observed build order exactly.

- [s3] [s3] CONCLUSION: to reach target's `sw ; move16 ; sll` order, sll's assignment LUID must be GREATER than move16's LUID — i.e., in C source, `v1 = a0<<4` must appear AFTER `i = a0`. But every direct rewrite of this shape (H1, decl-init-decouple) rotates RA identically. RA priority follows assignment LUID too; the two axes are coupled at C-source-level.

- [s3] [s3] NEAR-HIT (rejected/cse-fold-anon-shift.c): inlining a0<<4 into s4's initializer while re-declaring v1 later gives the target schedule ORDER (move16 before sll) with correct RA, at cost of one extra `move v1,v0` copy (score 2, 84 insns vs target 83). Decouples the LUID axis from the RA axis for the first time in this function's grind — the closing lever is somewhere in this shape's neighborhood, minus the CSE-move copy.

- [s3] [s3] STRUCTURAL AXIS `count/i/s5` position while v1-before-i is confirmed neutral to include `i between v1 and s4` (new permutation).

- [s3] [s3] Split-init-accumulation on s5 is HARMFUL here (rose 2->11 with same RA rotation as H1) — the sanctioned split-init lever only helps when target's RTL already contains a matching copy insn; here target has none.

- [s3] [s3] For-init loop shape lowers to the same block-0 RTL LUID as `s32 i = a0; if/do-while` — neutral.

- [s4] [s4] Baseline reconfirmed: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 (candidate.c from s3 remains the current in-src form).

- [s4] [s4] Permuter workspace via tools/decomp-permuter/import.py against src/text1a_c.c FAILS with (a) pycparser syntax error on the whole-function inline `__asm__(...)` block for func_80044010 (base.c malformed at that region even with `_permuter_ignore_line` prefix) AND (b) compile.sh crashes at MASPSX with 'too many values to unpack (expected 2)' after cc1 emits ~10 conflicting-type warnings (D_80095328, D_800A9D04, func_80042874, func_8004574C) — the cpp expansion collides header decls with file-scope decls in text1a_c.c, a fragility of the TU relative to the permuter's cpp-first pipeline. See tmp/grind/func_80045294/s4/permuter_workspace_setup.txt.

- [s4] [s4] Cross-scope CSE across a file-scope subexpression and a nested loop-scope re-declaration does NOT fold in GCC 2.7.2: `s32 s4 = *(base + (a0<<4)); if (...) { s32 v1 = a0<<4; ... }` produces two distinct sll insns and a 35-point RA cascade.

- [s4] [s4] Second-angle proof that inlining a0<<4 into s4's init yields target's schedule order + CSE-move copy: `s32 v1; s32 s4 = *(base + (a0<<4)); ...; v1 = a0<<4;` (decl-early + assign-late) produces sw/move/sll/move-copy at 84 insns — identical closing shape to cse-fold-anon-shift.c from a different C spelling.

- [s4] [s4] Third-angle proof RA priority is COUPLED to assignment LUID (not decouple-able via nested block scope): inner-block wrap of s4/s5/body around a statement-position `v1 = a0<<4` produces the exact H1-kill shape (score=11, RA rotates, sll operand CSE-propagated to $16). global.c allocates function-wide, not per-block.

- [s4] [s4] Closing-lever constraint SHARPENED: any C spelling with TWO tree-level a0<<4 expressions produces the CSE-move copy; any C spelling with ONE tree-level a0<<4 expression that assigns LATE forces RA to follow the late LUID and rotates. The two-copy problem and the rotation problem are two faces of the same constraint at the C level.

- [s5] [s5] Baseline reconfirmed: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78 (candidate.c from s3 remains the in-src form; s5 edits reverted at session end).

- [s5] [s5] Target's first-loop body uses base+index addressing: `lui $1,0x0 ; addu $1,$1,$3 ; lw $2,0($1) ; ... ; addiu $3,$3,16` (v1 is $3, a running OFFSET). A walking-pointer C spelling would produce `lw ,0($ptr)` without the addu step — cannot byte-match. Frontier #1 killed by direct target-bytes read.

- [s5] [s5] GCC 2.7.2 expand_mult normalizes constant power-of-2 multiplication to (ashift RTX) at the front-end, before tree_LUID is assigned: (u32)a0*16u, a0*16, and (u8*)0+a0 route all produce identical byte output. Any C expression evaluating to a0*16 collapses to the same subtree; no LUID shift is achievable from the arithmetic-tree-shape axis.

- [s5] [s5] Combined with s3/s4 evidence: the ONLY spellings that shift LUID are those with either (a) a second tree-level a0<<4 subtree (adds +1 CSE-move copy) or (b) a late-assigned single-tree-level a0<<4 (rotates RA to $21). No pure-C spelling reachable by hand-derivation from the arithmetic-form axis breaks the coupling without incurring one of these costs.

- [s5] [s5] Permuter workspace NOT built this session; s4's blocker (whole-function __asm__ block for func_80044010 + cpp-expansion conflicting decls) still applies. See tmp/grind/func_80045294/s5/permuter_workspace_status.txt for the narrowed search space a future permuter session should target.

- [s6] [s6] Baseline recon reconfirmed at end of session: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src reverted to s3 candidate.c after RTL dump.

- [s6] [s6] Baseline greg (tmp/grind/func_80045294/s3/base.i.greg:14006-14026): single pseudo 75 allocated to $3 (v1), globally, holding the ashift result AND serving as loop base. No copy insn.

- [s6] [s6] Cse-fold-anon-shift.c greg (tmp/grind/func_80045294/s6/base.i.greg:14006-14030): two pseudos — 76 (anon, `76 in 2`/v0, single-block, local-alloc pool) and 81 (declared v1, `81 in 3`/v1, loop-carried, global-alloc pool). Insn 29 `(set 81 76)` with REG_DEAD 76 survives all passes.

- [s6] [s6] Combine (base.i.combine:12633) declines to substitute insn 14 into insn 29 because pseudo 76 has two uses (insn 18 address + insn 29 copy). Multi-use blocks combine's substitution; MIPS mem addressing cannot accept a nested ashift as a base subexpr so combine cannot fold into insn 18 either.

- [s6] [s6] Named pass and decision producing the divergence: local_alloc() (local-alloc.c) places single-block pseudo 76 in $2 BEFORE global_alloc() (global.c) places multi-block pseudo 81 in $3 — GCC 2.7.2 has no cross-pass coalescer (register coalescing was added in later GCC releases), so the two pools cannot merge equivalent pseudos linked by a copy where source dies.

- [s6] [s6] Two-axis coupling reformulated at pass level: (a) two-tree branch's +1 copy is uneliminable due to local/global pool split with no coalescer; (b) one-tree-late-assign branch's RA rotation is driven by global_alloc's reg_n_refs priority scoring against assignment LUID. Both branches sit on GCC-pass mechanics with no C-source lever known to defeat either.

- [s6] [s6] Frontier #2 KILLED at mechanism: 'make global.c see [76 and 81] as a single equivalence class' has no execution path in GCC 2.7.2 — the pass that would do it does not exist. Any pure-C respelling that keeps v1 loop-live AND introduces a second tree-level a0<<4 recapitulates the pool split.

- [s6] [s6] Frontier #1 (directed permuter over PERM_STMT_LIST / PERM_ADD_SUB) remains un-measured; its search space is over C spellings the hand-derivation has not considered, so pass-split mechanism doesn't preclude a permuter find — the search would look for spellings whose expand routes through a different pass sequence entirely (e.g. mutations that eliminate the second tree-level shift altogether while still shifting sll's LUID).

- [s7] [s7] Baseline reconfirmed pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Rotation form (decl-init-decouple applied to src) measured score=11 with edits, then reverted.

- [s7] [s7] Rotation cse pass dump (tmp/grind/func_80045294/s7/base.i.cse:12610+) shows the ashift at insn 17 has operand (reg/v:SI 77) — pseudo 77 is `i`, not pseudo 72 (a0). Baseline cse pass dump (s3/base.i.cse:12610+) shows the ashift at insn 14 has operand (reg/v:SI 72) — pseudo 72 is a0. CSE substitution proven at pass boundary, not inferred.

- [s7] [s7] a0's live-range measurement: baseline pseudo 72 has TWO reads (insn 14 ashift + insn 22 i-move) → live range 4→22; rotation pseudo 72 has ONE read (insn 15 i-move only, then substituted away by CSE) → live range 4→15. ~50% shortening of a0's live range.

- [s7] [s7] global_alloc queue order shift measured directly in greg dumps: pseudo 72 moves from position 8 (baseline) to position 10 (rotation). By position 10, $16-$20 are claimed by higher-priority pseudos, forcing 72 into $21 — reproducing the H1 cascade (a0->$21, s4->$18, s5->$20).

- [s7] [s7] Pass identified as ROOT CAUSE of the one-tree-late-assign branch RA rotation: cse.c operand substitution at insn 17 (ashift). s6's global_alloc reg_n_refs naming is a downstream effect, not the primary driver.

- [s7] [s7] The cse.c fold is deterministic given (set new_pseudo a0_pseudo) precedes (ashift a0_pseudo K) in the same basic block. No pure-C intervention (self-assign, void-cast, tautological guard) can break value-equivalence without changing semantics. Combined with s6's named local/global pool split for the two-tree branch, BOTH near-hit branches are pass-level dead ends for hand-derivation.

- [s7] [s7] Corollary for frontier #1 (permuter): the s6/s7 pass-level analyses do NOT preclude a permuter find because permuter mutations can reach C shapes routing through a different expand→cse→global path entirely (e.g. spellings that avoid the cse-fold trigger while achieving the LUID movement). Permuter workspace blocker (s4/s5) unchanged: whole-function inline asm + cpp-conflict decls in TU; extract-and-build path for a single-function .c + target.o from disc bytes remains the un-blocker.

- [s8] [s8] Baseline reconfirmed pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c reverted to s3 candidate.c after Novel 1 and Novel 2 measurements.

- [s8] [s8] m2c reconstruction (tools/m2c/m2c.py --target mips-gcc-c on asm/funcs/saTan0Init.s) produces the SAME structural shape as the current candidate — nested guarded do-while + sum!=0 branch + two-loop scan — with two distinctive local differences: i-before-v1 init order (H1 shape, known KILLED) and inline D_800A33AC (no cached `count` local, measured NEUTRAL this session). No new structural lever surfaced.

- [s8] [s8] Novel 1 (i-before-v1 with ashift operand SPELLED as `i` rather than `a0`) tested cse.c's substitution-direction preference: does cse.c substitute pseudo 77 (i) back to pseudo 72 (a0) in the ashift, or accept the (ashift 77) as-emitted? Measurement: score=11 identical to s3/s4 rotation shapes. cse.c accepts the initial (ashift 77) and does NOT rewrite it to (ashift 72). a0's live range still collapses at insn 15 (i=a0), reproducing the H1/decl-init-decouple/inner-block-defer-v1 cascade.

- [s8] [s8] Novel 2 (drop `count` local, inline D_800A33AC in first-loop guards) measured NEUTRAL (score=2). GCC 2.7.2's licm allocates a loop-invariant pseudo for the load either way; the C decl vs inline distinction is codegen-invisible for a global read that's used in the loop compare. Confirms caching D_800A33AC in a local is a free axis (like the s1 count/i/s5 permutation cluster).

- [s8] [s8] Kengo symbol confirms function identity: saTan0Init at 0x00147dc8, size 0x14c=83 insns (src/sato/sa_tan0.c). But Kengo source is NOT in the repo — only kengo_functions_full.txt (symbol names + sizes + source-file names) and the ELF. Kengo-source transplant is unavailable.

- [s8] [s8] All three rederive axes — fresh m2c decompile, decomp.me corpus (blocked: curl_cffi missing + Postgres unreachable), Kengo transplant (blocked: no source) — surveyed. Only m2c produced testable output; its two distinctive levers both mapped to known-KILLED or NEUTRAL axes. No new C-shape lever produced by rederive.
