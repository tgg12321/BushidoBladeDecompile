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
