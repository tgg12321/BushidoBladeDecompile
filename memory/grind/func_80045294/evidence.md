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

- [s9] [s9] Baseline reconfirmed pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean throughout; no src edits this session. Artifact: tmp/grind/func_80045294/s9/baseline_recon.txt.

- [s9] [s9] curl_cffi was already present in .venv (pip verify returned 'Requirement already satisfied'); s8's 'blocked: curl_cffi missing' was a stale diagnosis. tools/decomp_me_scrape.py search runs cleanly against the local corpus roots under tmp/grind/cpu_side_move_dir_4/s44/corpus/{gcc272psx,gcc272cdk,psyq35}.

- [s9] [s9] Shingle-search top similarity across all three toolchain-family corpora (~5.4MB gcc272psx + gcc272cdk + psyq35): 0.127 (gcc272cdk/Z60NJ), 0.124 (KrpLx), 0.098 (psyq35/Tu4L2), 0.089 (gcc272psx/5t0dj). No match crosses even a 0.15 similarity threshold.

- [s9] [s9] Body inspection of the four top-similarity score=0 hits: Z60NJ (func_8002DBF8) is a GP-swap SFX-dispatch wrapper, no accumulator loop; KrpLx (func_8009D174) is a 4-arg guard-chain with no loop; 5t0dj (func_8003AAEC) is a billboard-slot allocator with a single conditional block; pQrLs (func_80024024) is a masked-index field-match search loop with s16-cast prologue. None resembles the guarded do-while + shift-index accumulator + sum!=0 branch + second base+index-loop shape of saTan0Init.

- [s9] [s9] Combined with s8 evidence: all three rederive axes are now measured dead for this function -- m2c reconstruction (s8, produced only i-before-v1 shape KILLED + inline-D_800A33AC NEUTRAL), Kengo source transplant (s8, source not shipped in repo -- only symbol metadata), decomp.me corpus (s9, no >0.15 similarity match anywhere in the local scraped corpus). The rederive modality has no residual hand-derivation lever to explore for this function.

- [s9] [s9] Frontier updates preserved from s8: (1) permuter-workspace un-block via extract-minimal-single-function .c + target.o from disc bytes @0x35A94, then directed PERM_STMT_LIST + PERM_ADD_SUB + PERM_DUMMY_COMMA_EXPR + PERM_REORDER_DECLS sweep; (2) novel semantic-invariant CFG-split rewrite that puts i=a0 and v1=a0<<4 in distinct basic blocks (defeats s7's cse.c BB-scoped substitution) but requires an existing semantic bounds check that saTan0Init lacks -- likely requires (1).

- [s10] [s10] Baseline reconfirmed pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean throughout; src edits reverted after H10 measurement. Artifact: tmp/grind/func_80045294/s10/synthesis_summary.md.

- [s10] [s10] H10 (do-while(0) around v1=a0<<4 after i=a0, FAKE-annotated, owner-sanctioned per do-while-zero-exception) MEASURED KILLED at score=12, build_insns=83. Rejected form saved to memory/grind/func_80045294/rejected/dowhile0-around-v1-after-i.c with the two mechanism-plausible explanations for the kill.

- [s10] [s10] The ONE no-semantic-purpose wrap sanctioned in BB2 source (do-while(0), per owner ruling 2026-07-06) is NOT a lever for defeating the s7 cse.c BB-scoped operand substitution on this function. Whether jump.c folds the wrap before cse.c runs (mechanism a) or cse.c spans fall-through edges (mechanism b), the outcome is identical: pseudo 77 (i) still substitutes for 72 (a0) in the ashift, a0's live range still collapses, RA still rotates.

- [s10] [s10] Frontier #2 (from s7/s8/s9: 'novel CFG-split rewrite that puts i=a0 and v1=a0<<4 in distinct basic blocks defeats s7 cse.c BB-scoped substitution') LOSES its natural realization. The only sanctioned no-semantic-purpose wrap fails to create the needed CFG boundary in cse.c's view. Any surviving realization would require a genuine semantic conditional branch, which saTan0Init lacks (no natural bounds check on a0/a1). Frontier #2 effectively dead for hand-derivation absent a permuter find of a shape routing through a different expand/cse/global path entirely.

- [s10] [s10] Synthesis consolidation: the two GCC-pass-level dead ends (s6 local_alloc/global_alloc pool split with no coalescer for the two-tree branch; s7 cse.c BB-scoped operand substitution for the one-tree-late-assign branch) are together robust to every hand-derivation axis measured across s1-s10, INCLUDING the sanctioned do-while(0) CFG-split lever. The rederive axes are exhausted (s8/s9). The one remaining lever with the ability to reach C shapes hand-derivation cannot enumerate is a directed permuter over PERM_STMT_LIST + PERM_ADD_SUB + PERM_DUMMY_COMMA_EXPR + PERM_REORDER_DECLS on an un-blocked minimal-single-function workspace.

- [s11] [s11] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c reverted to s3 candidate.c after all three probe measurements; working tree clean.

- [s11] [s11] Combined-declarator packing (`s32 v1 = X, s4 = Y;`) does NOT shift tree_LUID or RTL emission versus two separate `s32` statements in GCC 2.7.2 -- declarator lists lower identically at the DECL_STMT level.

- [s11] [s11] The count-before-i permutation (order: sum,v1,s4,count,i,s5) is NEUTRAL, extending the s1/s3 {i,s4,count,s5} free-axis inference to its last un-measured slot; count's decl position within that cluster is a free axis as long as v1-before-i holds.

- [s11] [s11] GCC statement-expression `({ a0 << 4; })` collapses to the bare expression in c-parse.y BEFORE RTL creation -- it emits no NOTE_INSN_BLOCK_* nor JUMP_INSN, so it cannot create the cse-visible BB boundary the s10 do-while(0) probe sought. Independent second-mechanism confirmation of s10's kill via a distinct C-front-end lowering path.

- [s11] [s11] Consolidated across s1-s11: every hand-derivable pure-C structural axis for the sll/move16 sched2 tie in func_80045294 is measured dead. The two GCC-pass-level dead ends (s6 local_alloc/global_alloc pool split with no coalescer for the two-tree branch; s7 cse.c BB-scoped substitution for the one-tree-late-assign branch) plus s10's do-while(0) kill are together robust to declarator packing, free-axis permutations, and the GCC statement-expression alternative CFG-splitter.

- [s12] [s12] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c returned to s3 canonical candidate; working tree clean.

- [s12] [s12] sum's decl position is LOAD-BEARING at position 0, distinct from the free-axis cluster {i, s4, count, s5}. Moving sum to position 5 (after s5) raises score 2 -> 4 -- two register-choice diffs. The free-axis inference from s1/s3/s11 does NOT extend to sum's position.

- [s12] [s12] Introducing a named intermediate local (`s32 aa = a0;`) as an alias for a0 and deriving both `v1 = aa<<4` and `i = aa` from it is NEUTRAL (score=2). cse.c's value-equivalence class merges {a0, aa, i}; the added assignment LUID shifts sll and move16 symmetrically. Rules out named-alias intermediate as a lever independent of the s7 cse.c BB-scoped substitution mechanism.

- [s12] [s12] Destination-type-declaration axis (u32 v1 vs s32 v1) is NEUTRAL for the a0<<4 sll: expand_shift emits `(ashift (reg:SI a0) (const_int 4))` regardless of decl type because the constant shift is signedness-agnostic. Extends s5's arithmetic-form axis kills to the dest-type declaration surface.

- [s12] [s12] Consolidated across s1-s12: every hand-derivable structural axis for the sll/move16 sched2 tie -- init position permutations across the entire {sum, v1, s4, i, count, s5} decl block, guard form, s5 hoist, split-init-accumulation, decl/init decouple, inner-block scoping, alias intermediates, arithmetic-tree shape (signed/unsigned/ptrdiff), destination type (s32/u32), declarator packing (combined-declarator), CFG-splitter attempts (do-while(0), statement-expression), and rederive axes (m2c, Kengo, decomp.me corpus) -- is measured dead. The two GCC-pass-level dead ends (s6 local_alloc/global_alloc pool split with no coalescer for the two-tree branch; s7 cse.c BB-scoped substitution for the one-tree-late-assign branch) are robust to all measured axes.

- [s13] [s13] Baseline reconfirmed pre and post: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78. Working tree clean (no src edits this session).

- [s13] [s13] Permuter workspace un-blocked: tmp/grind/func_80045294/s13/perm/ contains base.c (cpp-preprocessed src/text1a_c.c, 2313 lines), settings.toml, compile.sh (cat|cc1|prologue_fix|maspsx|sed|multu_pad + awk-extract + as-assemble), target.o (from asm/funcs/saTan0Init.s renamed to func_80045294), and empty prologue_config sidecars matching engine's empty_overrides recipe.

- [s13] [s13] Root cause of the 2-diff residual: `prologue_fix: 1` in queue.json (tools/prologue_config.json line 280 has an entry) reorders cc1's natural `sll ; sw s0 ; move16` into target's `sw s0 ; move16 ; sll` in the canonical build. The sandbox `disable=all` strips prologue_fix via env-var-driven empty configs, exposing cc1's raw sched2 output as the honest pure-C floor. My workspace must strip prologue_fix identically to reproduce sandbox conditions.

- [s13] [s13] cc1 (GCC 2.7.2) exit=33 with -w -quiet on this TU (conflicting-types on D_80095328, func_80042874, D_800A9D04, too-few-args to func_8004574C at line 2153) but the exit code is non-fatal: when cc1 receives input via STDIN, it emits full 4914 lines of asm to stdout despite the errors; when given the same file as a positional argument, it emits ZERO stdout. compile.sh must pipe base.c to cc1 (not name it as argv[1]).

- [s13] [s13] Directed campaign launch verified via tools/permuter_campaign.py (owner-required telemetry per feedback/permuter-fresh-seed-discipline): pid=430, base_score=60, label=s13-random-fresh1, jobs=4, --stop-on-zero, --stack-diffs.

- [s13] [s13] Harvest at 668s: 17773 iterations, 3 finds total (1 pre-launch from smoke.sh, 2 novel), best_new_score=60 (no improvement over base). Novel find #1: swap `s32 sum = 0` and `s32 v1 = a0<<4` decl order (equivalent-basin free axis, extends s1/s3 permutation cluster). Novel find #2: break v1 by moving `v1 = a0<<4` INTO the loop body while leaving v1 uninitialized at declaration — semantically broken (UB read of v1 in s4's init) but permuter's scorer normalized past it.

- [s13] [s13] Ledger-listed directed macros PERM_STMT_LIST / PERM_ADD_SUB / PERM_DUMMY_COMMA_EXPR / PERM_REORDER_DECLS DO NOT EXIST in tools/decomp-permuter (verified by grep of src/ + tools/permuter_annotate.py HINTS). The un-tested surface for this function is: hand-annotated PERM_LINESWAP around the {sum, v1, s4, i, count, s5} decl block, PERM_GENERAL around `a0 << 4` / `s4 + a1` / the s4 init expression.

- [s13] [s13] Bytes-level proof of workspace fidelity: with standard prologue_fix, base.o (from perm/) byte-matches target.o (0 diff via diff on objdump -Mno-aliases -drz). With empty prologue_config env vars, base.o has sll at position 7 while target.o has sll at position 9 (byte 53 differs) — the exact sched2 tie the ledger describes at s1.

- [s14] s14 relaunch of the directed full-TU PERM_LINESWAP+PERM_GENERAL chassis (tmp/grind/func_80045294/s14/perm) ran 4320 iters in 478.7s to the natural permuter iteration limit; 1 new output-60-2 (score=60, seconds_since_launch=28.3), 0 sub-60. Combined with the prior s14 run (929 iters) the chassis has ~5249 directed iters banked at score-60 plateau.

- [s14] Novel minimal-base chassis built at tmp/grind/func_80045294/s14/perm_min/ — 63-line hand-authored base.c (typedefs + 8 extern decls + func_80045294) vs the 2524-line cpp-expanded full-TU base. Same PERM_LINESWAP annotation over the 6-decl cluster. Smoke test: cc1 -> prologue_fix (empty gates) -> maspsx -> multu_pad -> awk extract -> as pipeline produces base.o=1684 bytes cleanly.

- [s14] Minimal-base permuter campaign (label s14-minimal-base-lineswap) ran 720 iters in 112.4s; finds_total=1 (pre-existing output-60-1, not from this run), finds_new=0, best_new_score=null. Iter rate 6.4/s across 4 jobs is COMPARABLE to (not faster than) the full-TU 9/s rate — the frontier #2 hypothesized 5x iters/sec gain did not materialize; the compile pipeline (not pycparser parse) dominates.

- [s14] Both chassis show the SAME score-60 basin with the SAME distribution of near-hits (60/70/84/98 tiers) observed in the s14-original log — the plateau is chassis-independent and mutation-lever-independent for the specific PERM_LINESWAP+PERM_GENERAL surface tested.

- [s14] src/text1a_c.c:1602-1648 currently carries the inherited candidate.c body verbatim (verified line-by-line); no src edits this session. memory/grind/func_80045294/candidate.c is unchanged.

- [s14] Consistent with ledger s6/s7 pass-level dead-ends (local/global_alloc pool split with no coalescer; cse.c BB-scoped operand substitution) — no C-source structural lever tested here moves the residual 2-insn sched2 tie.

- [s15] [s15] Baseline reconfirmed pre and post: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78. src/text1a_c.c edits reverted after H1 dump; working tree clean.

- [s15] [s15] Sched2 ready-list dumps proved backward-list-sched picks HIGHER LUID first (emits later). Candidate: LUID(sll=14) < LUID(move16=22) -> sll emitted first (wrong). H1: LUID(move16=15) < LUID(sll=17) -> move16 emitted first (matches target sched2 order). Mechanism (b) fully specified with pass-source citation.

- [s15] [s15] CSE pass RTL directly shows the substitution: candidate insn 14 uses (reg 72) = a0 in the ashift; H1 insn 17 uses (reg 75) = i in the ashift (rewritten from a0). Substitution PROVEN at pass output, not inferred from downstream fingerprints.

- [s15] [s15] Greg reg-alloc queue positions measured: pseudo 72 (a0) at position 9 in candidate (gets $18) vs position 11 in H1 (gets $21). 2-slot demotion caused by reg_n_refs dropping from 2 to 1 due to the cse.c substitution. RA rotation fingerprint identical to s1's H1 kill.

- [s15] [s15] The frontier's (a)/(b) discrimination is resolved as BOTH-COUPLED: (b) is the direct sched2 mechanism; (a) is the upstream trigger for every C-source lever that flips LUID. Fix must be upstream of cse.c fall-through per the ledger's own framing.

- [s15] [s15] Consistent with s10 (do-while(0) kill: jump.c collapses before cse.c) and s11 (statement-expression kill: front-end collapses before RTL): no hand-derivable pure-C mechanism creates a cse.c-respected BB boundary between i=a0 and v1=a0<<4 in this function.

- [s15] [s15] Frontier #3 (OWNER-ESCALATION) still not ripe: PERM_RANDOMIZE (frontier #1 from ledger s14) is the last unmeasured sanctioned axis. Driver policy requires every sanctioned axis measured dead before owner-gated.

- [s15] [s15] No rejected form saved this session: the H1 shape used for the discrimination is already banked as rejected/i-before-v1-init.c. candidate.c unchanged (baseline preserved).

- [s16] [s16] Baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean; no src edits this session.

- [s16] [s16] sched1 block-0 forward order at the cluster (from s3/base.i.sched line 19097-19123): insn 12 (pos 1) -> insn 14 sll (pos 2) -> insn 22 move16 (pos 3) -> insn 19 (pos 4). sll emitted before move16, driven by rank_for_schedule INSN_LUID tiebreak (LUID(14)=14 < LUID(22)=22 -> 22 picked first in backward list -> emitted later in forward).

- [s16] [s16] sched2 block-0 forward order at the cluster (from s3/base.i.sched2 line 20105-20157): pos 8 = insn 14 sll, pos 9 = insn 211 sw (callee-save, LUID=211 from reload), pos 10 = insn 22 move16. The sll<move16 relative order matches sched1; sw's placement between them is driven by the ready-list hazard-hoist rule ('insn 211 has a greater potential hazard'), NOT by LUID.

- [s16] [s16] Reload inserts callee-save/restore/frame-setup insns numbered 195-211 into block 0 between sched1 and sched2. The pre-reload insns (4, 6, 12, 14, 19, 22, 25, 28, 31, 32) preserve their INSN_LUIDs across reload; new insns get monotonic LUIDs from 195 upward. Therefore any LUID relation between pre-reload insns is FROZEN through reload.

- [s16] [s16] The wrong sll-before-move16 order is COMMITTED AT SCHED1, not first at sched2. sched2 makes the same LUID-driven choice for insns 14 and 22 that sched1 made; sched2 does not re-decide this pair.

- [s16] [s16] Corollary: a hypothetical build-flag intervention that ONLY disables sched2 (-fno-schedule-insns2) cannot close the gap -- sched1's already-committed order would carry through to final. Similarly, disabling sched1 (-fno-schedule-insns) would leave the natural pre-sched RTL insn order intact, which is IDENTICAL to sched1's output for these two insns because both derive from the same LUID relation. There is no scheduler-side lever independent of the C-source LUID axis for this pair.

- [s16] [s16] Hardens s15's mechanism-(b) conclusion: the LUID-driven sched decision is made TWICE (sched1 and sched2) by the SAME rank_for_schedule tiebreak against the SAME LUIDs. Therefore the s7 cse.c upstream (a) coupling that pins every C-source LUID lever to RA rotation holds against BOTH scheduling passes, not just sched2. The (a)/(b) coupling is stronger than the ledger stated: not one downstream pass but two, and there is no scheduler-only defeat available.

- [s16] [s16] The sw insn's placement between sll and move16 in the final output is confirmed to be a SCHED2-EXCLUSIVE hazard-hoist decision (log line 'insn 211 has a greater potential hazard'). sched2 groups memory ops away from ALU chains for pipeline reasons. This is not a lever -- it does not alter the sll/move16 relative order -- but it explains why the residual is a 3-insn cluster (sw/move16/sll vs sll/sw/move16) rather than a 2-insn sll/move16 swap.

- [s16] [s16] All hypotheses about scheduler-side interventions for func_80045294 are now exhausted. The LUID axis is provably coupled to cse.c (s7), coupled to RA rotation via reg_n_refs (s6), and now shown coupled to BOTH scheduling passes (s16). Any closing lever must be either (i) a pure-C construct that shifts LUID while avoiding cse.c's BB-scoped substitution -- proven impossible for hand-derivation across s1-s15, PERM_RANDOMIZE unmeasured -- or (ii) an owner-sanctioned canonical-asm authorization.

- [s17] [s17] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. No src edits this session; candidate.c unchanged.

- [s17] [s17] m2c cross-target reconstructions (mipsel-gcc-c passes=3, mips-ido-c passes=3) BYTE-IDENTICAL to s8's mips-gcc-c output for the first-loop init cluster (`var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=...`). Artifact: tmp/grind/func_80045294/s17/m2c_mipsel_gcc_p3.txt + m2c_mips_ido_p3.txt.

- [s17] [s17] The convergent m2c reconstruction is the H1 shape (i-before-v1 + `arg0 * 0x10`) — banked KILLED at s1 (score=11 via cse.c substitution rewriting the ashift operand from a0 to i, RA rotates a0->$21) and at s5/s8 (arg0*K -> ashift fold measured NEUTRAL versus a0<<4 spelling). Three m2c targets provide independent convergence on a KILLED shape.

- [s17] [s17] Kengo exported metadata for saTan0Init is a symbol+size line only. kengo_debug_full.txt and kengo_globals_full.txt have ZERO hits. s8's Kengo-source-absent finding extended to debug-info-absent at a distinct evidence surface.

- [s17] [s17] text1a_c.c sibling functions on D_800EED10/14/18/1C (saTan5TakeGetPos_80045694, func_800456F0, func_8004574C, func_800457A0) use single-target search loops without accumulator/branch — no shape-family transplant available.

- [s17] [s17] The rederive modality is now measured dead across four independent sub-axes: (1) single-target m2c (s8), (2) cross-target m2c convergence (s17), (3) Kengo source+debug+globals (s8+s17), (4) decomp.me local corpus shingle-search (s9), (5) BB2 sibling shape (s17). Every rederive lever this modality can reach has been enumerated and killed.

- [s17] [s17] Consistent with ledger s6/s7/s15/s16 pass-level conclusions: sched1+sched2 LUID coupling to cse.c BB-scoped substitution is upstream of any C-source shape change hand-derivation or m2c can reach; only permuter-family mutations reaching a different expand/cse/global path remain sanctioned. PERM_RANDOMIZE (frontier #1) is the last unmeasured axis before OWNER-ESCALATION becomes ripe.

- [s18] [s18] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean throughout; no src edits this session.

- [s18] [s18] m2c --gotos-only with mips-gcc-c passes=5 produces a distinct CFG surface (explicit block_2 / loop_1 / block_5 / block_7 / block_8 labels + gotos) but the initial decl cluster init order is IDENTICAL to s8/s17: var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=temp_s4+arg1. CFG representation is orthogonal to decl-print order in m2c.

- [s18] [s18] m2c --target mipsel-mwcc-c produces byte-identical init cluster to gcc-c and ido-c dialects. The mwcc dialect selector adjusts prologue/epilogue matching but not the decl init printer. 4th distinct m2c dialect surface confirms init order is fixed by asm structure, not m2c dialect priors.

- [s18] [s18] m2c --target mipsee-gcc-c --no-stack-spill --deterministic-vars produces init order sum(pos 8) -> i(pos 10) -> v1(pos 11) -> s4(pos 17) -> s5(pos 22). The ASM-position-derived numeric suffixes give independent evidence that m2c's decl creation ORDER is: sum, i, v1, s4, s5. This is the i-before-v1 shape banked KILLED at s1 (H1 rejected/i-before-v1-init.c).

- [s18] [s18] The three new sub-axes converge on the SAME KILLED shape (H1 / i-before-v1) via three DIFFERENT paths: CFG representation (gotos-only), compiler dialect (mwcc), and analysis flags + position-derived variable suffixes (mipsee + no-stack-spill + deterministic-vars). Independent convergence hardens s8/s9/s17's rederive-exhaustion conclusion.

- [s18] [s18] Rederive modality is now measured dead across EIGHT independent sub-axes: (1) single-target m2c mips-gcc-c (s8), (2) cross-target m2c mipsel-gcc-c + mips-ido-c (s17), (3) Kengo source (s8) + Kengo debug + globals (s17), (4) decomp.me local corpus shingle-search (s9), (5) BB2 sibling shape (s17), (6) m2c --gotos-only (s18), (7) m2c mwcc dialect (s18), (8) m2c mipsee + no-stack-spill + deterministic-vars (s18). Every rederive lever reachable by this modality has been enumerated and converges on either the H1 shape (KILLED) or a NEUTRAL free-axis permutation.

- [s18] [s18] The rederive-modality exhaustion is a FIFTH independent argument (alongside s1-s16 hand-derivation exhaustion + s13-s14 directed permuter plateau + s15 (a)/(b) discrimination + s16 sched1+sched2 LUID-coupling + s17 five-sub-axis rederive kill) that only PERM_RANDOMIZE remains as a sanctioned axis before OWNER-ESCALATION becomes ripe.

- [s18] [s18] candidate.c unchanged; no src edits this session; no rejected forms new-banked (all three s18 outputs converge on the already-banked rejected/i-before-v1-init.c form, so no new file needed).

- [s19] [s19] Baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean; no src edits; candidate.c unchanged. Artifact: tmp/grind/func_80045294/s19/baseline_recon.txt.

- [s19] [s19] Synthesis: the residual admits ONE problem statement -- reach INSN_LUID(sll) > INSN_LUID(move16) at both sched1 and sched2 ready-list evaluation -- and four proven pass-level failure modes for any C-source lever hand-derivation can reach: (1) s7/s15 cse.c BB-scoped operand substitution shortening a0's live range and demoting global_alloc priority; (2) s6 local_alloc/global_alloc pool split with no coalescer producing +1 copy for any two-tree-a0<<4 spelling; (3) s16 sched1+sched2 both committing via same LUID mechanism (no scheduler-only flag defeats); (4) s10/s11 no pure-C CFG-splitter defeats cse's BB view (do-while(0) collapses in jump.c; stmt-expr collapses in c-parse.y; saTan0Init has no natural semantic conditional to insert). Full merged writeup: tmp/grind/func_80045294/s19/synthesis.md.

- [s19] [s19] PERM_RANDOMIZE macro clarification (tools/decomp-permuter/src/randomizer.py:151-173 + perm/parse.py:81): the macro emits `_permuter randomizer start`/`end` pragmas that bound the randomization Region for a given iter; absence of any PERM_RANDOMIZE annotation falls back to Region.unbounded() (line 172). Therefore s13's random-mode campaign on the full-TU chassis (17773 iters, plateau 60) was ALREADY EQUIVALENT to whole-function PERM_RANDOMIZE. Scoped PERM_RANDOMIZE (e.g. wrapping just the decl block) is a strict SUBSET of s13's unbounded search -- randomizer picks random.choice(regions) per iter, so narrowing the region set narrows reach, not expands it.

- [s19] [s19] The genuinely un-measured chassis-lever surfaced by comparing s13 and s14: **minimal-base chassis + random-mode (unannotated)**. s13 was full-TU chassis + random-mode; s14 was minimal-base chassis + directed (PERM_LINESWAP + PERM_GENERAL) on both full-TU and minimal-base. Minimal-base + random is a distinct chassis/mode combination not empirically banked. Reasonable expectation: also plateaus at 60 (s14 confirmed chassis-independent basin for directed macros; s13 confirmed random-mode plateau on full-TU), but this is the last sanctioned axis before every combination is measured.

- [s19] [s19] Frontier reset for s20: PRIMARY = minimal-base + random-mode permuter fresh-seed campaign (reuse s14 perm_min/ chassis, strip annotations, launch via permuter_campaign.py). SECONDARY = draft owner-escalation entry if #1 also plateaus at 60 (bank the final sanctioned axis before emitting owner-gated). DEPRIORITIZED = scoped PERM_RANDOMIZE experiments (provably a SUBSET of s13's unbounded search).

- [s19] Baseline reconfirmed: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78 (tmp/grind/func_80045294/s19/baseline_recon.txt).

- [s19] PERM_RANDOMIZE macro is a REGION scoper (start/end pragma pair); absence falls back to Region.unbounded() at randomizer.py:172. s13's random-mode (17773 iters, plateau 60) = unbounded PERM_RANDOMIZE on full-TU. Scoped PERM_RANDOMIZE is provably SUBSET, not superset.

- [s19] The residual admits ONE problem statement: reach INSN_LUID(sll) > INSN_LUID(move16) at both sched1 and sched2. Four proven pass-level walls block every C-source lever hand-derivation can reach: (1) cse.c BB-scoped substitution (s7/s15), (2) local/global_alloc pool split with no coalescer (s6), (3) sched1+sched2 both LUID-committed (s16), (4) no pure-C CFG-splitter defeats cse's BB view (s10/s11).

- [s19] 22 rejected forms in memory/grind/func_80045294/rejected/ all map to one of the four pass-level walls; no rejected form escaped classification.

- [s19] Rederive modality dead across EIGHT sub-axes (s8: m2c mips-gcc-c + Kengo source; s9: decomp.me corpus; s17: m2c mipsel-gcc-c + m2c mips-ido-c + Kengo debug/globals + BB2 sibling shapes; s18: m2c --gotos-only + m2c mwcc dialect + m2c mipsee no-stack-spill deterministic-vars).

- [s19] Directed permuter dead across 2 chassis / ~6000 iters (s13/s14: full-TU + minimal-base, PERM_LINESWAP over 6-decl cluster + PERM_GENERAL on a0<<4 / s4+a1, plateau=60 chassis-independent).

- [s19] Random permuter dead on full-TU chassis (s13: 17773 iters / 668s, plateau=60, only novel finds were equivalent-basin sum/v1 swap + broken UB false-match).

- [s19] Genuinely un-measured chassis/mode combination = minimal-base chassis + random-mode (no directed annotations). s14 minimal-base only tested directed; s13 random only tested full-TU. This is the final sanctioned axis before OWNER-ESCALATION becomes ripe.

- [s19] Merged synthesis writeup: tmp/grind/func_80045294/s19/synthesis.md.

- [s20] s20 permuter campaign at tmp/grind/func_80045294/s20/perm_min: base_score=60, 13977 iters over 531.6s, best_new_score=60, finds=[output-60-1@39s, output-60-2@433s], stopped=true, procs_killed=5.

- [s20] Minimal-base chassis reproduced from s14: 63-line base.c with narrowed extern decls (D_800EED10/14/18/1C, D_800A33A0/A4/AC), gpu_DrawSync, func_800520B8; PERM_LINESWAP wrapper removed to expose the whole function body to random-mode mutation.

- [s20] empty_prologue.json + empty_delay_slot.txt + empty_frame_fix.txt = cheat-invisible ground truth (matches sandbox --disable all gradient).

- [s20] Every empirically-testable chassis/mode combination now measured dead: full-TU directed (s13/s14 lineswap), full-TU random (s13), minimal-base directed (s14 lineswap), minimal-base random (s20). No further permuter axis exists in decomp-permuter's mutation family for this function.

- [s20] Hand-derivation enumeration exhausted across s1-s12 structural + s8/s9/s17/s18 rederive: 22 rejected forms banked (a0-alias-local, combined-decl-v1-s4, count-before-i, cse-fold-anon-shift, cse-reuse-shift, decl-init-decouple, dowhile0-around-v1-after-i, i-before-v1-init, i-before-v1-with-i-as-shift-operand, inline-count-no-local, inner-block-defer-v1, late-v1-assign-with-s4-inline, s5-inside-sum-block, shift-as-signed-mult, shift-as-unsigned-mult, shift-via-ptrdiff, split-init-s5, stmt-expr-shift, sum-init-at-end, swap-s5-operands, u32-v1-cast, v1-guard-scoped).

- [s20] Four pass-level walls named + cited across s6/s7/s10/s11/s15/s16 forensics: (1) s7 cse.c BB-scoped operand substitution collapses a0's live range; (2) s6 no cross-pool coalescer in GCC 2.7.2 local_alloc/global_alloc; (3) s16 sched1 + s15 sched2 both LUID-coupled (scheduling-only intervention insufficient); (4) s10/s11 no CFG-splitter defeats cse.c BB-scoped substitution (do-while(0) does not create a distinct BB in this pass).

- [s20] docs/grind/decisions.md contains no OWNER-ESCALATION entry for func_80045294; per contract 'owner-gated' is not emittable yet.

- [s21] [s21] Baseline reconfirmed pre and post: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78; src/text1a_c.c reverted to canonical candidate.c after the s21 measurement.

- [s21] [s21] Comma-expression form `s32 v1; s32 s4 = (v1 = a0<<4, *(s32*)((u8*)&D_800EED14 + v1));` measured: score=2, target_insns=83, build_insns=83. Objdump prologue: sll v1,s2,0x4 at 0x2630, sw s0,16(sp) at 0x2634, move s0,s2 at 0x2638 — same sll-before-move16 order as baseline.

- [s21] [s21] Mechanism: GCC 2.7.2's expand_expr on COMPOUND_EXPR calls expand_stmt for the LHS effect (v1 assignment) and returns the RHS value — the ashift RTX is NOT nested inside s4's init tree; it is emitted as its own standalone stmt at the SAME LUID position as the two-statement form `s32 v1 = a0<<4; s32 s4 = tbl_deref;`. LUID relationship to i=a0's move16 is unchanged.

- [s21] [s21] The cse-fold-anon-shift.c 'NEXT LEVER' comment (2026-07-18) specifically proposed the comma-expression candidate to eliminate the +1 CSE-move copy without pushing sll's LUID back down. s21 empirically disproves the hypothesis: the comma form collapses to the baseline shape (wrong order, no copy) rather than to a new sw/move16/sll shape without copy.

- [s21] [s21] Consequence for the search space: NO intermediate middle-path exists between baseline (sll/sw/move16, 83 insns) and cse-fold-anon-shift (sw/move16/sll+copy, 84 insns) at the C-front-end level. The two shapes are the only two attractor states hand-derivation can reach for this cluster, corresponding to the two mechanically-distinct RTL LUID relations (`ashift-before-move16` vs `ashift-inside-s4-tree`).

- [s21] [s21] Rejected form banked: memory/grind/func_80045294/rejected/comma-expr-v1-assign.c.

- [s21] [s21] Consolidated across s1-s21: EVERY sanctioned structural axis reachable by hand-derivation is measured dead. The remaining sanctioned axis at the pipeline level is OWNER-ESCALATION per the s19/s20 ledger. No entry exists yet in docs/grind/decisions.md for func_80045294; per contract 'owner-gated' is not emittable this session.

- [s22] s22 fresh-seed lineswap chassis: 720 iters / 137.5s, base=60, 1 find at score=60 (basin), best_new_score=60. Adds to s14's 5249 directed iters as third independent lineswap-chassis exhaustion sample. Cumulative directed-mode budget on this chassis: 5969 iters, 0 sub-60 finds.

- [s22] s22 reclaimed alt-chassis (cse-fold-anon-shift): 1310 iters / 442.3s, base=105, 0 finds. Newly-banked measurement — the +1-copy near-hit basin is empirically terminal under random-mode permuter, corroborating s6's mechanism verdict at a distinct chassis.

- [s22] Ledger state entering s22 already recorded: hand-derivation exhausted (23 rejected forms across s1-s21, all mapping to one of four pass-level walls: cse.c BB-scoped operand substitution / local_alloc-global_alloc pool split / sched1+sched2 LUID coupling / no pure-C CFG-splitter defeats cse.c BB view); rederive exhausted across 8 sub-axes (s8/s9/s17/s18); directed permuter exhausted on 2 chassis (s13/s14); random permuter exhausted on 2 chassis (s13 full-TU / s20 minimal). s22 adds a 3rd chassis (cse-fold-anon-shift) and a 3rd sample on the s14 chassis.

- [s22] src/text1a_c.c:1602-1608 restored to baseline candidate.c shape (v1-before-i init order). Sandbox --disable all: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78.

- [s22] cpu_side_move_dir_4 precedent (docs/grind/decisions.md 2026-07-09 01:25 — FAIL): the exact same LUID tiebreak mechanism was ruled forbidden under no-new-park-categories.md 'Register-rotation infrastructure' as the first archetypal example — 'these are pure-C-reachable, the lever just hasn't been found ... the remaining gap is more search, not a new category.' This binds any OWNER-ESCALATION for func_80045294 with the same framing to a very likely FAIL, but the ledger's per-function measured-dead frontier remains the correct evidence path to file the request.

- [s23] [s23] Baseline reconfirmed post-campaign: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean throughout (only tmp/ scratch + memory/grind/ ledger touches); no src edits this session.

- [s23] [s23] Third distinct permuter chassis basin banked: H1-shape (i-before-v1) minimal-base + random-mode reports base_score=84 in the permuter's weighted scorer. Prior banks: baseline chassis (base=60) at s13/s14/s20/s22; cse-fold-anon-shift chassis (base=105) at s22. Three chassis, three basins, all measured terminal for sub-60 finds.

- [s23] [s23] s23 H1 chassis campaign: 37313 iters / ~26 min, 7 new finds at scores {79,79,60,61,60,69,60}. best_new_score=60. Zero sub-60 finds. Fresh-seed rule satisfied: >>20 min since last novel-score find (last output-60-3 at 956.9s, still above 60).

- [s23] [s23] Cumulative permuter budget across all chassis/mode combinations for this function is now ~69590 iters (s13 5249+17773 + s14 720+720 + s20 13977 + s22 720+1310 + s23 37313) with zero sub-60 finds across the entire budget. Chassis-independent basin claim (s14/s20) hardened by a third seed shape at a distinct base score.

- [s23] [s23] The frontier's specific hypothesis ('m2c-reconstructed H1-shape base with random-mode plausibly-KILLED by s6 mechanism but not yet empirically banked') is now empirically banked. s6's local_alloc/global_alloc pool split with no coalescer predicts every mutation from the H1 basin that decouples ashift into an anonymous pseudo (moving toward cse-fold-anon-shift shape) pays the +1 copy penalty (basin=105+), while every mutation that preserves the H1 direct-shift shape retains the RA rotation penalty. The 60 attractor is reached only by mutations that collapse H1 back to the baseline v1-before-i shape, which cannot go below 60 by construction. The banking matches the mechanism.

- [s23] [s23] Consequence: every empirically-testable permuter chassis/mode combination is now measured dead — full-TU directed (s13), full-TU random (s13), minimal-base directed (s14), minimal-base random (s20), fresh-seed lineswap (s22), cse-fold-anon-shift chassis (s22), and H1-chassis random (s23). Seven independent samples on the same 60 attractor; two independent samples on higher basins (84, 105) collapsing at best to 60. No further permuter axis exists in decomp-permuter's mutation family for this function.

- [s23] [s23] Combined ledger: hand-derivation exhausted (23 rejected forms mapping to four pass-level walls: s7 cse.c BB-scoped substitution / s6 local/global_alloc pool split no-coalescer / s16 sched1+sched2 both LUID-committed / s10-s11 no pure-C CFG-splitter defeats cse.c BB view); rederive exhausted (8 sub-axes at s8/s9/s17/s18); permuter exhausted (7 chassis/mode combinations, ~69590 iters). Every sanctioned axis at the pipeline level is measured dead.

- [s23] [s23] docs/grind/decisions.md has NO OWNER-ESCALATION entry for func_80045294 as of this session (grep confirms zero hits). Per contract, 'owner-gated' is not emittable this session; the required draft entry is next session's mandated modality per the s22 frontier ('owner-escalation-drafting is the only sanctioned remaining move').

- [s23] [s23] Artifacts: tmp/grind/func_80045294/s23/perm_h1/{base.c, compile.sh, settings.toml, empty_*, target.o, campaign.log, campaign_meta.json, output-{60-1,60-2,60-3,61-1,69-1,79-1,79-2}/}. Summary at tmp/grind/func_80045294/s23/campaign_summary.md.

- [s23] [s23] candidate.c unchanged (in-src form remains the s3 baseline shape; H1 was tested as a permuter chassis seed, not as an in-src edit). No new rejected forms banked (the H1 body is already banked at rejected/i-before-v1-init.c since s1; the permuter finds at scores 79/69/61/60 are equivalent-basin variants of already-banked shapes, per the s13 novel-find analysis pattern).

- [s24] [s24] Baseline reconfirmed: & tools/wteng.ps1 main sandbox func_80045294 --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean; no src edits this session.

- [s24] [s24] Novel forensic surface banked (new to the s1-s23 ledger): local-alloc.c pass output (base.i.lreg) directly shows the two-pool split. Baseline (v1-before-i) has no block-0-scoped anon pseudo for the shift — pseudo 75 (v1) is multi-block (11 uses / 17 insns) and goes to global_alloc; no local claim exists for the shift's destination.

- [s24] [s24] cse-fold-anon-shift.c lreg shows pseudo 76 tagged '2 uses / 2 insns in block 0' → local_alloc emits `;; Register 76 in 2.` (hard reg $v0). Pseudo 81 (declared v1) is un-tagged, 11 uses / 16 insns, goes to global_alloc which picks $v1 ($3). Two distinct hard regs across the pool boundary → (set 81 76) at insn 29 cannot fold to a self-move; the +1-copy penalty s6 named is generated exactly at this pass boundary.

- [s24] [s24] Named GCC pass and decision at the divergence for the two-tree branch: flow.c live-range analysis classifies pseudo 76 as block-0-local; local-alloc.c claims a hard reg for it before global-alloc.c runs; global-alloc.c has no coalescer to merge across the pool boundary. The absent GCC 2.7.2 pass is a register coalescer (first added in GCC 3.x).

- [s24] [s24] All four pass-level walls now have direct pass-output citations: (i) s6 local/global pool split via local-alloc.c — NOW DIRECTLY BANKED at lreg; (ii) s7 cse.c BB-scoped substitution — banked at s7 base.i.cse insn 17 operand rewrite; (iii) s15/s16 sched1+sched2 LUID coupling — banked at base.i.sched + base.i.sched2 ready-list dumps; (iv) s10/s11 no pure-C CFG-splitter defeats cse's BB view — banked at score measurements + pass-source cite.

- [s24] [s24] docs/grind/decisions.md has NO OWNER-ESCALATION entry for func_80045294 (grep confirms zero hits at 811-line file). Per contract, `owner-gated` is not emittable this session; the required draft entry is next session's mandated modality per the s22/s23 frontier and the ledger consensus.

- [s24] [s24] candidate.c unchanged (in-src form remains the s3 baseline shape). No new rejected forms banked this session (no new src edits measured; the two lreg slices come from prior s3/s6 dump artifacts).

- [s24] [s24] Artifacts: tmp/grind/func_80045294/s24/forensics_lreg_pool_split.md + baseline_lreg_slice.txt + csefold_lreg_slice.txt.

- [s25] [s25] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c edits reverted to canonical candidate.c shape after dump; working tree clean (only metrics/events.jsonl dirty as expected).

- [s25] [s25] With do-while(0)-around-v1-after-i applied to src, sandbox --disable all -> score=12 (reconfirms s10 KILLED measurement at same score; s10 recorded score=12 as well).

- [s25] [s25] Pre-jump.c CFG structure (base.i.rtl:14365-14396): do-while(0) wrap generates code_label 18 (loop head), note 19, insn 20 ashift, note 21 LOOP_CONT, code_label 22, jump_insn 23 unconditional to label 29, barrier 24, code_label 25, jump_insn 26 unconditional to label 18 (dead back-edge), barrier 27, note 28 LOOP_END, code_label 29 (loop exit). Real CFG structure present.

- [s25] [s25] Post-jump.c (base.i.jump:13599-13612): all 4 code_labels, 2 jump_insns, 2 barriers DELETED. Only the 3 LOOP notes (17 LOOP_BEG, 21 LOOP_CONT, 28 LOOP_END) plus insn 20 ashift survive. jump.c's jump_optimize collapsed the trivially-nested exit + back-edge into straight-line insn stream. MECHANISM (a) DIRECTLY WITNESSED.

- [s25] [s25] cse.c basic-block partitioner (base.i.cse:12612 header): 'Processing block from 2 to 28, 5 sets.' — cse.c spans the 3 remaining LOOP notes (17, 21, 28) as ONE block, treating them as BB-transparent. Even if mechanism (a) had NOT fired, mechanism (b) would still collapse the do-while(0) into cse.c's value-numbering scope. MECHANISM (b) DIRECTLY WITNESSED.

- [s25] [s25] Pre-cse.c ashift operand (base.i.jump:13603): (ashift (reg/v:SI 72) (const_int 4)) — reg 72 is a0. Post-cse.c ashift operand (base.i.cse:12649-12651): (ashift (reg/v:SI 75) (const_int 4)) — reg 75 is i. cse.c substituted 75 for 72 in the ashift because insn 15 (set 75 72) is in the same block. Identical fingerprint to s7's insn 17 substitution on canonical H1 shape; the score=12 is H1 basin (11) plus one insn from LOOP-note overhead.

- [s25] [s25] Wall (iv) UPGRADED to direct pass-output tier. All four pass-level walls now cited at direct dump output: (i) s24 base.i.lreg — local/global pool split; (ii) s7 base.i.cse — cse.c BB-scoped substitution; (iii) s15+s16 base.i.sched+sched2 — sched1+sched2 LUID coupling; (iv) s25 base.i.jump+base.i.cse — jump.c CFG-collapse + cse.c LOOP-note-spanning, TWO STACKED walls (both fire independently).

- [s25] [s25] Consequence for the OWNER-ESCALATION draft: mechanism (a) and (b) from the s10 evidence-bank entry (two competing hypothesis explanations for the do-while(0) kill) can be re-stated as TWO CONFIRMED STACKED WALLS. Any future proposed CFG-splitter (whether the sanctioned do-while(0), a hypothetical semantic conditional insert, or another construct) must EACH survive both jump.c's optimizer AND cse.c's LOOP-note-transparent block partitioner. This tightens the escalation ask.

- [s25] [s25] docs/grind/decisions.md still contains NO OWNER-ESCALATION entry for func_80045294 (grep confirms zero hits). Per contract, 'owner-gated' is not emittable this session; the required draft entry is next session's mandated modality per the s19-s24 ledger consensus. This s25 forensics contribution supplies the last pass-output citation the drafter will cite for wall (iv).

- [s25] [s25] No new rejected forms banked (dowhile0-around-v1-after-i.c already banked at s10; the s25 measurement reproduces s10's score=12 kill and adds pass-output evidence). candidate.c unchanged (in-src form remains s3 baseline).

- [s26] [s26] Baseline reconfirmed pre-probes: sandbox --disable all -> score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78.

- [s26] [s26] Working tree clean; no src edits this session; candidate.c unchanged (baseline v1-before-i form preserved).

- [s26] [s26] m2c --stack-structs + --descending-regs (novel combination): produces H1 shape (i-before-v1) BYTE-IDENTICAL to s8/s17/s18 init cluster. Artifact: tmp/grind/func_80045294/s26/m2c_stackstructs_descregs.txt.

- [s26] [s26] m2c --context (BB2 SaTanEntry struct + D_800EED10[] array + real prototypes) + --globals all (novel combination): produces H1 shape with chan/delta arg-names from context but IDENTICAL init order and offset arithmetic (raw &D_800EED14 base+offset, no subscript rewrite). Artifact: tmp/grind/func_80045294/s26/m2c_context_globals_all.txt.

- [s26] [s26] Rederive modality now measured dead across TEN independent sub-axes: (1) m2c mips-gcc-c s8, (2) m2c mipsel-gcc-c s17, (3) m2c mips-ido-c s17, (4) Kengo source/debug/globals s8+s17, (5) decomp.me local corpus shingle-search s9, (6) BB2 sibling shape s17, (7) m2c --gotos-only s18, (8) m2c mwcc s18, (9) m2c mipsee no-stack-spill deterministic-vars s18, (10) m2c --stack-structs+--descending-regs AND m2c --context+--globals all s26. Every configuration converges on either H1 (KILLED at s1 via cse.c substitution) or a NEUTRAL free-axis permutation.

- [s26] [s26] The i-before-v1 shape is fixed by the target's asm insn ordering (move-before-sll at insn positions 8-10) and m2c's SSA-to-decl printer against it. No m2c flag can invert this without semantic change to the target asm.

- [s26] [s26] Every previously-cited pass-level wall (s6 pool-split, s7/s15 cse.c BB-scoped substitution, s16 sched1+sched2 LUID coupling, s10/s11/s25 no pure-C CFG-splitter) unchanged and load-bearing.

- [s26] [s26] docs/grind/decisions.md STILL has no entry for func_80045294 (grep -n '80045294\|saTan0Init' docs/grind/decisions.md returns nothing); owner-gated remains unemittable per contract. The owner-escalation-drafting frontier from s19-s25 remains the sole sanctioned move.

- [s26] [s26] No rejected forms new-banked (both s26 probes converge on already-banked rejected/i-before-v1-init.c shape); no new file needed.

- [s27] Kengo saTan0Init disassembly (tmp/grind/func_80045294/s27/kengo_saTan0Init.s) shows 9 external jal calls vs BB2 func_80045294's 2, 160-byte vs ~32-byte stack frame, tslGlobalMemAlloc allocation pipeline absent from BB2, six-iteration fixed loop vs D_800A33AC dynamic loop.

- [s27] s8's Kengo-transplant conclusion (block was 'no source shipped') is now upgraded to a stronger evidence tier: even with the asm inspectable, the semantic mismatch means transplant was never a live option regardless of source availability.

- [s27] s17's kengo_debug_full.txt / kengo_globals_full.txt emptiness for saTan0Init is corroborated: no debug info exists because the function's structure is unrelated to BB2's — Kengo emits richer info for functions the compiler considered nontrivial, and this one's simplicity in Kengo's compilation offered nothing informative for BB2.

- [s27] Total rederive sub-axes measured dead now: 11 (s8:2, s9:1, s17:3, s18:3, s26:3, s27:1). Rederive modality remains terminally exhausted per the s19/s26 consolidated frontier.

- [s27] candidate.c (v1-before-i baseline) sustains sandbox floor=2 unchanged; no edit applied to src/text1a_c.c this session (rederive session, no LUID/lever measurement).

- [s28] [s28] Baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean; no src edits; candidate.c unchanged. Artifact: tmp/grind/func_80045294/s28/baseline_recon.txt.

- [s28] [s28] Four pass-level walls consolidated with direct-pass-output citations: (i) s7/s15 cse.c BB-scoped operand substitution [base.i.cse insn 17]; (ii) s6/s24 local_alloc/global_alloc pool split no coalescer [base.i.lreg claim lines]; (iii) s16 sched1+sched2 both LUID-committed [base.i.sched + base.i.sched2 ready-list]; (iv) s10/s11/s25 no pure-C CFG-splitter defeats cse.c BB view [base.i.jump + base.i.cse block partitioner header]. All four elevated to direct-dump tier.

- [s28] [s28] 23 rejected forms in memory/grind/func_80045294/rejected/ enumerated against the four walls; every form maps to exactly one wall or is a NEUTRAL free-axis probe. Rejected bank closed against reachable pure-C search space for this shape.

- [s28] [s28] Permuter budget totalled across all sessions: ~69590 iters over 7 chassis/mode combinations (s13 full-TU directed 5249 + full-TU random 17773; s14 min directed 720+4320; s20 min random 13977; s22 fresh lineswap 720 + alt-chassis reclaim 1310; s23 H1-chassis random 37313). Three basin base scores (60/84/105) all measured terminal for sub-base finds. No sub-60 finds anywhere in the budget.

- [s28] [s28] Rederive modality dead across 11 sub-axes (s8: m2c mips-gcc-c + Kengo source absent; s9: decomp.me corpus 0.127 max similarity; s17: mipsel-gcc-c + mips-ido-c + Kengo debug/globals empty + BB2 sibling non-analogue; s18: --gotos-only + mwcc + mipsee no-stack-spill deterministic-vars; s26: --stack-structs+--descending-regs + --context+--globals all; s27: Kengo saTan0Init asm-semantic-diff proves same-name-transplant premise dead). All configurations converge on H1 (KILLED at s1) or a NEUTRAL free-axis permutation.

- [s28] [s28] docs/grind/decisions.md has NO OWNER-ESCALATION entry for func_80045294 as of this session (grep -n '80045294\|saTan0Init' returns zero hits). Per contract, 'owner-gated' is not emittable this session; the owner-escalation-drafting modality is the required next-session move.

- [s28] [s28] cpu_side_move_dir_4 precedent (docs/grind/decisions.md 2026-07-09 01:25 FAIL) binds any func_80045294 escalation framed as 'register rotation infrastructure' to a likely FAIL under no-new-park-categories.md. Escalation framing must either (a) argue the four stacked pass-level walls constitute a distinct category, or (b) explicitly request practical-limit closure at floor=2 with the four walls as pass-mechanical proof of unreachability.

- [s28] [s28] Synthesis writeup consolidating four walls, rejected-form bank, modality exhaustion table, and cpu_side_move_dir_4 framing distinction: tmp/grind/func_80045294/s28/synthesis.md.

- [s29] s29 baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c reverted to canonical candidate.c shape after probes; working tree carries only tmp/ scratch + memory/grind/ ledger touches.

- [s29] s29 refines s12's 'sum decl position 0 LOAD-BEARING' claim: sum's position is a bounded free-axis. Positions {0, 1, 2} (LUID before i=a0's assignment) give score=2; positions {3, 4, 5} (LUID after i=a0) give score=4 with a 2-diff RA cascade.

- [s29] The load-bearing boundary is i=a0's assignment, not sum's absolute position. Same LUID/RA coupling documented by walls (i) s7/s15 cse.c BB-scoped substitution and (ii) s6/s24 local_alloc/global_alloc pool split with no coalescer -- no new mechanism, no new lever.

- [s29] s29 broadens the enumeration surface: the s28 category (A) statement position exhaustion tally now includes 3 new sub-positions (sum at 1, 2, 3) beyond the s1/s3/s11/s12-banked positions. All map to the same two mechanism attractors (score=2 free-axis or score=4 post-i cascade).

- [s29] New rejected form banked: memory/grind/func_80045294/rejected/sum-after-i.c (representing the post-i cascade at positions 3/4/5).

- [s29] No structural axis reachable from this refinement decouples the four pass-level walls. s28 modality exhaustion conclusion holds; s29 only tightens the description of the sum-position sub-axis.

- [s29] docs/grind/decisions.md STILL contains no OWNER-ESCALATION entry for func_80045294 (checked via grep -n 'func_80045294' docs/grind/decisions.md; zero hits). Per contract, 'owner-gated' is not emittable this session.

- [s29] candidate.c unchanged (in-src form remains the s3 baseline v1-before-i shape). src/text1a_c.c line 1602-1608 restored to canonical baseline post-probes.

- [s30] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83. src/text1a_c.c reverted to canonical candidate shape after probes; working tree carries only tmp/ scratch + memory/grind/ ledger touches.

- [s30] Two-axis simultaneous permutation probe A (sum@2 + s5/count swap, decl order: v1,s4,sum,i,s5,count) measured NEUTRAL (score=2). Independent variables from the {sum-position} sub-axis (s12/s29) and {count/s5 free-axis cluster} (s1/s3/s11) compose linearly at the LUID/RA surface.

- [s30] Two-axis simultaneous permutation probe B (sum@1 + count-before-i, decl order: v1,sum,s4,count,i,s5) measured NEUTRAL (score=2). count's move from position 4 to position 3 (before i) combines cleanly with sum's move from 0 to 1 without perturbing the sll/move16 LUID relation.

- [s30] Three-axis simultaneous permutation probe C (sum@2 + s5-before-i + count-last, decl order: v1,s4,sum,s5,i,count) measured NEUTRAL (score=2). Cross-product of three free-axis moves confirms superposition of independent free-axes at this decl cluster: no combination lifts the score above 2 nor drops it below.

- [s30] Formal closure of the s29 frontier's "two-axis / cross-product" surface: all measured combinations in the free-axis cluster {sum in {0,1,2}, count/s5/i mutual permutations with v1-before-i preserved} compose to the baseline attractor. The pass-level walls (s7 cse.c BB-scoped substitution, s6/s24 pool split no-coalescer, s16 sched1+sched2 LUID coupling, s10/s11/s25 no pure-C CFG-splitter) are robust to multi-axis combinations, not just single-axis moves. No new mechanism, no new lever.

- [s30] docs/grind/decisions.md STILL contains no OWNER-ESCALATION entry for func_80045294 (grep -n 'func_80045294' docs/grind/decisions.md returns zero hits). Per contract, 'owner-gated' is not emittable this session; owner-escalation-drafting modality remains the mandated next-session move per s19-s29 ledger consensus.

- [s30] No new rejected forms banked (all three probes converge back to the baseline score=2 attractor; no divergent form to save). candidate.c unchanged.

- [s30] Artifacts: tmp/grind/func_80045294/s30/{baseline.txt, probe_A_sum2_s5count_swap.txt, probe_B_sum1_countbeforei.txt, probe_C_sum2_s5_countlast_ilast.txt, baseline_post.txt}.

- [s30] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree carries only tmp/ scratch + memory/grind/ ledger touches at session end.

- [s30] Probe A (sum@2 + s5/count-swap; v1,s4,sum,i,s5,count) -> score=2 NEUTRAL.

- [s30] Probe B (sum@1 + count-before-i; v1,sum,s4,count,i,s5) -> score=2 NEUTRAL.

- [s30] Probe C (sum@2 + s5-before-i + count-last; v1,s4,sum,s5,i,count) -> score=2 NEUTRAL.

- [s30] Free-axis independence at this decl cluster is now confirmed empirically at the two-axis and three-axis surface, not just the single-axis surface s1/s3/s11/s12/s29 previously banked. The s29 frontier's 'two-simultaneous permutations' probe is discharged.

- [s30] The four pass-level walls (s7 cse.c BB-scoped operand substitution / s6/s24 local_alloc/global_alloc pool split with no coalescer / s16 sched1+sched2 both LUID-committed / s10/s11/s25 no pure-C CFG-splitter defeats cse's BB view) are robust to multi-axis structural combinations. The mechanism is: single-axis free-axis moves are LUID-neutral at both sched1 and sched2 (they preserve the ashift's LUID relation to the move16 insn), so composing them preserves neutrality — no combinatorial cascade emerges.

- [s30] docs/grind/decisions.md STILL has no OWNER-ESCALATION entry for func_80045294 (grep -n 'func_80045294' returns zero hits). Per contract, owner-gated is not emittable this session.

- [s31] s31 fresh-seed campaign: 21218 iters in 779.6s, 10 finds banked. Best score=10 (output-10-1 at 336.4s from launch). Other novel finds: score=66, 70, 79-2 (not present in s23's find set). Campaign log at tmp/grind/func_80045294/s31/perm_h1_freshseed/campaign.log; find bundle at output-*/.

- [s31] output-10-1 diff (tmp/grind/func_80045294/s31/perm_h1_freshseed/output-10-1/diff.txt): 3 changes — (1) dead 's32 *new_var;' decl added, (2) 'i = a0;' inserted mid-loop before 'v1 += 0x10; i += 1;', (3) 'ptr' initialization split into 'new_var = ...; ptr = new_var;' alias in second loop.

- [s31] Semantic analysis: 'i = a0;' inside the loop makes post-body i always equal a0+1. First-loop terminator 'i < count' then compares a0+1 vs count — for a0+1 < count the loop never terminates. Target's honest semantics require i to advance from a0 up to count. The permuter's score-only scoring does not detect this — score=10 is a codegen-similarity measure, not a semantic-correctness measure.

- [s31] This is the FIRST sub-60 permuter find across 5 random-mode campaigns totaling ~90800 iters (s13 17773 + s14 720 + s20 13977 + s23 37313 + s31 21218). All prior sub-60 attempts across 7 chassis plateaued at 60 or above. The sub-60 basin exists but is a semantic-break basin.

- [s31] Consequence: the s28-cited four pass-level walls (cse.c BB-scoped operand substitution, local-alloc/global-alloc pool split with no cross-pool coalescer, sched1+sched2 LUID coupling, no pure-C CFG-splitter defeats cse.c BB view) are re-confirmed at a fresh angle — the LUID-coupling shift that would lower score requires an intervention pure C cannot legitimately emit (either semantic break, or canonical-asm cheat class).

- [s31] s31 candidate.c unchanged from s30 (H1 baseline; not applied to src/ since sandbox floor=2 remains). One rejected form banked: rejected/perm-score10-mid-loop-i-reset-semantic-break.c documents the semantic-break basin for future sessions.

- [s31] OWNER-ESCALATION status: entry NOT yet filed in docs/grind/decisions.md. owner-gated remains inadmissible per driver contract; s31 emits 'progress' with the fifth-angle wall reconfirmation as its banked finding. Owner-escalation-drafting modality is the sole remaining sanctioned move; s31 does not draft it because the mandated modality this session was permuter.

- [s32] s32 baseline-chassis fresh-seed campaign (tmp/grind/func_80045294/s32/perm_baseline_fresh): iters=39420, elapsed=947.4s, base=60, 2 finds at score=60, no sub-60. Plateau reconfirmed.

- [s32] s32 orphan sum-at-2 chassis (tmp/grind/func_80045294/s32/perm_sum_at_end): iters=772, elapsed=1191.7s, base=60, 1 find at score=60, no sub-60. Structurally distinct decl order also plateaus at 60.

- [s32] Cumulative permuter budget for func_80045294 now ~137,752 iters across 9 chassis/mode combinations, all plateau=60 except cse-fold-anon-shift (105) and the s31 semantic-break find (10).

- [s32] src/text1a_c.c func_80045294 body unchanged from banked candidate.c (baseline v1-before-i shape); sandbox floor=2 reconfirmed by no-edit invariant.

- [s32] docs/grind/decisions.md contains ZERO hits for '80045294' or 'saTan0Init' -- OWNER-ESCALATION entry not yet filed; owner-gated not emittable this session per contract.

- [s33] Sandbox baseline reconfirmed s33: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 (file-wide).

- [s33] combine.c pass output (tmp/grind/func_80045294/s3/base.i.combine line 12633) shows func_80045294 combiner statistics: 56 attempts, 45 substitutions (23 requiring new space), 2 successes. Insn 4 (set 72 a0), insn 14 (set 75 (ashift 72 4)), insn 22 (set 78 72), insn 89 (set 78 72) all surviving as independent SETs.

- [s33] Pseudo 72 (a0) at flow.c pass output (tmp/grind/func_80045294/s3/base.i.flow line 632): '4 times across 5 insns in block 0; pointer' — pseudo 72 is block-0-scoped at flow.c's REG_BASIC_BLOCK tagging, which is what feeds local_alloc's pool split decision (s6/s24) but is upstream of combine.c's substitution decision.

- [s33] Combine slice (baseline) at tmp/grind/func_80045294/s33/combine_baseline_slice.txt (178 lines); flow slice at tmp/grind/func_80045294/s33/flow_baseline_pseudo_refs.txt (29 lines).

- [s33] s16 sched1+sched2 LUID coupling wall is now traced one pass further upstream: sched1's ready-list input inherits LUIDs 14 and 22 from combine.c's output; combine.c is where the 8-delta between sll and move16 is minted. No prior pass (cse.c, loop.c, jump.c) can shrink the delta because combine's substitute-and-delete is what would collapse either insn, and both insns' destinations are loop-carried multi-use pseudos.

- [s33] 5 forensic sessions (s3, s6, s7, s15, s16, s24, s25, s33) now have direct base.i.<pass> pass-output citations. Chain: flow.c REG_BASIC_BLOCK tag → combine.c 56/45/2 substitute-and-delete leaves both insns → cse.c BB-scoped operand substitution (H1 rotate) → local_alloc/global_alloc pool split (cse-fold +1-copy) → sched1+sched2 rank_for_schedule LUID tiebreak → reorg.c delay-slot fill (unchanged).

- [s34] Sandbox baseline reconfirmed: score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78 (file-wide). No src edits.

- [s34] EXPAND-pass forensics upstream of s33's combine.c wall: base.i.rtl lines 14343-14378 show emit_insn emits — insn 4 (set 72 a0) arg-save; insn 6 (set 73 a1); insn 12 (set 74 0) sum; insn 14 (set 75 (ashift 72 4)) SLL; **insn 17 (set 77 (symbol_ref D_800EED14)) symbol_ref temp**; **insn 19 (set 76 (mem (plus 75 D_800EED14))) s4 load**; insn 22 (set 78 72) MOVE16. The 8-UID delta between sll (14) and move16 (22) is minted by EXPAND: it counts insns 17 and 19 belonging to s4's initializer expansion.

- [s34] base.i.combine lines 12656-12673 show combine SUCCESSFULLY deletes insn 17 (symbol_ref temp) via substitute-and-delete: pseudo 77 is single-use, combine folds symbol_ref into insn 19's mem-plus addressing. Insn 19 SURVIVES (LOG_LINK `insn_list 14` explicit) because pseudo 76 (s4) is used downstream as call-arg to func_800520B8 at insn 82. INSN_UIDs are permanent — never recycled — so 14 and 22 keep their permanent numbers regardless of interposed deletions.

- [s34] Sixth pass-level wall named: insn 19's placement between insns 14 and 22 is structurally forced by s4 = *(&D_800EED14 + v1)'s data dependence on v1. Cannot move insn 19 before insn 14 (v1 = pseudo 75 must be SET first). Cannot move insn 19 after insn 22 without adopting the H1 shape (i-before-v1), which is KILLED via cse.c BB-scoped substitution (s7/s15) causing RA rotation. Cannot delete insn 19 (s4 is a live call-arg). The between-insn count `SLL → ... → MOVE16` is structurally ≥ 1 at expand for every v1-before-i candidate.

- [s34] Upstream chain now: expand emit_insn (s34 mints insn order — s4's load is the load-bearing survivor between SLL and MOVE16) → jump.c pass1 (s25) → cse.c BB-scope (s7/s15/s25) → loop.c (unchanged) → cse2.c (unchanged) → flow.c REG_BASIC_BLOCK (s33) → combine.c 56/45/2 (s33) → local/global_alloc pool split (s6/s24) → sched1/sched2 rank_for_schedule LUID tiebreak (s3/s15/s16) → reorg.c delay-slot fill (unchanged). Six pass-output-cited walls now, from expand through reorg.

- [s34] s33's "combine.c mints the 8-delta" is refined: combine.c is where the delta becomes IRREDUCIBLE (combine can't collapse either endpoint), but the delta itself is minted at EXPAND by s4's initializer producing two intermediate SETs, of which the second (insn 19, the s4 load) survives combine because s4 is downstream-live. Refinement adds precision without changing the escalation posture.

- [s34] docs/grind/decisions.md STILL has no OWNER-ESCALATION entry for func_80045294 (checked: grep -n 'func_80045294' docs/grind/decisions.md → zero hits). Per contract, 'owner-gated' is not emittable this session; owner-escalation-drafting remains the sole sanctioned next move.

- [s34] candidate.c unchanged (baseline v1-before-i shape preserved). No new rejected form banked (this session added forensics evidence, not a new C form to test).

- [s34] Artifact: tmp/grind/func_80045294/s34/expand_luid_origin.md (writeup); cites existing s3/base.i.rtl (lines 14343-14378) + s3/base.i.combine (lines 12656-12673).

- [s34] Sandbox baseline reconfirmed: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 (file-wide). No src edits this session.

- [s34] base.i.rtl 14343-14378 (expand output): insns 4,6,12 → insn 14 (sll = ashift 72 4 into pseudo 75) → insn 17 (symbol_ref D_800EED14 into pseudo 77) → insn 19 (mem-plus load of s4 into pseudo 76, reading pseudo 75) → insn 22 (move16 = i = pseudo 72 → pseudo 78) → 25/28. UID delta 8 across insns 14→22 = insns 17+19 (both belonging to s4's initializer).

- [s34] base.i.combine 12656-12673 (post-combine baseline): insn 17 DELETED via substitute-and-delete (symbol_ref folded into insn 19's mem-plus addressing). Insn 19 SURVIVES with LOG_LINK `insn_list 14`. Insns 14 and 22 both survive as independent standalone SETs (s33 finding, reconfirmed).

- [s34] INSN_UIDs are permanent identifiers assigned by emit_insn during expand; combine.c never renumbers surviving insns. This is why the LUID relation sched2 tiebreaks on (INSN_LUID(sll) < INSN_LUID(move16)) is fixed at expand, and combine can only either DELETE endpoints (which it doesn't) or preserve the relation (which it does).

- [s34] The load-bearing survivor between insns 14 and 22 is insn 19 (s4 load). Its placement is structurally forced: s4's address `(plus v1 D_800EED14)` reads pseudo 75, so insn 19 must emit AFTER insn 14. Relocating it after insn 22 requires the H1 shape (i-before-v1), which is banked KILLED at cse.c BB-scoped substitution wall (s7/s15). Deleting it is impossible (s4 is call-arg-live).

- [s34] Sixth pass-level wall named at EXPAND, upstream of the five ledger walls (cse.c BB substitution s7/s15/s25; local/global_alloc pool split s6/s24; sched1+sched2 LUID coupling s16; no pure-C CFG-splitter defeats cse s10/s11/s25; combine.c substitute-and-delete refusal s33). Full chain now spans EXPAND → jump → cse → loop → cse2 → flow → combine → local/global_alloc → sched1/sched2 → reorg, with six of ten passes cited from direct base.i.<pass> dumps.

- [s34] docs/grind/decisions.md STILL has no OWNER-ESCALATION entry for func_80045294 (grep 'func_80045294' → 0 hits). Per contract, 'owner-gated' is not emittable this session; owner-escalation-drafting is the sole remaining sanctioned move (s19-s33 consensus).

- [s35] [s35] Baseline reconfirmed pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c unchanged; working tree clean (only metrics/events.jsonl modified, engine capture).

- [s35] [s35] Novel rederive sub-axis: block-0-isolated fingerprint (23-line prologue+block-0 slice) shingle-searched (k=3, top=8) across all three corpora. Max similarity 0.122 (gcc272psx 6LWWm), below the 0.15 relevance threshold established at s9.

- [s35] [s35] Cross-corpus zero-score matched scratches at reduced similarity (hvTSS/Z60NJ/KrpLx/AFJQk/sdH51/DTKlm/wnwX1 all sub-0.12): none carry a transplantable form of the sll/move16/sw prologue cluster. Their matched C shapes solve different block-0 structures.

- [s35] [s35] Direct comparison to s9: whole-function shingle top=0.127, block-0-only shingle top=0.122. Narrowing the input REDUCES overlap because most PSX-GCC-2.7.2 scratches don't share the 4-callee-save preserve + a0<<4 shift-index derivation prologue. This function's block-0 fingerprint appears unique in the local corpus.

- [s35] [s35] Rederive modality dead across a 12th independent sub-axis (adds to the 11 banked at s26): (1) single-target m2c mips-gcc-c (s8), (2) cross-target m2c mipsel/ido (s17), (3) Kengo source/debug/globals (s8/s17), (4) whole-function decomp.me shingle (s9), (5) BB2 sibling shape (s17), (6) m2c --gotos-only (s18), (7) m2c mwcc dialect (s18), (8) m2c mipsee no-stack-spill (s18), (9) m2c stack-structs+desc-regs (s26), (10) m2c context-populated+globals-all (s26), (11) Kengo asm-level semantic comparison (s27, different function proved), (12) block-0-isolated corpus shingle (s35).

- [s35] [s35] Consistent with the six pass-level walls named across s6/s7/s10/s11/s15/s16/s33/s34: expand emit_insn structural forcing of insn 19 (s34) -> combine 56/45/2 substitute-and-delete refusal (s33) -> cse.c BB substitution (s7) -> local+global_alloc pool split with no coalescer (s6) -> sched1+sched2 LUID coupling (s15/s16) -> no pure-C CFG-splitter (s10/s11). No rederive-reachable C shape defeats this chain.

- [s36] [s36] Baseline reconfirmed pre and no src edits attempted: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree carries only metrics/events.jsonl (engine capture) and tmp/ scratch + memory/grind/ ledger touches. candidate.c unchanged (baseline v1-before-i shape preserved).

- [s36] [s36] Novel rederive sub-axis A (tail-block-isolated fingerprint, s36/tail_block.s = 39-line slice from .L80045308 through .L800453A0 pre-epilogue): shingle-search k=3 max = 0.156 (gcc272cdk DqBYX), all other hits sub-0.14. DqBYX inspected: 22-line fn-ptr dispatch wrapper without accumulator loop or gpu_DrawSync — spurious skeleton match on the `lh ; jalr` shape only. At k=4 the DqBYX signal collapses to 0.056, confirming the 0.156 was a sparse-shingle artifact on that dispatch pattern rather than structural similarity.

- [s36] [s36] Novel rederive sub-axis B (whole-function k=5 stricter shingle): max similarity 0.095 across all three corpora (KrpLx gcc272cdk 0.095, Tu4L2 psyq35 0.083, 5t0dj gcc272psx 0.059). k=5 result is below both s9's k=3 whole-function max (0.127) and s35's k=3 block-0-isolated max (0.122), continuing the monotone signal decrease that s35 named: this function's fingerprint is rare in the local corpus, not a shared cross-project pattern.

- [s36] [s36] Rederive modality now dead across 14 independent sub-axes (adds sub-axis 13 = tail-block-isolated corpus shingle, sub-axis 14 = whole-function k=5 corpus shingle to the 12 banked at s35). Every rederive lever the modality can reach converges on either the H1 shape (banked KILLED via cse.c BB-scoped substitution wall) or a NEUTRAL free-axis permutation, or produces no corpus match above threshold.

- [s36] [s36] Six pass-level walls from s6/s7/s10/s11/s15/s16/s33/s34 unchanged and load-bearing (expand emit_insn -> jump.c pass1 -> cse.c BB substitution -> local/global_alloc pool split with no coalescer -> sched1+sched2 rank_for_schedule LUID tiebreak -> reorg.c delay-slot fill). 25 rejected forms banked in memory/grind/func_80045294/rejected/; no new form banked this session (rederive-modality axes produce no in-src measurement).

- [s36] [s36] docs/grind/decisions.md grep for '80045294' and 'saTan0Init' returns 0 hits (confirmed by direct grep). OWNER-ESCALATION entry not filed; per contract, 'owner-gated' is not emittable this session.

- [s36] [s36] Consistent with s28 modality-exhaustion table and s34/s35 pass-chain synthesis: hand-derivation dead across s1-s12+s29+s30 (multi-axis free-axis surface formally closed); rederive dead across 14 sub-axes; permuter dead across 9 chassis/mode combinations at ~137,752 cumulative iters; only sanctioned move remaining at the pipeline level is the owner ruling.
- [s37] Baseline reconfirmed at score=2 target=83 build=83 rules_dropped=0 cheat_asm_stripped=78 via `& tools/wteng.ps1 main sandbox func_80045294 --disable all`. Artifact: tmp/grind/func_80045294/s37/sandbox_baseline.txt.
- [s37] Six pass-level walls consolidated at direct pass-output tier: (1) EXPAND emit_insn insn 19 structural forcing [s34]; (2) combine.c substitute-and-delete refusal on pseudo 72 (a0) 3-BB uses [s33]; (3) cse.c BB-scoped ashift-operand substitution 72->77 [s7/s15/s25]; (4) local_alloc/global_alloc pool split with no coalescer [s6/s24]; (5) sched1+sched2 rank_for_schedule LUID tiebreak [s3/s15/s16]; (6) no pure-C CFG-splitter defeats cse's BB view — jump.c pass 1 collapses do-while(0) AND cse_extended_basic_block spans fall-through [s10/s11/s25].
- [s37] Rejected forms bank closed against pure-C search space at 25 forms mapping 1-to-1 onto the six walls; enumerated across categories A (statement position), B (decl/init decoupling), C (arithmetic-form), D (CFG splitters), E (CSE folding), F (operand order); no un-tested axis exists in any category [s28/s37].
- [s37] Permuter empirical closure at ~137,752 iters across 9 chassis/mode combinations (baseline minimal random s20 13977 + baseline full-TU random s13 17773 + directed lineswap s14 5249 + fresh-seed lineswap s22 720 + cse-fold-anon-shift alt s22 1310 + H1 chassis s23 37313 + H1 fresh-seed s31 21218 + baseline fresh s32 39420 + sum-at-2 s32 772); all basins converge on score=60 except cse-fold-anon-shift at 105 and s31 mid-loop-i-reset at score=10 (semantic break, banked rejected/perm-score10-mid-loop-i-reset-semantic-break.c).
- [s37] Rederive dead across 14 independent sub-axes: 10 m2c configurations (mips-gcc-c, mipsel-gcc-c, mips-ido-c, mipsel-mwcc-c, mipsee-gcc-c, --gotos-only, --no-stack-spill, --deterministic-vars, --stack-structs+--descending-regs, --context+--globals all) all converge on the H1 shape banked KILLED; Kengo debug/globals metadata empty; Kengo asm-level saTan0Init is a semantically different function (alloc+init pipeline vs per-frame accumulator); BB2 sibling functions in text1a_c.c share no structural analogue; decomp.me shingle-search max sim 0.127 whole-fn (s9) / 0.122 block-0-only (s35) / 0.156-collapsing-to-0.056 tail-block k=3->k=4 (s36) / 0.095 whole-fn k=5 (s36) — all below 0.15 threshold.
- [s37] Structural free-axis formally closed at s30: sum-position bounded to {0,1,2} (s29); two-axis probes A+B and three-axis probe C all NEUTRAL at score=2; multi-axis superposition surface exhausted.
- [s37] OWNER-ESCALATION entry NOT filed in docs/grind/decisions.md (verified `grep -c func_80045294 docs/grind/decisions.md` = 0 at s37 same as s24); owner-gated emission is blocked THIS session per contract precondition. Frontier reset to single top item: DRAFT the escalation entry citing the six walls + 25-form bank + 137,752-iter permuter budget + 14 rederive sub-axes + s30 free-axis closure; supplementary evidence: rare-fingerprint signal across three narrowed corpus searches.

- [s37] s37 sandbox baseline reconfirmed at score=2 target_insns=83 build_insns=83 rules_dropped=0 cheat_asm_stripped=78 (artifact: tmp/grind/func_80045294/s37/sandbox_baseline.txt).

- [s37] Six pass-level walls chained upstream-to-downstream at direct pass-output tier: EXPAND emit_insn insn 19 structural forcing (s34) -> combine.c substitute-and-delete refusal on pseudo 72 3-BB uses (s33) -> cse.c BB-scoped ashift-operand substitution 72->77 (s7/s15/s25) -> local/global_alloc pool split with no coalescer (s6/s24) -> sched1+sched2 rank_for_schedule LUID tiebreak (s3/s15/s16) -> no pure-C CFG-splitter defeats cse's BB view via BOTH jump.c collapse AND cse_extended_basic_block spanning (s10/s11/s25).

- [s37] 25-form rejected bank closed against reachable pure-C search space; every form maps 1-to-1 onto one of the six walls; categories A/B/C/D/E/F all enumerated.

- [s37] Permuter empirical closure at ~137,752 iters across 9 chassis/mode combinations (s13 17773 + s14 5249 + s20 13977 + s22 720 + s22-alt 1310 + s23 37313 + s31 21218 + s32 39420 + s32-sum 772); all basins converge on score=60 except cse-fold-anon-shift at 105 and s31 semantic-break at score=10.

- [s37] Rederive dead across 14 independent sub-axes (10 m2c configurations converge on H1-shape, Kengo debug/globals empty, Kengo asm semantically different function, BB2 siblings no structural analogue, decomp.me max sim 0.127 whole-fn / 0.122 block-0-only / 0.156-collapsing-to-0.056 tail-block / 0.095 whole-fn k=5 -- all below 0.15 threshold).

- [s37] Structural free-axis formally closed at s30 (sum-position bounded to {0,1,2}; two-axis A+B and three-axis C superposition all NEUTRAL at score=2).

- [s37] OWNER-ESCALATION entry NOT filed in docs/grind/decisions.md (grep -c = 0 at s37, unchanged from s24); owner-gated emission blocked this session per contract precondition.
