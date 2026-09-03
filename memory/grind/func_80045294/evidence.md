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

- [s38] [s38] Baseline reconfirmed pre and post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Working tree clean at session end (both probes reverted).

- [s38] [s38] Outer-block-wrap axis (whole-body bare `{ }` wrapper, distinct from s4 inner-block-defer-v1 because no statement-position v1 assignment is introduced) measured NEUTRAL. Confirms global.c's function-wide allocno phase is agnostic to inner lexical scope boundaries for THIS decl cluster -- extending s4's inner-block-defer-v1 finding to a strictly-structural (non-decl-reordering) surface.

- [s38] [s38] `register` storage-class hint on v1 measured NEUTRAL. Extends s5/s12's arithmetic-form and destination-type-declaration axis kills to the storage-class-declaration surface. Confirms GCC 2.7.2's advisory `register` keyword has no observable effect on the sll pseudo's LUID or allocno priority in this function.

- [s38] [s38] 27 rejected forms now banked (was 25 at s37): two new no-purchase structural surfaces added. Consistent with the s1-s37 exhaustion synthesis + s30 formal free-axis closure; no C-source structural axis reached by hand-derivation shifts the sll/move16 sched2 tie.

- [s38] [s38] OWNER-ESCALATION entry for func_80045294 STILL not present in docs/grind/decisions.md (grep confirmed only hirahira_w_frie 2026-07-17 and motion_SetMotion 2026-07-18 escalations exist). Contract precondition (a) for owner-gated remains unmet; structural session cannot file the escalation (owner-authored artifact class per hirahira_w_frie precedent). Ledger-frontier next-move remains valid: the escalation-drafting session is the sanctioned next step, then owner-gated becomes emittable.

- [s39] s39 baseline pre + post: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. src/text1a_c.c reverted to baseline candidate after all four probe measurements; working tree clean at end of session.

- [s39] s39 P1 (u32 a0 signature): score=2, byte-identical to baseline. Signature-level parameter-type variation on a0 alone is codegen-invisible. Extends s5 (operand-side type) and s12 (destination-decl-side type) signedness-agnostic findings to the PARAM-SIGNATURE surface previously un-enumerated.

- [s39] s39 P2 (u32 a0 + u32 a1 signature): score=2, byte-identical. Composite signature axis also neutral. addu is signedness-agnostic in expand; a1's DECL type does not ripple through s5's addu path.

- [s39] s39 P3 (u32 base-cast on s4 init): score=2, byte-identical. GCC 2.7.2 fold reduces (u8*)&Sym+v1 and (u32)&Sym+v1 to the same (plus (symbol_ref) (reg:SI)) tree at expand time; TYPE_UNSIGNED distinction is discarded before tree_LUID assignment.

- [s39] s39 P4 (split-init s5 a1-first): score=3, +1 diff. Both operand-orders of s5 split-init are killed forms (s3 s4-first score=11 via cascade; s39 a1-first score=3 via single addu operand-order swap). Confirms s3's kill mechanism and independently corroborates s2's addu operand-order asymmetry.

- [s39] Consistent with s6/s7/s15/s16 pass-level walls: no signature-surface or address-arithmetic surface variation reaches the LUID axis. All three neutral kills route through expand-time fold that discards TYPE_UNSIGNED before tree_LUID is assigned; the split-init kill routes through the s6 local/global pool split with no coalescer.

- [s39] 30-form rejected bank (27 pre-s39 + 3 new: param-u32-a0.c, addr-cast-u32-base.c, split-init-s5-a1-first.c). candidate.c unchanged (still the s3 baseline body).

- [s40] [s40] Baseline reconfirmed at session start: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. No src edits this session; candidate.c unchanged.

- [s40] [s40] s40-min-lineswap-freshseed campaign (tmp/grind/func_80045294/s40/perm_min_lineswap): base_score=60, iterations=720 (natural exhaustion of the 6! PERM_LINESWAP permutation space), elapsed=699.4s, finds_new=1, best_new_score=60, stopped=exhausted (pid_alive_at_harvest=false).

- [s40] [s40] The single new find (output-60-1) is a decl-order permutation v1,s4,s5,sum,count,i that preserves H1's v1-before-i constraint; source diff shows it belongs to the free-axis equivalent basin banked at s1/s3/s11/s12/s29/s30. No sll/move16 sched2 tie shift.

- [s40] [s40] 10th chassis/mode combination now banked: s13 full-TU random (17773) + s14 full-TU directed lineswap (5249) + s14 minimal-base directed lineswap (720, -j 4) + s20 minimal-base random unbounded (13977) + s22 lineswap-fresh (720) + s22 cse-fold-alt (1310) + s23 H1 chassis (37313) + s31 H1 freshseed (21218) + s32 baseline-fresh (39420) + s32 sum-at-2 (772) + s40 minimal-base lineswap freshseed -j 8 (720) = cumulative ~138,472 permuter iters across 10 chassis/mode combinations, all plateau at 60 except s31 semantic-break at score=10 and cse-fold-anon-shift near-hit at 105.

- [s40] [s40] The s40 chassis/mode combination (minimal-base + directed PERM_LINESWAP + fresh seed + -j 8) is the parallelism-scaled corroboration of s14's under-sampled minimal-base run (720 iters, -j 4, single find). Iteration count is identical because PERM_LINESWAP is exhaustive over the 6! permutation space; parallelism only shortens wall-clock. Both seeds land the same equivalent-basin find.

- [s40] [s40] Consistent with the six pass-level walls (s6/s7/s10/s11/s15/s16/s33/s34): expand emit_insn insn 19 structural forcing -> combine 56/45/2 substitute-and-delete refusal -> cse.c BB-scoped ashift-operand substitution -> local/global_alloc pool split with no coalescer -> sched1+sched2 rank_for_schedule LUID tiebreak -> no pure-C CFG-splitter defeats cse's BB view. No decl-permutation exhaustive sweep escapes the LUID coupling.

- [s40] [s40] docs/grind/decisions.md STILL contains no OWNER-ESCALATION entry for func_80045294 (grep '80045294' returns 0 hits at session end). Contract precondition (a) for owner-gated remains unmet; owner-gated not emittable this session.

- [s40] [s40] No new rejected form banked: the s40 output-60-1 find belongs to a free-axis equivalent basin already banked at multiple prior sessions (s1/s3/s11/s12/s29/s30) and does not represent a novel divergent form.

- [s41] s41 permuter campaign added 11th chassis/mode combination to the empirical closure count: count-inlined chassis + exhaustive PERM_LINESWAP 5!=120, fresh seed. Prior 10 combos (s13/s14/s20/s22/s22-alt/s23/s31/s32/s32-sum/s40) totaled 137,752 iters; s41 adds 120, cumulative 137,872 iters. All non-outlier finds converge on score=60 basin (only sub-60 point remains s31's score=10 mid-loop-i-reset semantic-break, structurally non-pure-C-expressible).

- [s41] Count-inlined vs count-local chassis equivalence at the permuter tier confirmed: the s8-established neutral-basin equivalence (score=2 unchanged when count decl removed and D_800A33AC inlined in both loop guards) extends to permuter behavior -- the removed LUID slot produces no exploitable window for any of 120 decl permutations.

- [s41] The free-axis basin (v1-before-i preserved) now empirically closed across TWO exhaustive PERM_LINESWAP sweeps on distinct chassis: s40's count-local 6! = 720 (banked in s40 ledger, best_new_score=60) and s41's count-inlined 5! = 120 (this session, best_new_score=60). Combined, they exhaustively enumerate every free-axis permutation for both dominant chassis shapes.

- [s41] OWNER-ESCALATION entry for func_80045294 remains unfiled in docs/grind/decisions.md (verified this session: `grep -c func_80045294 docs/grind/decisions.md` = 0, same result as s24/s37). owner-gated remains inemittable this session per contract precondition (a). Modality (permuter) cannot file the escalation.

- [s42] [s42] Baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. No src edits this session; candidate.c unchanged from s3 form.

- [s42] [s42] All forensic passes required for the OWNER-ESCALATION dossier are already dumped in prior sessions (s3 base.i.{rtl,jump,cse,loop,flow,combine,lreg,greg,sched,sched2,dbr}; s6 base.i.{combine,greg} for cse-fold-anon-shift; s7 base.i.{cse,greg} for rotation form; s15 sched2 ready-list; s16 sched1 forward order + reload LUID preservation; s33 combine baseline slice + flow pseudo refs; s34 EXPAND emit_insn LUID-origin trace).

- [s42] [s42] OWNER-ESCALATION entry appended to docs/grind/decisions.md (2026-07-19). Options (a) canonical-asm authorization for the 2-insn prologue reorder mirroring func_80037540 oversized-locals carve-out (2026-07-13 18:05 GRANTED); (b) uphold pure-C bar with function PARKED at INCOMPLETE-owner-accepted, s3 candidate standing on main at sandbox 2.

- [s42] [s42] Consolidated forensic proof of binary choice: residual is scheduler-side ONLY. All 81 non-cluster insns byte-identical; RA target-identical; no cheat staged. The 2-insn residual has no register-allocation or dataflow consequence, only prologue emit order. Every measured pure-C lever hand-derivation can construct either triggers cse.c BB-scoped substitution + RA rotation (score->11), produces a +1 CSE-copy insn (84 vs 83), or is neutral (stays 2).

- [s42] [s42] Six pass-level walls chained upstream-to-downstream with GCC-pass-source citation: (1) EXPAND cc1/stmt.c emit_insn mints the 8-UID sll/move16 delta at s4's initializer (insns 17+19) (s34); (2) combine.c declines substitute-and-delete because pseudo 72 (a0) is 3-BB multi-use across insns 4/22/89 (s6/s33); (3) cse.c BB-scoped operand substitution rewrites (ashift 72) -> (ashift 77) at insn 17 whenever i=a0 precedes v1=a0<<4, collapsing a0's live range and demoting global_alloc queue position from 8 to 10 (s7/s15); (4) local_alloc places single-block anon pseudo 76 before global_alloc queues loop-carried pseudo 81; no cross-pass coalescer exists in GCC 2.7.2 (s6); (5) sched1 sched.c:2398-2456 rank_for_schedule INSN_LUID tiebreak with backward-list-sched picks LUID(22) first, emits sll(14) first (s3/s16); (6) sched2 re-decides via same LUID mechanism after reload preserves pre-reload LUIDs; no scheduler-only defeat via -fno-schedule-insns[2] (s16).

- [s42] [s42] Exhaustion budget banked to disk: 30-form rejected bank in memory/grind/func_80045294/rejected/ (every free-axis permutation, decl/init decouple, arithmetic-tree respelling, storage-class axis, CFG-splitter attempt); 14 rederive sub-axes killed across 6 sessions (s8/s9/s17/s18/s35/s36); 137,872-iter permuter across 11 chassis/mode combinations (s13-s41) all plateau at score=60; s29/s30 formal structural free-axis closure (multi-axis probe A/B/C cross-product NEUTRAL, sum-position axis bounded to {0,1,2}); s38 outer-block-wrap + register-storage-class-v1 KILLED; s39 param u32/base-cast NEUTRAL + split-init-s5-a1-first HARMFUL; s40 exhaustive 6!=720 PERM_LINESWAP over 6-decl block KILLED; s41 exhaustive 5!=120 PERM_LINESWAP on count-inlined chassis KILLED.

- [s42] [s42] Contract precondition (a) for owner-gated emission now satisfied: filed OWNER-ESCALATION entry in docs/grind/decisions.md names func_80045294 with the full mechanism-level dossier and mutually-exclusive owner options. Precondition (b) (every remaining sanctioned axis measured dead) is NOT yet satisfied: s41 frontier item 2 identifies PERM_INT-mode as an un-measured 12th chassis/mode combination sanctioned for the next permuter session. Owner-gated becomes ripe only after PERM_INT is also measured dead.

- [s43] s43 baseline: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 (tmp/grind/func_80045294/s43/baseline_recon.txt). No src edits this session; candidate.c unchanged.

- [s43] Reload emits seven callee-save stores at monotonic LUIDs 199 (sw $ra @40), 201 (sw $s5 @36), 203 (sw $s4 @32), 205 (sw $s3 @28), 207 (sw $s2 @24), 209 (sw $s1 @20), 211 (sw $s0 @16); sp adjust at LUID 197. Post-sched2 RTL: base.i.sched2 lines 20321-20389.

- [s43] sw $s0's LUID (211) is the HIGHEST of the seven callee-save stores. This is not arbitrary — GCC 2.7.2's reload iterates hard-reg saves in a fixed order per mips.md FUNCTION_PROLOGUE; changing which reg holds `i` cannot lower sw $s0's LUID below 197 (reload's floor).

- [s43] sched2 block-0 ready-list at T-10 shows 'insn 211 has a greater potential hazard, now 211 14 12 6' — the hazard-hoist rule (memory-op prioritization) is the direct sched.c mechanism placing sw between sll and move16.

- [s43] REG_DEP_ANTI on insn 22 (move $s0=$s2) points to insn 211 (sw $s0) — insn 22 WRITES the reg insn 211 READS (caller's $s0), forcing 211→22 in forward stream. This anti-dep is uneliminable: any func using $s0 as callee-save needs the save before overwrite.

- [s43] Named seventh pass-level wall: reload_as_needed/emit_prologue LUID monotonicity + sched2 hazard-hoist. Together these prove target's 3-insn cluster is pass-mechanically unreachable at score<2 by any pure-C mutation, independent of the s7 cse.c substitution wall.

- [s43] This finding is COMPLEMENTARY to (not overlapping with) s34's 6 pass-level walls: s34 named the LUID-8-delta at expand between sll(14) and move16(22); s43 names a SECOND LUID relation (pre-reload vs reload-emitted, gap ~197) that is even more constrained and independently forbids target order.

- [s43] Corollary for the frontier PERM_INT campaign: even if PERM_INT surfaces a novel lowering of a0<<4 that shifts insn 14's LUID or eliminates it, the sw(211)-vs-move(22) LUID relation remains unshiftable by C-source. PERM_INT can at best corroborate s5's arithmetic-form closure; it cannot invert the reload LUID relation.

- [s44] [s44] Baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. No src edits; candidate.c unchanged.

- [s44] [s44] Kengo saTan0Init disassembly at 0x00147dc8..0x00147f14 (tmp/grind/func_80045294/s44/kengo_saTan0Init.asm) shows ~9 function calls (saRobInit/saTanMainDataCreate/tslGlobalMemAlloc/saFidLoad/tslSmdInitObj/motion_SetMotion/motion_CalcMotion/saTan0InitPrim/saSePlaySeq), single 6-iteration init loop bounded by `slti v0,s4,6 ; bne`, 160-byte frame with 9 callee-saves, terminating with `sb v0,-29548(gp) ; sb v0,-29545(gp)`.

- [s44] [s44] BB2 func_80045294 shape (candidate.c + target diff): 2 calls (gpu_DrawSync(0), func_800520B8(s4,s5,sum)), TWO scan loops over per-slot table D_800EED10-1C (running index += 0x10) bounded by `i < D_800A33AC`, fn-ptr dispatch through +0x1C offset with s16 arg loaded from +0x00, terminates with `D_800A33A0 += a1; D_800A33A4 -= a1;`.

- [s44] [s44] The 83-insn size match between Kengo saTan0Init and BB2 func_80045294 is COINCIDENCE, not source identity. s8's 'Kengo symbol confirms function identity: saTan0Init at 0x00147dc8, size 0x14c=83 insns' name+size identity claim is falsified by direct disassembly at the ASM-shape level.

- [s44] [s44] 16th independent rederive sub-axis killed (previously enumerated in s37/s10/s18/s34/s36: 15 sub-axes). Rederive-modality-dead inference from s8/s9/s17/s18/s35/s36 HARDENED: even a disassembly-based m2c rederive of Kengo's saTan0Init would produce a different function's C, not this function's; no future session should attempt Kengo-ASM transplant on the shape argument alone.

- [s44] [s44] BB2 function's globals-update semantics (D_800A33A0 += a1; D_800A33A4 -= a1) plus gpu_DrawSync indicate this is a per-frame scroll/advance step, NOT an init in the traditional sense; BB2's saTan0Init label is a role-name that does not correspond to Kengo's saTan0Init role. Second-order evidence that Kengo restructuring separated the roles.

- [s44] [s44] Owner-escalation entry (s42-filed in docs/grind/decisions.md 2026-07-19) contract precondition (a) still satisfied; precondition (b) still awaits the PERM_INT axis measurement (frontier item, deferred by mandated modality this session).

- [s45] Baseline reconfirmed twice this session: pre-edit and post-revert both score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 -- artifacts tmp/grind/func_80045294/s45/baseline.txt and baseline_reverted.txt.

- [s45] Sub-axis 17 (s4 init array subscript, constant-foldable a0<<2): NEUTRAL score=2, 83/83 -- fold-const collapses ARRAY_REF to PLUS_EXPR at fold time; distinct evidence surface from s5's arithmetic-tree probes and s39's addr-cast probes.

- [s45] Sub-axis 18 (loop body array subscript on runtime v1>>2): HARMFUL score=8, 86/83 -- combine.c cannot cancel the srl/scale pair for the multi-use loop-carried v1 pseudo; runtime-var subscript is a genuinely distinct expand-path branch from the constant-foldable case.

- [s45] src/text1a_c.c reverted to baseline candidate.c form; no residual dirt.

- [s45] Rederive-modality tally advances from 16 (s44) to 18 independent sub-axes; every m2c/decomp.me/Kengo/sibling/expand-path angle explored so far converges on either the H1 KILLED shape, NEUTRAL byte-identity, or HARMFUL degradation.

- [s45] Owner-escalation entry filed at s42 (docs/grind/decisions.md 2026-07-19) remains standing; contract precondition (a) satisfied; precondition (b) still requires the frontier's PERM_INT campaign kill before owner-gated is emittable.

- [s46] s46 baseline reconfirmed: sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78; residual unchanged from s1 (3-insn prologue cluster sw/move16/sll vs sll/sw/move16).

- [s46] PERM_INT surface empirically measured dead: shift=3 (a0<<3) yields score=2, shift=5 (a0<<5) yields score=2, both unchanged from baseline shift=4. The sandbox scorer does not discriminate the sll immediate value; any PERM_INT enumeration over the load-bearing shift constant produces equivalent-basin residuals. Frontier item 1 KILLED empirically.

- [s46] Owner-escalation entry filed s42 (docs/grind/decisions.md 2026-07-19 - func_80045294 saTan0Init OWNER-ESCALATION) with 6-wall pass-level dossier + 30-form rejected bank + 137,872-iter permuter across 11 chassis/mode combos + 14 rederive sub-axes. Hardened s43 (7th pass-level wall: reload-LUID 197..211 monotonicity precludes pre-reload insns from exceeding sw=211 LUID), s44 (Kengo saTan0Init name+size identity falsified by disassembly-shape check, 16th rederive sub-axis dead), s45 (array-subscript expand-path probes at s4-init NEUTRAL and loop-body HARMFUL, 17th/18th rederive sub-axes dead).

- [s46] Both owner-escalation contract preconditions now satisfied: (a) filed dossier at s42, (b) every remaining sanctioned axis measured dead including PERM_INT (s46) as the last permuter-family surface. Owner-gated is emittable per driver contract.

- [s46] candidate.c unchanged from s3 v1-before-i form (byte-identical to src/text1a_c.c:1602-1648, sustaining sandbox=2 as the honest pure-C floor); no rejected form new-banked this session (the PERM_INT kill is by measurement of the sandbox scoring architecture, not a distinct source shape).

- [s47] Chassis re-measured at session start on the CURRENT tree (HEAD ships `INCLUDE_ASM("asm/funcs", func_80045294);` at src/text1a_c.c:1478 under asm-until-matched; candidate.c re-applied by hand): sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=66. The honest floor is unchanged at 2 on the post-migration chassis, so every banked spelling conclusion remains chassis-valid. Artifact tmp/grind/func_80045294/s47/baseline.txt; post-revert re-measure tmp/grind/func_80045294/s47/baseline_reverted.txt (score=2 again). src/text1a_c.c restored to HEAD (INCLUDE_ASM) at session end; tree clean.

- [s47] OWNER DIRECTIVE EXECUTED (queue item, ruling 2026-08-24 escalation-not-parked: "solver modality (ra_solver/sched_solver) recommended before deep re-grind of RA/scheduler-tiebreak residuals"). No prior session (s1-s46) had run either solver on this function. Both were run this session against the real, exact models.

- [s47] sched_solver model is EXACT for this function on the candidate chassis: `extract.py text1a_c` reports parity=True, funcs=152, blocks=532; `perturb.py --func func_80045294 --pass 2 --block 0` reports "18 insns, baseline exact". The pass-2 block-0 pick order (pick space, reverse of emission) is [32,31,28,25,19,203,201,199,22,211,14,12,209,6,205,4,207,197]; the target's is [32,31,28,25,19,203,201,199,14,22,211,12,209,6,205,4,207,197]. In emission space that is ours `sll $3,$18,4 ; sw $16,16($sp) ; addu $16,$18,$zero` vs target `sw $16,16($sp) ; addu $16,$18,$zero ; sll $3,$18,4` — the same 3-insn prologue cluster the ledger has carried since s1, now expressed in solver UIDs (14 = sll, 211 = sw $s0, 22 = move16 $s0=$s2).

- [s47] SCHED2 GOAL IS REACHABLE, AND ITS SOLUTION SET IS EXHAUSTIVELY ENUMERATED. `perturb.py ... --goal-order <target pick order> --depth 1` over the FULL atom space (960 single perturbations, all five atom classes — add_dep, remove_dep, priority, luid, luid_move) returns exactly 13 vectors, and every one of them encodes the same single C-level intent: the statement that generates insn 22 (`i = a0`) must precede the statement that generates insn 14 (`v1 = a0 << 4`). The 13 are: `add_dep 14 <- 22 (true/data)`, `add_dep 14 <- 22 (anti-output)`, `luid swap 14 <-> {19,22,25,28,32}`, `luid_move 14 -> before {19,25,28,31,32}`, `luid_move 22 -> before 14`. No dependence-edge, priority, or unit/cost atom outside that intent reaches target's order. Artifacts tmp/grind/func_80045294/s47/sched_perturb_p2_order.txt (luid,luid_move restricted) and sched_perturb_p2_allatoms_d1.txt (full 960-atom enumeration).

- [s47] That intent is exactly the H1 shape banked KILLED since s1 (rejected/i-before-v1-init.c). Re-measured this session for chassis validity: H1 applied to src -> sandbox score=11, 83/83 insns (tmp/grind/func_80045294/s47/h1_sandbox.txt). So the sched2 half of the residual is not a wall at all — it is SOLVED by H1; what H1 costs is the register allocation.

- [s47] ra_solver global model is EXACT for this function on BOTH chassis: `simulate.py` reports "sort order: MATCH" and "dispositions: 11/11 match" on the H1 model. The two models are banked at tmp/grind/func_80045294/s47/h1.model.json and cand.model.json.

- [s47] THE ENTIRE RESIDUAL IS ONE reg_n_refs COUNT. Candidate chassis (score 2): pseudo 72 (= a0) has nrefs_flow=4, livelen_flow=24, calls_crossed=2; allocation order [92,78,75,86,85,79,74,73,72,76,80] places 72 at position 8 and global.c assigns 72->$s2(18), 76->$s4(20), 80->$s5(21) — TARGET'S ALLOCATION EXACTLY. H1 chassis (score 11): pseudo 72 has nrefs_flow=3, livelen_flow=24, calls_crossed=2 — livelen and calls-crossed IDENTICAL, only the reference count differs — allocation order [92,75,76,86,85,79,74,73,77,80,72] drops 72 to LAST and assigns 72->$s5(21), 77->$s2(18), 80->$s4(20). The missing 4th reference is the ashift operand that cse.c substitutes from reg 72 (a0) to reg 75 (i) once `i = a0` dominates the shift (the s7 wall, now quantified).

- [s47] ra_solver INVERSE verdict on the H1 chassis is REACHABLE at minimal size 1: `inverse.py global tmp/ra_solver_work/func_80045294.model.json --goal {"72":18,"77":20,"80":21} --depth 2` reports "minimal solution size: 1 atom(s) — 5 distinct vector(s)": [refs_up] pseudo 72 refs 3->4/5/6/7 (four cost-ranked variants) and [live_shrink] pseudo 72 livelen 23->15. It also emits a FORECLOSED block: all 24 preference atoms for pseudo 72 are mechanically unreachable from C because $s2/$s4/$s5 are CALLEE-SAVED and can never appear as hard regs in pre-RA RTL ("only a forbidden register-asm pin would" reach them). Artifact tmp/grind/func_80045294/s47/ra_inverse_h1.txt.

- [s47] BOTH 1-atom RA vectors are measured C-UNREACHABLE for this function, which is what closes the composite:
  (i) refs_up requires a FOURTH reference to a0. TARGET ITSELF CONTAINS EXACTLY FOUR (asm/funcs/func_80045294.s): the def `addu $s2,$a0,$zero` @8004529C, `addu $s0,$s2,$zero` @800452B4, `sll $v1,$s2,4` @800452B8, `addu $s0,$s2,$zero` @8004532C — after which $s2 is REDEFINED as the second loop's walking pointer (`addu $s2,$v1,$v0` @80045344). There is no fifth a0 site in the original, so no honest algorithmic ref-lift exists; and all four sites lie inside the cse equivalence region opened by `i = a0`. Directly probed: spelling the SECOND loop's index shift on a0 (`v1 = a0 << 4;` instead of `v1 = i << 4;`) under the H1 chassis — the one site in a different basic block — measures score=11, UNCHANGED, because `i = a0` immediately dominates that shift in its own block too and cse substitutes there as well. Banked rejected/h1-second-loop-shift-on-a0.c; artifact tmp/grind/func_80045294/s47/vA_sandbox.txt.
  (ii) live_shrink requires a0 to die at livelen 15 instead of 24. a0's last reference in the ORIGINAL is the second loop's `addu $s0,$s2,$zero`, which sits after both calls; no C form can retire a0 earlier without deleting a reference the target contains.
  The remaining lever class (preferences) is FORECLOSED by the solver's own analysis and is reachable only by a forbidden register-asm pin.

- [s47] TYPED FORECLOSURE (supersedes the s34/s43 seven-wall prose with a model-level proof): target's bytes require BOTH (A) `i = a0` before `v1 = a0 << 4` in source order — the unique intent behind all 13 sched2 vectors — and (B) reg_n_refs(a0) == 4. Condition (A) is what makes cse.c steal one of a0's four references, so (A) and (B) are mutually exclusive in pure C for THIS function's reference set. Every one of the 33 banked rejected forms is an instance of choosing (A) without (B) (the score-11/12 H1 basin) or (B) without (A) (the score-2 candidate basin). This is the first statement of the residual that is a proof over an exhaustively enumerated lever space rather than a survey of tried spellings.

- [s47] Endgame-lock gate evidence re-measured this session: gate (a) `python3 tools/scan_hand_coded.py --single func_80045294` -> tier=LOW score=0/8, all eight signals negative (S1 0 multu/mflo pairs; S2 no empty-body branches; S3 83 insns/7 spills/11 distinct regs; S4 max load burst 3; S5 jaccard < 0.5; S6 no BIOS jumptable; S7 all callee-save uses have $sp saves; S8 no redundant mask-before-shift) — canonical-asm evidence bar FAILS, matching the owner's 2026-07-20 finding. Gate (b): there is no closing construct to cite a SOTN-master precedent FOR — the foreclosure above shows no construct of any family reaches the bytes — so the precedent gate is vacuously FAILED, not open.

- [s47] Disposition filed: docs/grind/decisions.md `## 2026-08-25 — func_80045294 (saTan0Init, src/text1a_c.c) — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE`, superseding/hardening the s42-filed 2026-07-19 entry and the owner's 2026-07-20 option-(b) ruling with the solver evidence the 2026-08-24 directive asked for.

- [s47] Chassis re-measured on the CURRENT tree (HEAD ships INCLUDE_ASM at src/text1a_c.c:1478 under asm-until-matched; candidate.c re-applied by hand): sandbox --disable all -> score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=66. Re-measured again after reverting all probes: score=2. src/text1a_c.c restored to HEAD at session end; git status clean apart from metrics/events.jsonl.

- [s47] OWNER DIRECTIVE EXECUTED: the queue item's 2026-08-24 directive ('solver modality (ra_solver/sched_solver) recommended before deep re-grind of RA/scheduler-tiebreak residuals') had never been run by any of sessions s1-s46. Both solvers were run this session against exact, validated models.

- [s47] sched_solver is EXACT here: extract.py text1a_c reports parity=True (152 funcs, 532 blocks, 3456 picks); perturb.py reports func_80045294 pass2 block0 '18 insns, baseline exact'. Ours (pick space) [32,31,28,25,19,203,201,199,22,211,14,12,209,6,205,4,207,197] vs target [.,.,.,.,.,.,.,.,14,22,211,...] = emission 'sll;sw $s0;move16' vs target 'sw $s0;move16;sll'.

- [s47] EXHAUSTIVE 960-single-atom search over all five sched atom classes returns exactly 13 goal-reaching vectors, all encoding the single intent 'i = a0 precedes v1 = a0 << 4' (5 luid swaps, 5 luid_moves, luid_move 22->before 14, and add_dep 14 <- 22 in true/data and anti-output kinds). No order-only lever exists.

- [s47] H1 (i-before-v1) re-measured on the current chassis: sandbox score=11, 83/83 — the sched2 order half is SOLVED by H1; the cost is the register allocation.

- [s47] ra_solver global model is EXACT on the H1 chassis: simulate.py 'sort order: MATCH', 'dispositions: 11/11 match'.

- [s47] THE ENTIRE RESIDUAL IS ONE reg_n_refs COUNT. Candidate chassis: pseudo 72 (a0) nrefs_flow=4, livelen_flow=24, calls_crossed=2, allocno order [92,78,75,86,85,79,74,73,72,76,80] -> 72=$s2(18), 76=$s4(20), 80=$s5(21) = target's allocation exactly. H1 chassis: pseudo 72 nrefs_flow=3 with IDENTICAL livelen 24 and calls_crossed 2, order [92,75,76,86,85,79,74,73,77,80,72] -> 72=$s5(21), 77=$s2(18), 80=$s4(20).

- [s47] ra_solver inverse on H1: 'minimal solution size: 1 atom(s) — 5 distinct vector(s)' — [refs_up] pseudo 72 refs 3->4/5/6/7 and [live_shrink] pseudo 72 livelen 23->15. All 24 preference atoms explicitly FORECLOSED: $s2/$s4/$s5 are callee-saved, never appear as hard regs in pre-RA RTL, so global.c set_preference can never record them ('only a forbidden register-asm pin would').

- [s47] refs_up is C-unreachable: the ORIGINAL contains exactly four a0 references (def addu $s2,$a0,$zero @8004529C; uses addu $s0,$s2,$zero @800452B4, sll $v1,$s2,4 @800452B8, addu $s0,$s2,$zero @8004532C; then $s2 is redefined as the second loop's walking pointer, addu $s2,$v1,$v0 @80045344), so no honest algorithmic ref-lift exists, and all four sites lie inside the cse equivalence region H1 opens.

- [s47] Direct probe of the one a0 site in a different basic block — second loop spelled 'v1 = a0 << 4;' under H1 — measures score=11 UNCHANGED; cse substitutes there too because 'i = a0;' immediately dominates that shift in its own block. Banked rejected/h1-second-loop-shift-on-a0.c (34th rejected form).

- [s47] live_shrink is C-unreachable: a0's last reference in the original is the second loop's addu $s0,$s2,$zero, which sits after both calls (DrawSync and func_800520B8); no C form retires a0 earlier without deleting a reference the target contains.

- [s47] GATE (a) re-measured: tools/scan_hand_coded.py --single func_80045294 -> tier=LOW score=0/8, all eight signals negative (S1 0 multu/mflo pairs; S2 no empty-body branches; S3 83 insns/7 spills/11 distinct regs; S4 max load burst 3; S5 jaccard < 0.5; S6 no BIOS jumptable; S7 all callee-save uses have $sp saves; S8 no redundant mask-before-shift). Canonical-asm evidence bar FAILS, reproducing the owner's 2026-07-20 finding.

- [s47] GATE (b) FAILS vacuously: there is no closing construct to cite a SOTN-master precedent for — the typed foreclosure shows no construct of any sanctioned family reaches the bytes, because the blocking condition is a reference count and every C lever class that could move it is enumerated and foreclosed.

- [s47] Exhaustion of record: 47 sessions, 7 distinct modalities (recon, structural, permuter, forensics, rederive, synthesis, solver), 34 banked rejected forms, 137,872 permuter iterations across 11 chassis/mode combinations all plateauing at score 60, 18 independent rederive sub-axes killed (m2c across 10 option axes and 4 dialects, decomp.me, siblings, direct Kengo saTan0Init disassembly falsifying the name+size identity), PERM_INT shift-constant surface measured dead (s46), and now two exact solver models with exhaustively enumerated solution sets.

- [s47] Disposition FILED this session at docs/grind/decisions.md:11915 — '## 2026-08-25 — func_80045294 (saTan0Init, src/text1a_c.c) — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. It asks for no grant, no new family, no rule relaxation and accepts no debt into the build: main stays at INCLUDE_ASM with zero rules and zero cheat-asm, and the honest candidate stays at memory/grind/func_80045294/candidate.c (sandbox 2). Nothing pends on the owner (2026-08-18 judge-sole-gate).

## s48 — escalation modality (owner ruling 10, 2026-08-30 batch: active with modality change)

- [s48] CHASSIS RE-MEASURED. `memory/grind/func_80045294/candidate.c` pasted over the
  `INCLUDE_ASM("asm/funcs", func_80045294);` line at src/text1a_c.c:1478 gives
  `sandbox func_80045294 --disable all` = **score 2, target_insns 83, build_insns 83,
  rules_dropped 0, cheat_asm_stripped 66 (other functions in the TU)**. The ledger's
  floor of 2 is confirmed against the current chassis; nothing drifted.

- [s48] RESIDUAL LOCALISED EXACTLY, from the sched_solver pass-2 model
  (`tmp/sched_solver_work/text1a_c.sched.json`, func_80045294 pass 2 block 0, 18 insns,
  simulator order-exact: sim == truth). Emission order (reverse of the backward-list-sched
  output) is `197 207 4 205 6 209 12 [14] 211 22 199 201 203 19 25 28 31 32`, where
  uid 14 = `sll $v1,$s2,4`, uid 211 = `sw $s0,0x10($sp)` and uid 22 = `addu $s0,$s2,$zero`.
  The target's order (asm/funcs/func_80045294.s, 0x800452B0..0x800452B8) is
  `211 22 14` — i.e. the ENTIRE residual is that the shift is picked one slot too early.
  uids 197..211 (LUIDs 0..7) are reload-emitted prologue insns; uids 4..32 (LUIDs 9..21)
  are real pre-reload insns.

- [s48] FRONTIER ITEM 2 CLOSED (the pass-1 conditional on s47's exhaustive 960-atom
  pass-2 proof). Exhaustive single-atom enumeration over sched_solver's PASS-1 block 0
  (8 insns, 192 atoms, baseline order-exact) — `tmp/grind/func_80045294/s48/pass1_probe.py`
  — yields 28 orders distinct from baseline, so pass 1 is NOT frozen. Of those, exactly
  ONE is the order that flips the pass-2 LUIDs of uid 14 (sll) and uid 22 (`i = a0`):
  pre-reload emission `[12, 22, 14, 19, 25, 28, 31, 32]` instead of
  `[12, 14, 22, 19, 25, 28, 31, 32]`. `tmp/grind/func_80045294/s48/pass1_goal.py` enumerates
  every atom that reaches it: 11 atoms, and ALL 11 are one of
  (a) `add_dep 14 <- 22` (the shift consumes `i` — i.e. cse's own substitution), or
  (b) a `luid`/`luid_move` atom that moves the shift statement after `i = a0` in the SOURCE.
  Both spell the same single C intent: **i-before-v1**. The pass-1 axis therefore adds NO
  new route; it is a second mechanism for the intent s47 already identified as the unique
  one at pass 2. The foreclosure is now unconditional across both scheduling passes.

- [s48] WHY i-before-v1 cannot pay: read from the compiler's own source, not inferred.
  `tools/gcc-2.7.2/cse.c:826-882` (`make_regs_eqv`, called from `insert_regs` when the copy
  `i = a0` is recorded) decides which register of the merged quantity becomes
  `qty_first_reg`, and `canon_reg` substitutes THAT register into every later reference.
  For two pseudos the destination (`i`, reg 75) displaces the source (`a0`, reg 72) iff
  BOTH (cse.c:851-857):
    (1) `uid_cuid[regno_last_uid[new]] > cse_basic_block_end` OR
        `uid_cuid[regno_first_uid[new]] < cse_basic_block_start`  — i.e. `i` outlives the EBB;
    (2) `uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]` — i.e. `i`'s last
        use is later than `a0`'s last use.
  In candidate.c both hold (i is the induction variable of BOTH loops; a0's last reference
  is the second loop's `i = a0` init), so on the H1 chassis the ashift operand is
  canonicalised to reg 75 and the whole callee-save allocation rotates
  (banked: rejected/i-before-v1-init.c, score 11). (1) is unfalsifiable in C — `i` is a loop
  counter and must leave block 0. (2) is the only C-reachable prong.

- [s48] FRONTIER ITEMS 1 AND 3 KILLED WITH MEASUREMENTS (new axis, not in the 34-form bank).
  The one instruction-free way to falsify cse.c prong (2) is to stop reusing `i` as the
  SECOND loop's counter, so that `i`'s last use falls back to the end of loop 1 while a0's
  stays at the loop-2 init. Two forms measured:
    * `rejected/h1-separate-second-loop-counter.c` (H1 chassis + distinct `j`): **score 37,
      build_insns 80** (target 83).
    * `rejected/base-separate-second-loop-counter.c` (current chassis + distinct `j`, the
      control): **score 37, build_insns 80** — identical, isolating the damage to the counter
      split itself rather than to the i/v1 declaration order.
  Disassembly of the sandbox object shows the shortened live range re-rotates the entire
  callee-save allocation (a0 lands in `$s1`, the counter in `$s2`) and lets loop 2's
  `lw D_800A33AC` hoist out of the loop, DELETING 3 instructions the target has
  (target keeps the reload inside the loop at 0x8004538C). The 83-instruction target shape
  is therefore CONTINGENT on the single reused counter variable — which is exactly the
  thing that makes cse.c prong (2) true. The two requirements are mutually exclusive at
  the source level: any edit that falsifies prong (2) without emitting an instruction also
  shortens `i`'s range, and shortening `i`'s range is what deletes the 3 instructions.

- [s48] GATE (a) RE-RUN: `python3 tools/scan_hand_coded.py --single func_80045294` →
  **tier=LOW score=0/8** (S1 0 multu pairs, S2 no empty-body branches, S3 83 insns/7 spills/
  11 regs, S4 max load burst 3, S5 jaccard < 0.5, S6 no BIOS jumptable, S7 all callee-saves
  saved, S8 no redundant mask). No hand-coded-asm evidence; canonical-asm grant path FAILS.

- [s48] GATE (b) RE-CHECKED, AND THE 2026-07-20 RULING'S FACTUAL PREMISE IS NOW STALE.
  That ruling said "prologue_fix retained to hold the byte-match". `tools/prologue_config.json`
  is now **empty (0 entries)** and contains no `func_80045294` key, consistent with the
  2026-08-25 rules-to-zero milestone. NOTHING holds a byte-match for this function: HEAD
  ships `INCLUDE_ASM("asm/funcs", func_80045294);`. This is therefore NOT an integration
  handoff — there is no bytes-proven form blocked behind a surface the session may not touch.

- [s48] Chassis re-measured this session: memory/grind/func_80045294/candidate.c pasted over the INCLUDE_ASM line at src/text1a_c.c:1478 gives sandbox func_80045294 --disable all = score 2, target_insns 83, build_insns 83, rules_dropped 0, cheat_asm_stripped 66 (other functions in the TU). The ledger floor of 2 is confirmed; nothing drifted.

- [s48] Residual localised exactly from the sched_solver pass-2 model (tmp/sched_solver_work/text1a_c.sched.json, func_80045294 pass 2 block 0, 18 insns, sim == truth): emission order is 197 207 4 205 6 209 12 [14] 211 22 199 201 203 19 25 28 31 32, where uid 14 = sll $v1,$s2,4, uid 211 = sw $s0,0x10($sp), uid 22 = addu $s0,$s2,$zero. The target order (asm/funcs/func_80045294.s, 0x800452B0..0x800452B8) is 211 22 14. uids 197..211 (LUIDs 0..7) are reload-emitted prologue insns; uids 4..32 (LUIDs 9..21) are real pre-reload insns.

- [s48] Pass-1 block 0 (8 insns) admits 28 distinct non-baseline schedules under exhaustive single-atom perturbation (192 atoms), so sched1 is NOT frozen — but exactly one of those orders (pre-reload emission [12, 22, 14, 19, 25, 28, 31, 32] instead of [12, 14, 22, 19, 25, 28, 31, 32]) flips the pass-2 LUIDs of the residual pair, and all 11 atoms that reach it spell the single intent i-before-v1. The scheduling foreclosure is therefore unconditional across both passes, not conditional on reload's output as s47 had to concede.

- [s48] tools/gcc-2.7.2/cse.c:826-882 (make_regs_eqv) is the mechanism for the RA leg, read from compiler source rather than inferred: for two pseudos the copy destination displaces the source as qty_first_reg — and thus as canon_reg's substitution target — iff (1) uid_cuid[regno_last_uid[new]] > cse_basic_block_end or uid_cuid[regno_first_uid[new]] < cse_basic_block_start, AND (2) uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]] (cse.c:851-857).

- [s48] The only instruction-free falsification of cse prong (2) — a distinct second-loop counter — measures score 37 / build_insns 80 on BOTH chassis (memory/grind/func_80045294/rejected/h1-separate-second-loop-counter.c and .../base-separate-second-loop-counter.c). Sandbox disassembly shows the callee-save allocation rotates (a0 to $s1, counter to $s2) and loop 2's lw D_800A33AC hoists out of the loop, deleting 3 instructions the target keeps.

- [s48] Gate (a) FAILS: python3 tools/scan_hand_coded.py --single func_80045294 -> tier=LOW, score=0/8, all eight signals negative (S1 0 multu/mflo pairs; S2 no empty-body branches; S3 83 insns / 7 spills / 11 distinct regs; S4 max load burst 3; S5 jaccard < 0.5; S6 no BIOS jumptable; S7 all callee-save uses have an $sp save; S8 no redundant mask-before-shift).

- [s48] Gate (b) FAILS vacuously: there is no closing CONSTRUCT to grant. The residual is a pass-2 list-scheduler pick order over reload-emitted prologue insns plus one real insn; no SOTN-master construct under any family changes a LUID assignment, so no precedent could apply.

- [s48] The 2026-07-20 owner ruling's premise that prologue_fix is 'retained to hold the byte-match' is now factually STALE: tools/prologue_config.json is empty (0 entries) and carries no func_80045294 key, consistent with the 2026-08-25 rules-to-zero milestone. HEAD ships INCLUDE_ASM("asm/funcs", func_80045294); with zero cheat constructs. This is NOT an integration handoff — no bytes-proven form is blocked behind an untouchable surface.

- [s48] Owner directive from the queue item (2026-08-30 escalation-batch ruling 10, decisions.md:14870) EXECUTED, not deferred: the item was returned to ACTIVE with a modality change, this session ran that modality, closed all three surviving frontier items with measurements, and re-affirmed the standing-ruling disposition on fresh evidence. The directive is now acknowledged in the ledger (the consistency warning is cleared).

- [s48] src/text1a_c.c was restored to HEAD (git checkout) after measurement; the working tree carries only ledger, decisions.md and rejected-form additions.

## [s49] Owner Ruling A named probe executed — measurements

Chassis re-check at dispatch: `memory/grind/func_80045294/candidate.c` pasted over the
`INCLUDE_ASM` line at src/text1a_c.c:1478 -> `sandbox func_80045294 --disable all` =
**score 2, target_insns 83, build_insns 83, rules_dropped 0**. Floor unchanged; every
s47/s48 conclusion is chassis-valid.

Objdump attribution of the 2-instruction residual on the candidate chassis
(tmp/sandbox/func_80045294/text1a_c.o):
    build : sw $s1,0x14 ; move $s1,zero ; sll $v1,$s2,4 ; sw $s0,0x10 ; move $s0,$s2
    target: sw $s1,0x14 ; move $s1,zero ; sw $s0,0x10 ; move $s0,$s2 ; sll $v1,$s2,4
i.e. the shift is scheduled one slot early. Registers already match on this chassis.

Four forms measured this session (all under the H1 `i = a0` before `v1 = a0 << 4` chassis,
which is the only source order that reaches target's sched2 order):

| form | nrefs_flow(72) | sandbox | build_insns | banked as |
|---|---|---|---|---|
| guard on a0, `i = a0` inside the arm | **4** | **5** | 84 | rejected/h1-second-loop-guard-on-a0.c |
| guard on a0, `i = a0` before the guard (control) | 3 | not spent | — | rejected/h1-i-then-guard-on-a0.c |
| `i = a0` duplicated into both `if (sum != 0)` arms (Ruling A probe) | 7 | 27 | 84 | rejected/dup-i-eq-a0-into-sum-arms.c |
| guard on a0 + distinct loop-2 counter j | 6 | 38 | 81 | rejected/h1-guard-on-a0-distinct-j.c |

**The headline finding (and a correction to the s48 record).** The guard-on-a0 form is the
first spelling in 49 sessions to reach target's prologue-cluster ORDER and target's REGISTER
ALLOCATION at the same time. Its block 0 (tmp/grind/func_80045294/s49/v1_disasm.txt):

    sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s0,0x4        (build, s49 v1)
    sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s2,0x4        (target)

a0 -> $s2 and i -> $s0 exactly as the target allocates them; the score-11 H1 callee-save
rotation is gone. s48 recorded the `refs_up 72: 3->4` route as dead; it is not — the route
exists, ra_solver's inverse was right, and it does close the RA leg. What it does NOT do is
change which register of the 72/75 equivalence quantity cse.c uses as the ashift operand:
reg_n_refs is a global.c input, and the canonical-register choice is made much earlier in
`make_regs_eqv` (tools/gcc-2.7.2/cse.c:842-857), where reg 75 displaces reg 72 iff
`uid_cuid[regno_last_uid[75]] > uid_cuid[regno_last_uid[72]]`.

**The three routes to invert that inequality are each now measured dead:**
1. *Extend a0's last reference past loop 2.* No byte-free site exists. The target's 83
   instructions contain exactly three a0 uses, all before loop 2, and $s2 is reused as the
   walking pointer at 0x80045344, so a0 is dead inside and after loop 2. A duplicated
   reference (the Ruling A family probe) requires two paths converging on an identical
   continuation; this function's only convergence points are the epilogue (no a0 reference,
   cannot acquire one byte-free) and the loop-2 body tail (a0 dead). Measured: the duplicate
   survives into the bytes at 84 instructions, score 27 — prerequisite 2 of
   [[duplicated-statement-into-arms]] fails, and the else-arm copy is a dead store, which
   fails prerequisite 1 as well.
2. *Shorten i's last reference* (distinct loop-2 counter). Measured 81 instructions: the
   `lw %gp_rel(D_800A33AC)` the target keeps inside loop 2 at 0x8004538C hoists out.
   Reproduced independently of the guard spelling (s48 got the same 3-instruction deletion
   without it).
3. *Lift a0's reference count.* Measured: fixes the allocation, does not touch the canonical
   register, and costs one instruction (84 vs 83) because `i = a0` has to move into the arm.

**Endgame-lock gates (owner standing ruling 2026-07-27).**
- Gate (a) canonical-asm: `python3 tools/scan_hand_coded.py --single func_80045294` ->
  **tier=LOW, score=0/8**, all eight signals negative (0 multu/mflo pairs, no empty-body
  branches, 7 spills over 83 insns / 11 distinct regs, max load burst 3 in any 8-insn window,
  no high-similarity siblings, no BIOS jumptable pattern, every callee-save use has its $sp
  save, no redundant mask-before-shift). FAILS.
- Gate (b) SOTN-master precedent for the closing construct: there is no closing construct to
  cite — no spelling reaches distance 0. The one sanctioned family reached this session
  ([[duplicated-statement-into-arms]]) failed its own prerequisites BY MEASUREMENT rather
  than by a precedent question, so no precedent could sanction it. FAILS.

HEAD ships `INCLUDE_ASM("asm/funcs", func_80045294);`; `src/text1a_c.c` was restored to HEAD
at the end of the session (`git checkout -- src/text1a_c.c`), tree clean apart from
`metrics/events.jsonl`.

- [s49] Chassis re-check: candidate.c over the INCLUDE_ASM line at src/text1a_c.c:1478 -> sandbox --disable all = score 2, target_insns 83, build_insns 83, rules_dropped 0. Floor unchanged; all s47/s48 conclusions are chassis-valid.

- [s49] Objdump attribution of the 2-insn residual on the candidate chassis: build emits `sw $s1,0x14 ; move $s1,zero ; sll $v1,$s2,4 ; sw $s0,0x10 ; move $s0,$s2` where target emits `sw $s1,0x14 ; move $s1,zero ; sw $s0,0x10 ; move $s0,$s2 ; sll $v1,$s2,4` -- the shift is scheduled one slot early; registers already match on this chassis.

- [s49] Four forms measured this session with nrefs_flow(72) checked via extract.py BEFORE each sandbox run, as owner Ruling A directs: guard-on-a0 (nrefs 4, score 5, 84 insns), guard-on-a0 control with i=a0 first (nrefs 3, no sandbox spent), Ruling A duplication into arms (nrefs 7, score 27, 84 insns), guard-on-a0 + distinct counter j (nrefs 6, score 38, 81 insns).

- [s49] NEW: the guard-on-a0 form reaches target's block-0 instruction ORDER and target's REGISTER ALLOCATION at once (sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s0,0x4; a0 -> $s2, i -> $s0). The RA leg of this function is closed; the s48 record that the refs_up atom was dead is corrected.

- [s49] The residual is now ONE compiler decision: which of two registers holding the identical value the block-0 shift reads. It is settled in make_regs_eqv (tools/gcc-2.7.2/cse.c:842-857) by uid_cuid[regno_last_uid[75]] > uid_cuid[regno_last_uid[72]], not by anything global.c sees.

- [s49] The three routes to invert that inequality are each measured dead this session: extend a0's last reference past loop 2 (no byte-free site -- the duplication probe materializes bytes), shorten i's last reference (deletes 3 instructions the target keeps), lift a0's reference count (fixes allocation only, costs one instruction).

- [s49] Endgame gate (a) canonical-asm: tools/scan_hand_coded.py --single func_80045294 -> tier=LOW, score=0/8, all eight signals negative. FAILS. Consistent with the owner's own 2026-07-20 refusal of canonical asm for this function.

- [s49] Endgame gate (b) SOTN-master precedent: no closing construct exists to cite (no spelling reaches distance 0), and the one sanctioned family reached this session was refused by MEASUREMENT against its own prerequisites, not by a precedent question. FAILS.

- [s49] Exhaustion: 49 sessions over permuter/forensics/rederive/synthesis/solver/escalation; 137,872 permuter iterations over 11 chassis/mode combos; 40 banked rejected forms (4 filed this session); exhaustive single-atom enumerations over both scheduler passes (s47 pass-2 960 atoms, s48 pass-1 192 atoms) with every goal-reaching atom spelling the single intent i-before-v1.

- [s49] src/text1a_c.c restored to HEAD (git checkout --) at the end of the session; working tree clean apart from metrics/events.jsonl. HEAD ships INCLUDE_ASM("asm/funcs", func_80045294);.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=49

## [s50] rederive -- the block-0 residual fully attributed, and an equal-floor chassis with a DISJOINT residual

Chassis re-measured at dispatch: candidate.c applied over
INCLUDE_ASM("asm/funcs", func_80045294); at src/text1a_c.c:1445 gives
`sandbox func_80045294 --disable all` -> **score=2, target_insns=83,
build_insns=83**. Note the FILE chassis has moved since s49: four siblings in
src/text1a_c.c (func_80043BD0 / func_80043C7C / func_80043D34 / func_80043DE0)
are now pure C with POLY_FT3/FT4/GT3/GT4 typedefs, where s49's snapshot still
had register-asm bodies. tmp/grind/func_80045294/s49/text1a_c.orig.c is
therefore STALE -- s50 re-snapshotted HEAD to
tmp/grind/func_80045294/s50/text1a_c.orig.c. The floor is unchanged at 2, so no
banked spelling conclusion is voided by the file drift, but future sessions must
re-snapshot rather than reuse s49's harness.

### The exact residual on candidate.c (objdump, s50)
Only three positions differ, and they are a rotation of the same three insns:

    idx  build                    target
      7  sll  $v1,$s2,4           sw   $s0,0x10($sp)
      8  sw   $s0,0x10($sp)       addu $s0,$s2,$zero
      9  addu $s0,$s2,$zero       sll  $v1,$s2,4

i.e. the target emits the $s0 save/init pair BEFORE the shift; the build emits
the shift first. Everything else, including the entire callee-save allocation,
matches.

### Pass attribution (dumps READ, not guessed): sched.c rank_for_schedule
tmp/grind/func_80045294/s50/dumps_base/text1a_c.sched2 shows block 0 scheduled
BACKWARDS from T-1 to T-18 with **every insn at INSN_PRIORITY 1**. The decision
is a single tie at T-9, where the ready list is `22 14 12 6` (insn 22 = `i = a0`,
insn 14 = the `a0 << 4` shift). Insn 22 is at the head, so 22 is picked first
and therefore EMITTED LAST -- producing the build's `sll ; sw $s0 ; move $s0`.

Reading tools/gcc-2.7.2/sched.c:

  * `priority()` (sched.c:1434-1522) accumulates over **LOG_LINKS**, i.e.
    predecessors, as `priority(pred) + insn_cost(pred) - 1`. It is distance from
    the block START, not to the block end, and the `- 1` makes any chain of
    latency-1 insns come out flat at 1. Only insns with a LOAD predecessor reach
    2 -- in block 0 exactly insn 28 (`s5 = s4 + a1`, fed by the `lw` of s4) and
    insn 31 (the `slt`, fed by the `lw` of D_800A33AC). Neither insn 14 nor
    insn 22 has a load predecessor; both are fed by insn 4, the a0 param copy,
    so both are priority 1. **The priority clause cannot separate them.**
  * The middle clause classifies against `last_scheduled_insn`, which at T-9 is
    insn 199, a `sw` callee-save store. Neither insn 14 nor insn 22 has a
    dependence on a store, so both are class 3. **The class clause cannot
    separate them either.**
  * The decision therefore falls through to sched.c:2461-2463,
    `return INSN_LUID (tmp) - INSN_LUID (tmp2);` -- descending LUID, head
    picked, highest LUID emitted last.

**=> The target's emission order requires the shift's RTL insn to carry a HIGHER
LUID than `i = a0`'s, i.e. the SOURCE must declare i before v1.**

Verified directly rather than inferred: with the i-first order the build's
prologue becomes

    addiu sp,sp,-48 / sw s5,36 / move s5,a0 / sw s3,28 / move s3,a1
    sw s1,20 / move s1,zero / sw s0,16 / move s0,s5 / sll v1,s0,4 / sw ra,40 ...

which is the target's structure insn-for-insn (`sw $s0` and `move $s0` now
precede the `sll`). The only remaining difference is which hard registers the
allocator handed out.

### Why i-first has cost score 11 for 49 sessions: cse -> global.c, quantified
tmp/grind/func_80045294/s50/dumps_A/text1a_c.greg vs dumps_base:

    baseline (v1 first)  ;; 11 regs to allocate: 92 78 75 86 85 79 74 73 72 76 80
                         ;; 72(a0) in 18   76(s4) in 20   80(s5) in 21   <- TARGET
    i-first              ;; 11 regs to allocate: 92 75 76 86 85 79 74 73 77 80 72
                         ;; 72(a0) in 21   77(s4) in 18   80(s5) in 20

a0 falls from 9th to LAST in the allocation order. The cause chain:

  1. cse.c:842-857 `make_regs_eqv (new=i, old=a0)`: i displaces a0 as
     `qty_first_reg` because (a) i's live range leaves the cse EBB (the dump's
     first EBB is `;; Processing block from 2 to 36`, and i is loop 2's counter)
     and (b) `uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]]` -- i
     always dies last, since a0's last reference is the loop-2 preheader's
     `i = a0` while i's is loop 2's own termination test.
  2. `canon_reg` therefore rewrites the later `a0 << 4` into `i << 4`. Confirmed
     in the dump: the i-first .greg shows
     `(insn 17 ... (ashift:SI (reg/v:SI 16 s0) (const_int 4)))` -- the shift
     reads i, not a0. a0's `reg_n_refs` drops 4 -> 3.
  3. global.c `allocno_compare`: priority is
     `floor_log2(n_refs) * n_refs / live_length`, so a0 goes from
     `floor_log2(4)*4 = 8` to `floor_log2(3)*3 = 3` -- a 2.67x collapse, enough
     to drop it below s4 (3 refs, shorter live_length) and s5 (2 refs, much
     shorter live_length). The callee-saves rotate a0 $s2->$s5, s4 $s4->$s2,
     s5 $s5->$s4.

So the 49-session "wall" is a genuine COUPLING, now named on both sides: the
schedule wants i-first (sched.c LUID tiebreak), the allocation wants v1-first
(cse ref count -> global.c priority), and within the declaration-order axis
alone the two requirements are contradictory.

### Exhaustive sweep of the i-first declaration subspace (30/30 measured)
All 30 legal orderings of {sum, count} interleaved into the fixed chain
i < v1 < s4 < s5 were generated and measured
(tmp/grind/func_80045294/s50/v/P00..P29, results in
tmp/grind/func_80045294/s50/sweep.txt):

    sum before i  (6 forms, P00-P05):   score=11  insns=83  (all)
    i first       (24 forms, P06-P29):  score=14  insns=83  (all)

No declaration order anywhere in the i-first subspace goes below 11. The
subspace is closed by measurement, not by argument.

### THE NEW RESULT: an equal-floor chassis whose residual is DISJOINT
The coupling is breakable on the ALLOCATION side. The loop-2 preheader is a
**fresh cse EBB** (the dump's `;; Processing block from 70 to 109`) in which i
has been clobbered by loop 1, so an `a0 << 4` placed BEFORE that block's
`i = a0` is NOT canonicalized to i and pays a0's fourth reference back. Form
I_a0shift_before_i = i-first block 0 + loop-2 preheader respelled
`v1 = a0 << 4; i = a0;` measures **score=2, build_insns=83**, and its residual is

    idx  9  build `sll $v1,$s0,4`   target `sll $v1,$s2,4`   (block 0)
    idx 41  build `sll $v1,$s2,4`   target `sll $v1,$s0,4`   (loop-2 preheader)

and **nothing else** -- the prologue sw/move interleave, the
`sw $s0 / move $s0,$s2 / sll` ordering that candidate.c gets wrong, the complete
callee-save allocation (a0->$s2, a1->$s3, sum->$s1, i->$s0, s4->$s4, s5->$s5)
and every stack slot are byte-exact. Banked as
memory/grind/func_80045294/rejected/s50-i-first-a0ref-in-loop2-preheader.c.

**candidate.c and this form are exactly complementary**: candidate.c has both
`sll` operands RIGHT and the block-0 schedule WRONG; this form has the schedule
RIGHT and both operands WRONG. Both sit at 2. This is the first time the floor
of 2 has been reached from the i-first side at all, and it converts the residual
from a coupled 3-insn scheduling rotation into two isolated register-operand
choices.

Companion measurements from the same batch (block-0 head fixed, loop-2 preheader
varied) -- tmp/grind/func_80045294/s50/v/:

    B_shiftfirst        v1-first blk0, loop-2 `i=a0; v1=i<<4`   score=2   insns=83  (== candidate.c)
    B_a0shift_before_i  v1-first blk0, loop-2 `v1=a0<<4; i=a0`  score=3   insns=83
    I_shiftfirst        i-first  blk0, loop-2 `i=a0; v1=i<<4`   score=11  insns=83
    I_a0shift_before_i  i-first  blk0, loop-2 `v1=a0<<4; i=a0`  score=2   insns=83  <- new chassis
    B_a0shift_in_guard  loop-2 guard on a0                      score=27  insns=84
    I_a0shift_in_guard  loop-2 guard on a0                      score=5   insns=84

The `_in_guard` pair costs an instruction (84) because testing
`a0 < D_800A33AC` before `i = a0` emits the `slt` on $s2 and then still needs the
`move`, so a0's fourth reference cannot be bought from the loop-2 guard for free.

### What this leaves for the next session
On the new chassis the whole function reduces to ONE question: can the block-0
`a0 << 4` read a0 while sitting after `i = a0`? Under cse.c:842-857 that needs
`uid_cuid[regno_last_uid[i]] <= uid_cuid[regno_last_uid[a0]]`, i.e. a0 must be
referenced at or after i's last reference. s48 already measured the obvious way
to shorten i (a distinct loop-2 counter j): 80 insns, score 37 on BOTH chassis.
The untried direction is to LENGTHEN a0 rather than shorten i, or to shrink the
first cse EBB so that clause (a) of make_regs_eqv fails.

- [s50] Chassis re-measured at dispatch: candidate.c over src/text1a_c.c:1445 gives sandbox score=2, target_insns=83, build_insns=83. Floor unchanged at 2.

- [s50] The FILE chassis moved since s49: func_80043BD0 / func_80043C7C / func_80043D34 / func_80043DE0 in src/text1a_c.c are now pure C with POLY_FT3/FT4/GT3/GT4 typedefs where s49's snapshot still had register-asm bodies. tmp/grind/func_80045294/s49/text1a_c.orig.c is STALE and must not be reused as an apply harness; s50 re-snapshotted HEAD to tmp/grind/func_80045294/s50/text1a_c.orig.c.

- [s50] candidate.c's residual is exactly three positions and is a rotation of the same three insns: build emits `sll $v1,$s2,4 / sw $s0,0x10($sp) / addu $s0,$s2,$zero`, target emits `sw $s0,0x10($sp) / addu $s0,$s2,$zero / sll $v1,$s2,4`. Everything else including the whole callee-save allocation matches.

- [s50] sched.c priority() measures distance from the block START (`priority(pred) + insn_cost(pred) - 1` over LOG_LINKS), not to the block end. In block 0 this leaves every insn at priority 1 except the two fed by an `lw` (insn 28 `s5 = s4 + a1` and insn 31 the `slt`), which reach 2. This corrects any earlier assumption that the block-0 order is a critical-path/priority effect: it is a pure tiebreak.

- [s50] The block-0 order is decided by a single tie at T-9 of a BACKWARDS schedule, where the ready list is `22 14 12 6`; both contenders are priority 1 and class 3, so sched.c:2461-2463 `INSN_LUID (tmp) - INSN_LUID (tmp2)` decides. Descending LUID, head picked, highest LUID emitted last.

- [s50] cse.c:842-857 make_regs_eqv makes i the qty_first_reg on `i = a0` because i leaves the first cse EBB (`Processing block from 2 to 36`) and uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]] -- i is loop 2's counter so it always dies last. canon_reg then rewrites a later `a0 << 4` to `i << 4`, dropping a0's reg_n_refs from 4 to 3.

- [s50] global.c allocno_compare priority is floor_log2(n_refs)*n_refs/live_length, so a0's 4->3 ref loss collapses its priority from 8 to 3. Measured in the dumps: baseline allocation order `92 78 75 86 85 79 74 73 72 76 80` with 72(a0) in $18, 76(s4) in $20, 80(s5) in $21 (target); i-first order `92 75 76 86 85 79 74 73 77 80 72` with 72(a0) in $21, 77(s4) in $18, 80(s5) in $20.

- [s50] Exhaustive: all 30 legal block-0 declaration orders with i before v1 measured at build_insns=83 and score 11 (sum before i) or 14 (i first). None below 11. The declaration-order axis is closed by measurement.

- [s50] NEW equal-floor chassis: i-first block 0 plus loop-2 preheader respelled `v1 = a0 << 4; i = a0;` gives score=2 / 83 with a residual of exactly two `sll` operands (idx 9 and idx 41, swapped relative to the target) and a byte-exact prologue schedule, callee-save allocation and stack frame. Banked at memory/grind/func_80045294/rejected/s50-i-first-a0ref-in-loop2-preheader.c.

- [s50] src/text1a_c.c was restored to HEAD at end of session; git status shows only the two ledger files, the new rejected/ form, and metrics/events.jsonl modified. candidate.c is unchanged and remains the best form (tied at 2 with the new chassis).

## s51 (rederive) -- 2026-09-03

### Chassis re-measurement (kill re-audit)
Both banked score-2 forms re-measured on today's HEAD with `sandbox
func_80045294 --disable all`:
  memory/grind/func_80045294/candidate.c                       score=2  insns=83
  rejected/s50-i-first-a0ref-in-loop2-preheader.c              score=2  insns=83
The chassis is unchanged from s50; every s50 conclusion is still chassis-valid.
No FAKE construct is present in either form, so `fake_ablate.py` has nothing to
ablate; the ablation leg of the re-audit is vacuous for this function. The
closest instance kill (s50-H3, guard-on-a0 at 84 insns) was not re-run because
this session measured a strictly stronger statement about the same lever family
(see the jump.c:577 result below), which covers it.

### The distinct-loop-2-counter kill re-attributed (s48's 3 deleted insns)
`rejected/h1-separate-second-loop-counter.c` re-measured: score=37 insns=80,
reproducing s48 exactly. This session disassembled the sandbox object and
diffed it against asm/funcs/func_80045294.s (tmp/grind/func_80045294/s51/
build_disasm.txt + cmp.py). s48's attribution ("lets loop 2's `lw D_800A33AC`
hoist out of the loop") is WRONG. What actually happens: with two separate
counter pseudos, loop 1's counter no longer has to survive the two calls, so
GCC leaves it in the incoming `$a0` (`addiu a0,a0,1` at build idx 22) and needs
no `move $s0,$s2` copy in block 0 at all; a0 is copied once to `$s1`
(`move s1,a0`, idx 2), the count lands in `$a1`, and one callee-save
(`$s5`) drops out of the frame entirely -- the frame shrinks from 0x30 to 0x28.
The 3 missing instructions are the deleted block-0 copy plus the `$s5`
save/restore pair, not a hoisted load. This matters for the model: the SINGLE
shared counter is load-bearing precisely because one pseudo spanning block 0 to
loop 2 is what forces a callee-save allocation and the `move $s0,$s2` copy the
target has.

### sched1 closes the scheduler side (new)
The instrumented cc1 (tools/gcc-2.7.2/cc1) was run with BB2_PRIO_DEBUG and
BB2_RANK_DEBUG over the candidate.c chassis; dumps in
tmp/grind/func_80045294/s51/dumps_cand/ (sched_debug.txt = raw stderr,
9336 PRIODBG + 1873 RANKDBG lines).
  * sched2 block 0 ("from 197 to 32") reproduces the s50 trace: all block-0
    insns priority 1 except 28 and 31 (priority 2, each fed by an `lw`), the
    deciding tie at T-9 is `22 (1) 14 (1) 12 (1) 6 (1), now 22 14 12 6` with
    last_scheduled_insn = 199 (`sw $ra`). Emission order first->last is
    197,207,4,205,6,209,12,14,211,22,199,201,203,19,25,28,31,32; the target
    wants ...12,211,22,14,199..., i.e. exactly the 2-insn residual.
  * sched1 block 0 ("from 4 to 32") -- NEW this session -- has the identical
    structure: at T-6 the ready list is `22 (1) 14 (1) 12 (1)` sorting to
    `22 14 12`, so sched1 also emits the shift BEFORE the copy and hands sched2
    the source order unchanged.
  This closes the one route left open by s50: sched1 cannot pre-reorder the
  pair so that sched2's re-assigned LUIDs favour the shift.
  Raising the shift's INSN_PRIORITY to 2 is structurally unavailable --
  sched.c:1497's `priority(pred) + insn_cost(pred) - 1` is flat over latency-1
  chains, so priority 2 requires a LOAD predecessor, and `a0 << 4` reads a
  parameter. Lowering `i = a0`'s class below 3 (rank_for_schedule,
  sched.c:2429-2441) requires it to be a cost>1 predecessor of the
  last_scheduled `sw`, which a reg-reg move cannot be.

### jump.c:577 forecloses every DEAD-reference spelling of the lever (new)
The s50 frontier named "LENGTHEN a0's last use past i's" as the untried
direction on the cse side. Measured this session at four placements of a dead
`dead = a0;` on the i-first chassis -- after block 0's shift, in the loop-2
preheader after both calls, inside loop 2's body tail, and as the function's
last statement -- every one scores 11 at 83 instructions, byte-identical to the
no-dead-reference control (tmp/grind/func_80045294/s51/C_ifirst_plain.c).
The dumps for the last-statement variant (dumps_B/) show the cause:
  * .rtl (pre-cse) carries it: `(insn 191 188 193 (set (reg/v:SI 74)
    (reg/v:SI 72)))`, the function's final insn, reg 74 = the dead local,
    reg 72 = a0.
  * .jump has `(insn 188 186 193 ...)` -- insn 191 is already deleted.
  * .cse still shows `(insn 18 ... (set (reg/v:SI 77) (ashift:SI
    (reg/v:SI 76) (const_int 4))))`, i.e. block 0's shift rewritten from a0
    (reg 72) to i (reg 76) exactly as in the control.
The deleter is tools/gcc-2.7.2/jump.c:568-584: when jump_optimize is called
with `after_regscan` (which is how toplev calls it before cse) it deletes any
insn whose SET_DEST is a pseudo satisfying
`regno_first_uid[dest] == INSN_UID (insn) && regno_last_note_uid[dest] ==
INSN_UID (insn)` -- every set-once/never-read pseudo. So a dead a0 reference is
gone before the pass that would read regno_last_uid[a0] ever runs.
The complement is measured: with a LIVE destination (`i = a0;` as the tail
statement) the predicate at jump.c:577 does not fire, the store survives into
the bytes, and the build is 84 instructions / score 27
(rejected/s51-deadstore-i-eq-a0-tail.c). jump.c:577 therefore partitions tail
a0 references into exactly two cases -- dead destination (deleted pre-cse, zero
effect) and live destination (costs an instruction) -- and there is no third.

### Where the function stands after s51
The residual is one coupled decision with both sides now carrying a file:line
predicate and a measurement:
  (A) block 0 must emit `move $s0,$s2` before `sll $v1,$s2,4`
      <=> the SOURCE must write `i = a0` before `v1 = a0 << 4`
      (sched.c:2461-2463; both scheduler passes measured this session).
  (B) block 0's shift must read a0's register
      <=> cse must keep reg 72 as qty_first_reg (cse.c:855).
Under (A) the copy precedes the shift, and reg 75/76 (i) wins qty_first_reg
because its last reference (loop 2) postdates a0's. The three routes to invert
that are now each measured dead with a named predicate: shorten i's last
reference (deletes the block-0 copy and a callee-save pair -- 80 insns, s48 and
re-attributed here), lift a0's reg_n_refs (fixes the ALLOCATION only, s49 v1,
and costs an instruction), and lengthen a0's last reference (jump.c:577, this
session). The remaining unmeasured direction is clause (1) of cse.c:849-853 --
shrinking block 0's cse EBB (`;; Processing block from 2 to 37` in
dumps_B/text1a_c.cse) so that i's live range no longer leaves it -- which is
what the frontier below points at.

- [s51] Chassis re-audit: memory/grind/func_80045294/candidate.c and rejected/s50-i-first-a0ref-in-loop2-preheader.c BOTH re-measure score=2 build_insns=83 on today's HEAD, so every s50 conclusion is still chassis-valid. Neither form carries a FAKE construct, so the fake_ablate leg of the kill re-audit is vacuous for this function.

- [s51] The instrumented cc1 is tools/gcc-2.7.2/cc1, but engine.buildconfig.CC1 points at tools/gcc-2.7.2/build/cc1 -- a dump script that uses B.CC1 silently gets the UNinstrumented compiler and produces no BB2_*_DEBUG output. tmp/grind/func_80045294/s51/dump.sh hardcodes the instrumented path.

- [s51] sched1 and sched2 make the same decision in block 0: sched1 T-6 `ready list: 22 (1) 14 (1) 12 (1), now 22 14 12`; sched2 T-9 `ready list: 22 (1) 14 (1) 12 (1) 6 (1), now 22 14 12 6`. Both pick insn 22 (`i = a0`) over insn 14 (the shift) on the INSN_LUID tiebreak alone, so the emitted order of the pair is the source order at every scheduling opportunity.

- [s51] Insn 14's INSN_PRIORITY cannot exceed 1: sched.c:1497 subtracts one from each predecessor contribution, so a chain of latency-1 insns is flat, and the only way to reach 2 is a predecessor with insn_cost 2 (a load). `v1 = a0 << 4` reads a parameter, so no load can feed it without adding an instruction.

- [s51] jump.c:568-584 deletes every set-once/never-read pseudo BEFORE cse runs (predicate at jump.c:577). Verified against dumps: the dead store is present in .rtl as insn 191 and absent in .jump, and .cse still canonicalises block 0's shift from reg 72 (a0) to reg 76 (i).

- [s51] The distinct-loop-2-counter form's 80-instruction shape is an allocation collapse (loop 1's counter stays in $a0, block 0's copy vanishes, the $s5 save/restore pair drops, frame 0x30 -> 0x28), not a hoisted gp-rel load -- correcting the s48 record.

- [s51] cse EBB bounds on the i-first chassis (dumps_B/text1a_c.cse): `;; Processing block from 2 to 37` (block 0 + guard), `from 39 to 65` (loop 1), `from 71 to 110` (loop-2 preheader). Block 0's EBB ends at 37, so clause (1) of cse.c:849-853 is satisfied for i via `uid_cuid[regno_last_uid[i]] > cse_basic_block_end` and NOT via the first_uid disjunct -- while the preheader EBB satisfies it via the OTHER disjunct.

## s52 (structural) -- 2026-09-03

### THE FLOOR MOVED: 2 -> 1

`memory/grind/func_80045294/candidate.c` is replaced this session. The new form
(`tmp/grind/func_80045294/s52/F_a0ptr_ifirst.c`) measures
**score=1, target_insns=83, build_insns=83** under
`sandbox func_80045294 --disable all` on today's HEAD, and its residual is a
SINGLE instruction:

    idx 9   target  sll $v1, $s2, 4        build  sll $v1, $s0, 4

Every other instruction in the function -- the whole prologue interleave
(`sw $s0,0x10($sp)` / `addu $s0,$s2,$zero` / `sll` in the TARGET's order), the
complete callee-save allocation (a0->$s2, a1->$s3, sum->$s1, i->$s0, s4->$s4,
s5->$s5, ptr->$s2, idx->$s1), the stack frame, both loops, the delay slots and
the tail -- is byte-exact. Full disassembly comparison in
tmp/grind/func_80045294/s52/build_disasm.txt (+ cmp.py).

### What changed: a0 is REUSED as loop 2's pointer

The previous candidate declared a separate `s32 *ptr` for loop 2. The new form
assigns loop 2's walking pointer back into the parameter `a0`:

    a0 = (s32)((u8 *)&D_800EED14 + v1);
    do { *(s32 *)a0 += a1; ... a0 += 0x10; idx += 0x10; i += 1; }
    while (i < D_800A33AC);

This is a REAL value that materialises in the target's bytes: the target itself
reuses $s2 for a0 and then for the loop-2 pointer (`addu $s2,$v1,$v0` at
0x80045344 overwrites a0's register), so the register-level evidence for the
reuse is in the shipped code, not merely in a score.

The reuse buys the *block-0 schedule*, which 51 sessions of declaration-order
and scheduler work could not: extending a0's pseudo live range through loop 2
changes block 0's allocation pressure so that the i-first source order
(`i = a0;` then `v1 = a0 << 4;`) now emits `sw $s0 / addu $s0,$s2 / sll` in the
TARGET's order. On the pre-s52 chassis the same i-first source order scored 11.
The s51 finding still holds and now works FOR us: the emitted order of the
copy/shift pair is the source order at both scheduler passes, so writing the
copy first is what puts it first in the bytes.

Measured pairs on today's HEAD (loop-2 pointer spelling x block-0 order):

    separate `ptr` local, v1-first block 0 (old candidate.c)   score=2  insns=83
    separate `ptr` local, i-first block 0                      score=11 insns=83
    a0 reused as ptr,     v1-first block 0 (H_a0ptr_vfirst)    score=2  insns=83
    a0 reused as ptr,     i-first block 0 (NEW candidate.c)    score=1  insns=83

A second equal-floor spelling exists: moving `count = D_800A33AC` ahead of the
shift in block 0 (M_countfirst) also measures score=1 / 83, so the last residual
is insensitive to that declaration-order axis.

### The one remaining residual is a cse canonicalisation, fully attributed

Dumps for the new chassis: tmp/grind/func_80045294/s52/dumps_F/ (extracted
per-function in F.rtl.fn / F.cse.fn).

    .rtl   (insn 15 ... (set (reg/v:SI 75) (reg/v:SI 72)))            i = a0
           (insn 17 ... (set (reg/v:SI 76) (ashift (reg/v:SI 72) 4)))  v1 = a0<<4
    .cse   (insn 17 ... (set (reg/v:SI 76) (ashift (reg/v:SI 75) 4)))

i.e. cse rewrites the shift's operand from reg 72 (a0, allocated $s2) to reg 75
(i, allocated $s0). That single rewrite IS the last diff. The gate is
make_regs_eqv at tools/gcc-2.7.2/cse.c:842-857: when `i = a0` is walked, i
becomes qty_first_reg because
  (1) uid_cuid[regno_last_uid[i]] > cse_basic_block_end   (i lives past the EBB), and
  (2) uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]].
canon_reg then rewrites every later reference in the quantity to i.

### Why clause (2) resisted inversion in this chassis (measured, 6 spellings)

a0's last reference must be at or after i's last reference. i is loop 2's
counter, so i's last reference is loop 2's exit test, which GCC emits as the
LAST insn of the loop body -- no source statement can be placed after it inside
a do/while. Every way of getting a0 referenced later was measured this session:

    a0 += 0x10 moved to the last body statement (K)          score=5  insns=84
    exit test hoisted into `more` before a0's inc (G)        score=27 insns=85
    for(;;) + break so a0's inc follows the test (J)         score=29 insns=85
    loop 2 as a top-tested while, guard kept (T)             score=5  insns=84
    loop 2 as a top-tested while, no guard (S)               score=12 insns=85
    s50's guard-on-a0, re-audited on this chassis (N)        score=5  insns=84

All six cost at least one instruction; none reaches 83. The only zero-cost site
for a later a0 reference would be after loop 2 (the `D_800A33A0 += a1` tail),
and that is unavailable because a0's pseudo is already allocated $s2 for the
loop-2 pointer -- a tail use would print $s2 where the target prints $v0.

### Why clause (1) resisted too (measured, 4 spellings)

Failing clause (1) needs i's whole live range inside block 0's cse EBB. i is a
loop counter, so it always leaves the EBB. Making the block-0 copy's dest
short-lived does fail clause (1) -- and the shift then keeps a0 -- but the
short-lived pseudo is dead-eliminated and the SURVIVING copy lands after the
shift, which is the old score-2 rotation again:

    t = a0; v1 = a0<<4; ...; i = t;   (C)   score=2  insns=83  <- rotation residual
    t = a0; i = t; v1 = a0<<4;        (D)   score=11 insns=83  <- i crowned at `i = t`
    b = a0; i = b; v1 = b<<4; b reused as ptr (L)  score=6  insns=83
    loop-1 offset left to loop.c strength reduction (A/B)  score=11 insns=83

The strength-reduction route (frontier item 3 from s51) is now measured and
attributed: loop.c DOES create a preheader giv chain that reads a0 --
tmp/grind/func_80045294/s52/Agiv.loop.fn shows
`(insn 192 (set (reg 109) (reg/v:SI 72)))` / `(insn 193 (set (reg 110)
(ashift (reg 109) 4)))` with `REG_EQUAL (mult (reg/v:SI 72) 16)` -- but it is
emitted into loop 1's PREHEADER block, after the guard branch, whereas the
target's shift sits in block 0 feeding the `D_800EED14` load before the branch.
And block 0's own shift is already canonicalised to i by cse1 before loop.c
runs, so the giv never becomes the surviving block-0 shift.

### Separate-counter forms still collapse on this chassis

Giving loop 2 its own counter DOES fail clause (2) (a0 then outlives loop 1's
counter), but it also lets loop 1's counter stay in the incoming $a0, deleting
block 0's copy entirely -- the s48/s51 collapse, reproduced here on the
a0-as-pointer chassis:

    a0 reused as ptr + distinct loop-2 counter j (I)  score=30 insns=80
    a0 used as loop 2's counter, ptr separate    (E)  score=34 insns=80

### Kill re-audit (mandated)

The closest prior instance kill, s50's "buy a0's fourth reference from the
loop-2 guard" (`if (a0 < D_800A33AC)` ahead of `i = a0`), was re-measured on the
NEW chassis as N_guard_on_a0: score=5, insns=84. The kill stands and its cause
is unchanged (the a0 compare emits its own `slt` on $s2 and the `move` is still
needed). Neither the old nor the new candidate carries a FAKE construct, so the
`fake_ablate.py` leg is vacuous for this function, as recorded in s51.

### Vetting note for whoever submits this

The new candidate's only non-obvious construct is the reuse of the parameter
`a0` as loop 2's walking pointer. It is NOT a coercion: the value is real, it is
loaded from and stored through on every iteration, and it materialises in the
target's own bytes ($s2 carries both a0 and the pointer). It nonetheless sits in
the frozen "variable reuse for codegen control" family, so a candidate-ready
session must fill in `self_vet.md` with that family, its scope sentence, and a
precedent before submitting.

- [s52] FLOOR 2 -> 1. New candidate.c = i-first block 0 (`i = a0;` before `v1 = a0 << 4;`) plus loop 2's walking pointer assigned into the parameter `a0` instead of a separate `s32 *ptr`. sandbox score=1, target_insns=83, build_insns=83.

- [s52] The score-1 residual is exactly one instruction: idx 9, target `sll $v1,$s2,4` vs build `sll $v1,$s0,4`. Prologue interleave, callee-save allocation, frame, both loops, delay slots and tail are byte-exact.

- [s52] Reusing a0 as loop 2's pointer is corroborated by the TARGET's own register usage: 0x80045344 `addu $s2,$v1,$v0` overwrites a0's register with the loop-2 pointer, so $s2 genuinely carries both values in the shipped code.

- [s52] Mechanism of the last diff, read off tmp/grind/func_80045294/s52/dumps_F/: .rtl insn 17 is `(ashift (reg/v:SI 72) 4)` (a0); .cse insn 17 is `(ashift (reg/v:SI 75) 4)` (i). cse.c:842-857 make_regs_eqv crowns i as qty_first_reg on the `i = a0` copy and canon_reg rewrites the shift.

- [s52] Six zero-cost attempts to make a0's last reference postdate i's (K,G,J,T,S,N) all measure 84 or 85 instructions. The loop-2 exit test is the last insn of the body, so no source statement tried this session could be ordered after it inside a do/while.

- [s52] loop.c's strength reduction DOES emit a preheader giv chain reading a0 (`Agiv.loop.fn` insns 192/193, REG_EQUAL `mult (reg 72) 16`), but into loop 1's preheader block after the guard branch, not into block 0 where the target's shift feeds the D_800EED14 load. Frontier item 3 from s51 is answered: post-cse insn creation exists but lands in the wrong block.

- [s52] On the new chassis cse's first EBB for this function is printed as `;; Processing block from 2 to 199` with path retries (`2 to 61`, `2 to 51`, `2 to 33`), i.e. cse re-walks block 0 several times with different branch paths. Any EBB-boundary attack on clause (1) must therefore defeat EVERY retry pass, not just the first.

- [s52] FLOOR 2 -> 1. New candidate.c = i-first block 0 (`i = a0;` before `v1 = a0 << 4;`) plus loop 2's walking pointer assigned into the parameter `a0` instead of a separate `s32 *ptr`. sandbox score=1, target_insns=83, build_insns=83 on today's HEAD.

- [s52] The score-1 residual is exactly one instruction: idx 9, target `sll $v1,$s2,4` vs build `sll $v1,$s0,4`. Everything else in the function matches byte for byte, including the prologue sw/move/sll interleave that 51 previous sessions could not reproduce together with the correct shift operand.

- [s52] Reusing a0 as loop 2's pointer is corroborated by the target's own register usage: `addu $s2,$v1,$v0` at 0x80045344 overwrites a0's register with the loop-2 pointer, so $s2 genuinely carries both values in the shipped code. The value is real (loaded from, stored through, incremented), not a coercion.

- [s52] Mechanism of the last diff read off tmp/grind/func_80045294/s52/dumps_F/: .rtl insn 17 is `(ashift (reg/v:SI 72) 4)`; .cse insn 17 is `(ashift (reg/v:SI 75) 4)`. cse.c:842-857 make_regs_eqv crowns i as qty_first_reg on the `i = a0` copy and canon_reg rewrites the shift.

- [s52] Six zero-cost attempts to make a0's last reference postdate i's (a0-increment-last, cond-hoist, for/break, top-tested while with and without guard, guard-on-a0) all measure 84 or 85 instructions, never 83.

- [s52] Kill re-audit as mandated: s50's guard-on-a0 instance kill re-measured on the new chassis as N_guard_on_a0 = score 5 / 84 insns; the kill stands and its cause is unchanged. Neither candidate carries a FAKE construct, so the fake_ablate leg remains vacuous for this function.

- [s52] On the new chassis cse's first EBB for this function prints as `;; Processing block from 2 to 199` with path retries (`2 to 61`, `2 to 51`, `2 to 33`), i.e. cse re-walks block 0 several times with different branch paths -- any EBB-boundary attack on clause (1) must defeat every retry pass.

- [s52] src/text1a_c.c was restored to HEAD at end of session; the tree carries only the two ledger files, the new/updated candidate.c, sixteen new rejected/ forms, and metrics/events.jsonl.

## [s53] structural — block 0's residual is a two-valued cse canonical, and BOTH values are now reachable

**Chassis re-measured first.** `memory/grind/func_80045294/candidate.c` (a0-as-pointer,
i-first) re-measures **score=1, build_insns=83, target_insns=83** on HEAD 2026-09-03.
Its single differing instruction is idx 9: target `sll $v1,$s2,4` (a0's register)
vs build `sll $v1,$s0,4` (i's register).

**The block-0 constraint, stated exactly.** Block 0 contains three insns that
touch the a0 quantity, in the target's emission order:
  idx 8  `addu $s0,$s2,$zero`   the copy   i = a0
  idx 9  `sll  $v1,$s2,4`       the shift  reads **a0's register**
  idx 18 `slt  $v0,$s0,$a0`     the loop-1 entry guard, reads **i's register**
cse assigns ONE canonical register (`qty_first_reg`) per quantity per extended
basic block, and `canon_reg` rewrites every later use to it, so under a single
crown state the shift and the guard must print the SAME register. The target
prints different ones. Documented insn-by-insn this session in
`tmp/grind/func_80045294/s53/dumps_A/text1a_c.{rtl,cse}.fn`:
  .rtl insn 15 `(set (reg 75) (reg 72))`      copy      (reg 72 = a0, reg 75 = i)
  .rtl insn 17 `(ashift (reg 72) 4)`  -> .cse `(ashift (reg 72) 4)`   [crown failed]
  .rtl insn 31 `(lt (reg 75) (reg 79))` -> .cse `(lt (reg 72) (reg 79))` [i rewritten to a0]
and the mirror image on candidate.c (s52 dumps): insn 17 rewritten 72 -> 75, insn 31 left at 75.

**New this session: the crown state is now a controllable knob.** Reusing loop 1's
counter as loop 2's byte offset (`A_reuse_i_as_idx.c`) puts i's last mention
(`i += 0x10`) BEFORE a0's last mention (`a0 += 0x10`) inside loop 2's body, so
cse.c:855 clause (2) fails and a0 stays canonical. Result: block 0's first ten
instructions, including `sw $s0,0x10($sp) / addu $s0,$s2,$zero / sll $v1,$s2,4`,
are byte-exact against the target — the FIRST time the copy-first emission order
and the correct shift operand have coexisted. The price is the other side of the
same knob: the guard becomes `slt $v0,$s2,$a0`, and loop 2's two callee-saves swap
(build idx=$s0 / counter=$s1 against target idx=$s1 / counter=$s0). score=11/83.

**Three-corner map of block 0 on the a0-as-pointer chassis (all 83 insns):**
| form | RTL order | crown | shift operand | guard operand | emission order | score |
|---|---|---|---|---|---|---|
| candidate.c (i-first) | copy, shift | i wins | `$s0` WRONG | `$s0` right | sw/move/sll right | **1** |
| A_reuse_i_as_idx (i-first + offset reuse) | copy, shift | a0 wins | `$s2` right | `$s2` WRONG | sw/move/sll right | 11 |
| H_a0ptr_vfirst (v1-first) | shift, copy | i wins | `$s2` right | `$s0` right | **sll/sw/move WRONG** | 2 |
H is the only form with both operands right; its whole residual is that the shift
is emitted two slots early (before `sw $s0,0x10($sp)` and the copy) because
sched2's rank falls through to the LUID compare and preserves the source order.
The instrumented-cc1 trace for H on this chassis
(`tmp/grind/func_80045294/s53/dumps_H/sched_debug.txt`) shows block 0's rank
comparisons returning `val=0` (`RANKDBG last=23 y=15 cls=3 x=12 cls2=3 val=0`),
i.e. the tie is still class-3/class-3 with flat priority 1, as s51 measured on the
old chassis.

**Negative results banked (all measured with `sandbox func_80045294 --disable all`):**
- `a0 = a0;` between the shift and the guard, meant to invalidate reg 72's quantity
  so `delete_reg_equiv` promotes i: 1/83, byte-identical to its decl-split control
  (K0 = 1/83). expand never emits the store, so no pass can see it. Also measured
  in expression position (`v1 = (a0 = a0) << 4;`): 1/83.
- Loop 1 restructured four ways on this chassis — top-tested `while` (G1), guarded
  do-while with the offset left to loop.c (G2), `for` (G3), and candidate.c's
  explicit offset with a top-tested while (G4): every one is 1/83 with the identical
  idx-9 residual. `duplicate_loop_exit_test` is reached from the FIRST
  `jump_optimize (insns, 0, 0, 1)` call (toplev.c:2827, jump.c:626), i.e. before cse,
  so no loop spelling can make the entry guard a post-cse insn.
- Loop-1 counter distinct from loop-2's counter with `sum` reused as the loop-2
  offset (N): 31/80 — the s48/s51 allocation collapse (the counter lands in the
  incoming `$a0`, block 0's copy and the `$s5` save/restore pair disappear). The
  block-0 copy's destination must itself be live across DrawSync/func_800520B8.
- C_sepctr (plain distinct counters on this chassis) re-measured: 31/80, same collapse.

- [s53] candidate.c re-measured on HEAD 2026-09-03: score=1, build_insns=83, target_insns=83; the single differing instruction is idx 9, target sll $v1,$s2,4 vs build sll $v1,$s0,4.

- [s53] Block 0 contains exactly three insns touching the a0 quantity: the copy addu $s0,$s2,$zero (idx 8), the shift sll $v1,$s2,4 (idx 9, reads a0's register) and the loop-1 entry guard slt $v0,$s0,$a0 (idx 18, reads i's register). cse's one-canonical-per-EBB rule means the last two cannot differ under a copy-before-shift RTL order.

- [s53] Three-corner map, all at build_insns 83 on the a0-as-pointer chassis: candidate.c (i-first) = shift wrong / guard right / order right, score 1; A_reuse_i_as_idx (i-first + offset reuse) = shift right / guard wrong / order right, score 11; H_a0ptr_vfirst (v1-first) = both operands right / order wrong, score 2.

- [s53] H_a0ptr_vfirst's residual is exactly that the shift is emitted two slots early: build sll v1,s2,4 / sw s0,16(sp) / move s0,s2 against target sw $s0,0x10($sp) / addu $s0,$s2,$zero / sll $v1,$s2,4.

- [s53] The instrumented cc1 trace for H on this chassis still shows block 0's rank comparisons tying at class 3 with flat priority 1 (RANKDBG last=23 y=15 cls=3 x=12 cls2=3 val=0 in tmp/grind/func_80045294/s53/dumps_H/sched_debug.txt), so the s51 LUID tiebreak survives the chassis change.

- [s53] jump.c's duplicate_loop_exit_test is reached from the first jump_optimize call (toplev.c:2827, after_regscan = 1), which runs before cse_main (toplev.c:2865) - no loop-1 spelling can defer the entry guard past cse.

- [s53] GCC 2.7.2 emits no RTL at all for a0 = a0, so the s51 dead-store deletion path (jump.c:577) is not even reached; the self-assignment is inert one pass earlier than the dead store was.

## [s54] synthesis — the block-0 residual reduced to a two-registers-one-quantity impossibility, and the crown-invalidation lever closed at cse.c:6871-6902

**Chassis re-measured first (mandated).** `memory/grind/func_80045294/candidate.c`
(a0-as-pointer, i-first) re-measures **score=1, build_insns=83, target_insns=83** on
HEAD 2026-09-03. Its one differing instruction is still idx 9: target `sll $v1,$s2,4`
(a0's register) vs build `sll $v1,$s0,4` (i's register). Fresh dumps for the chassis in
`tmp/grind/func_80045294/s54/dumps_BASE/` (+ per-function extracts `BASE.rtl.fn`,
`BASE.cse.fn`, `BASE.cse2.fn`, extractor `fn.py`).

### The residual restated as a closed-form constraint (this session's main result)

Block 0 contains exactly three insns in the a0 quantity, and cse prints `qty_first_reg`
(the "crown") for EVERY canonicalised reference inside one extended basic block:

    RTL insn 15   (set (reg/v:SI 75) (reg/v:SI 72))          i = a0      -> target addu $s0,$s2,$zero
    RTL insn 17   (set (reg 76) (ashift (reg/v:SI ?) 4))     v1 = a0<<4  -> target sll  $v1,$s2,4   ($s2 = a0)
    RTL insn 31   (set (reg 81) (lt (reg/v:SI ?) (reg 79)))  i < count   -> target slt  $v0,$s0,$a0 ($s0 = i)

The target therefore needs TWO DIFFERENT registers for two references to the SAME cse
quantity inside ONE basic block. Measured EBB bounds on this chassis (header of
`BASE.cse2.fn`): `;; Processing block from 2 to 36` = block 0 alone; loop 1 is `38 to 67`,
the loop-2 preheader `70 to 109`, loop 2 `111 to 173` (plus retries 111-142 / 144-173).
cse EBBs are unions of whole basic blocks, so the shift and the guard are unavoidably in
the same EBB and receive the same crown. That leaves exactly three escape routes, and two
are now closed:

  (R1) **flip the crown, accept a wrong guard.** This is form A (`A_reuse_i_as_idx`,
       11/83) and it is strictly worse than the score-1 corner: the crown test is a global
       `regno_last_uid` comparison, so flipping it also flips the LOOP-2 PREHEADER's EBB,
       where the target wants `sll $v1,$s0,4` (i's register, 0x80045338). One crown flip
       costs the block-0 guard AND the preheader shift.
  (R2) **invalidate the crown between insn 17 and insn 31**, so `delete_reg_equiv` promotes
       `reg_next_eqv` (= i) to `qty_first_reg`. CLOSED this session — see below.
  (R3) **have one of the two insns created after BOTH cse passes** (cse1 and cse2 both run:
       `-O2` sets `flag_rerun_cse_after_loop` and `text1a_c.cse2` is in the dumps). s52
       measured the loop.c strength-reduction route for the shift (the giv chain lands in
       loop 1's PREHEADER, after the guard branch, not in block 0); s53 measured that
       `duplicate_loop_exit_test` builds the entry guard in the FIRST `jump_optimize`
       (toplev.c:2827), i.e. before cse. R3 is the only route the ledger has not closed.

### R2 is closed: a value-preserving write to the crown register is deleted by cse

Two measurements, one dump, one predicate.

  * **`a0 = i;` in block 0 between the shift and the guard, on the BASE chassis (crown = i,
    reg 75)** — `P1_a0eqi_blk0.c` = **score 6, insns 83**. The construct is FREE in
    instruction count (83, not 84) because a later pass propagates it away. But it does not
    move the crown: `dumps_P1/text1a_c.cse` keeps the copy as
    `(insn 31 (set (reg/v:SI 72) (reg/v:SI 75)))`, insn 17 is still
    `(ashift (reg/v:SI 75) 4)` and the guard still `(lt (reg/v:SI 75) ...)`. Reason: reg 72
    is not the crown here, so cse takes the `qty_first_reg[reg_qty[REGNO (dest)]] !=
    REGNO (dest)` branch at **tools/gcc-2.7.2/cse.c:6878** and rewrites SET_SRC to the class
    head; the insn stays a real copy and invalidates nothing.
  * **the same `a0 = i;` on form A, where a0 IS the crown** — `A2_A_plus_blk0_inval.c` =
    **score 11, insns 83, output identical to plain A (11/83)**. `A2.cse.fn` shows the copy
    GONE from the cse output (insn 28 links straight to insn 34) and the guard still
    `(lt (reg/v:SI 72) ...)`. Reason: canon_reg rewrites the source to the crown, the insn
    becomes `(set (reg 72) (reg 72))`, and the cse.c:6871-6877 comment ("In general, it is
    good to have a SET with SET_SRC == SET_DEST. However, there is an important exception:
    If both are registers that are not the head of their equivalence class, replace SET_SRC
    with the head of the class.") deliberately PRESERVES the self-move when dest is the
    head — a self-move that never reaches the assembler.

  Those two cases partition every value-preserving write to the a0 quantity: dest != head
  becomes a live copy that changes no equivalence; dest == head degenerates to a self-move
  and disappears. A write with a DIFFERENT value does invalidate, but it destroys the a0
  value that loop 2's preheader re-reads (`addu $s0,$s2,$zero` at 0x8004532C), and every
  spelling that preserves it in a second pseudo needs a seventh long-lived callee-save,
  which the target's frame (0x30, $s0-$s5 + $ra) does not have.

### The crown flip cannot be bought with a post-loop mention either

The crown comparison is `uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`
(cse.c:854-856). On the target's geometry i's last mention is loop 2's exit compare — the
last insn of the loop — so flipping the crown needs an a0 mention AFTER it. Measured:

    tail `a0 = i;` as the function's final statement (T1)            score 27  insns 84
    T1 + the block-0 `a0 = i;` invalidation attempt  (T2)            score 27  insns 84

Both cost an instruction. Unlike the s51 `dead = a0;` forms, a store INTO a0 is not
deletable by jump.c:577 (its `regno_first_uid[dest] == INSN_UID && regno_last_note_uid[dest]
== INSN_UID` predicate cannot hold for a pseudo with other mentions), and unlike P1 there is
no later reader to propagate it into, so it materialises.

### A new 83-instruction chassis banked (the parameter carries the count)

`P4_a0_carries_count.c` — the preserved a0 moves into a fresh local `base` ($s2, which also
becomes loop 2's walking pointer) and the PARAMETER pseudo is reused to hold `D_800A33AC`,
which the target really does keep in `$a0` (`lw $a0, %gp_rel(D_800A33AC)($gp)` at
0x800452D4). Result **score 6, insns 83**: the register economy is sound (no extra
callee-save, frame unchanged), but the crown analysis is untouched — the extra `base = a0`
copy simply crowns `base` and then `i`, so block 0's shift is still canonicalised to i.
Banked as a chassis, not a lever.

- [s54] candidate.c re-measured on HEAD 2026-09-03: score=1, build_insns=83, target_insns=83; the residual is idx 9 only (target `sll $v1,$s2,4` vs build `sll $v1,$s0,4`).
- [s54] cse EBB bounds on the a0-as-pointer chassis (BASE.cse2.fn): block 0 = `from 2 to 36`, loop 1 = `38 to 67`, loop-2 preheader = `70 to 109`, loop 2 = `111 to 173`. Block 0's shift and the loop-1 entry guard are unavoidably in the same EBB, so canon_reg gives them the same register; the target gives them different ones.
- [s54] make_regs_eqv's crown test (cse.c:842-857) is `(last_uid[new] > cse_basic_block_end || first_uid[new] < cse_basic_block_start) && last_uid[new] > last_uid[firstr]`. For i to LOSE the crown in block 0 while WINNING it in the loop-2 preheader EBB, clause 2 must hold (it is EBB-independent) and clause 1 must fail in block 0, i.e. i's last mention would have to sit inside cuid 36 — which contradicts i being loop 2's counter. The two EBBs cannot disagree.
- [s54] `a0 = i;` between block 0's shift and guard is free in instruction count (83) but never invalidates the quantity: on the BASE chassis (a0 not the crown) cse.c:6878 rewrites SET_SRC to the class head and keeps a live copy (score 6, dumps_P1); on form A (a0 IS the crown) it degenerates to `(set 72 72)` and is deleted outright, leaving output byte-identical to plain A (11/83, A2.cse.fn).
- [s54] A store INTO the parameter placed after loop 2 costs an instruction (T1 27/84, T2 27/84): jump.c:577's deletion predicate cannot hold for a pseudo with other mentions, and there is no later reader to propagate it into.
- [s54] `P4_a0_carries_count.c` (preserved a0 in a fresh `base`, parameter pseudo reused for `D_800A33AC` -> `$a0`) measures 6/83 — sound register economy, no crown movement.

- [s54] candidate.c re-measured on HEAD 2026-09-03: score=1, build_insns=83, target_insns=83; the single differing instruction is idx 9 (target `sll $v1,$s2,4`, build `sll $v1,$s0,4`).

- [s54] cse EBB bounds for func_80045294 on the a0-as-pointer chassis (tmp/grind/func_80045294/s54/BASE.cse2.fn header): block 0 = `;; Processing block from 2 to 36`, loop 1 = `38 to 67`, loop-2 preheader = `70 to 109`, loop 2 = `111 to 173` with path retries 111-142 and 144-173.

- [s54] Block 0 holds exactly three references to the a0 quantity - the copy (RTL insn 15), the shift (insn 17) and the loop-1 entry guard (insn 31) - and cse canonicalises every reference in one EBB to qty_first_reg, while the target prints $s2 for the shift and $s0 for the guard.

- [s54] make_regs_eqv's crown test (cse.c:842-857) is `(uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`. Clause 2 is EBB-independent, so block 0 and the loop-2 preheader EBB cannot disagree about the crown unless clause 1 fails in block 0, which would require the loop counter's last mention to sit inside cuid 36.

- [s54] `a0 = i;` between block 0's shift and guard is free in instruction count (83, not 84) on the BASE chassis - a later pass propagates it away - but cse.c:6878 rewrites its source to the class head and it never invalidates the quantity (score 6, dumps_P1/text1a_c.cse).

- [s54] On form A, where the parameter pseudo IS the class head, the same construct degenerates to `(set (reg 72) (reg 72))`, is dropped, and the build is byte-identical to plain A (both 11/83) - confirmed by the copy's absence from A2.cse.fn.

- [s54] A store INTO the parameter placed after loop 2 costs one instruction (T1 = 27/84, T2 = 27/84): jump.c:577's deletion predicate cannot hold for a pseudo with other mentions and there is no later reader to propagate it into.

- [s54] Reusing the parameter pseudo for D_800A33AC with a fresh `base` carrier keeps the frame at 0x30 and the build at 83 instructions (score 6), confirming the target's `lw $a0, %gp_rel(D_800A33AC)($gp)` allocation is reproducible in C, but it does not move block 0's crown.

- [s54] Neither candidate.c nor any form measured this session carries a FAKE construct, so the tools/fake_ablate.py leg of the kill re-audit stays vacuous for this function (as recorded in s51-s53).

## s55 (synthesis) — cse EBB structure is a lever, and cse2 is the only remaining blocker

Measurements (all `sandbox func_80045294 --disable all`, HEAD 2026-09-03 chassis,
`src/text1a_c.c` with the candidate body pasted over the `INCLUDE_ASM` line at
src/text1a_c.c:1445):

| form | score | build_insns | note |
|---|---|---|---|
| `memory/grind/func_80045294/candidate.c` | 1 | 83 | chassis confirmation; residual = idx 9 `sll $v1,$s2,4` vs `sll $v1,$s0,4` |
| `s55/B0_control_splitdecl.c` | 1 | 83 | candidate.c with all six block-0 locals declared uninitialised and assigned in the same order — byte-neutral control |
| `s55/B1_dowhile_after_copy.c` | 6 | 83 | B0 plus `do { i = a0; } while (0);` around block 0's copy |
| `s55/R1_while_loop1.c` | 1 | 83 | candidate.c with loop 1 respelled as a `while` loop (s53 kill re-audit) |

`tools/fake_ablate.py --func func_80045294 --file text1a_c --candidate
memory/grind/func_80045294/candidate.c` → "no FAKE-annotated constructs found ...
nothing to ablate". The kill ledger's s52-s54 instance kills are all recorded on
this same chassis with zero FAKE constructs, so none of them is FAKE-contaminated.

### GCC source facts established this session (read, not inferred)

* `tools/gcc-2.7.2/cse.c:842-857` — `make_regs_eqv`'s crown test is a conjunction
  of an **EBB-relative** clause (`uid_cuid[regno_last_uid[new]] >
  cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start`)
  and a **global** clause (`uid_cuid[regno_last_uid[new]] >
  uid_cuid[regno_last_uid[firstr]]`). s54's ledger entry described the test as
  purely global; that is corrected here.
* `tools/gcc-2.7.2/cse.c:8039` — `cse_end_of_basic_block` scans
  `while (p && GET_CODE (p) != CODE_LABEL)`: **any** code label terminates the
  extended basic block, irrespective of `LABEL_NUSES`.
* `tools/gcc-2.7.2/cse.c:8055-8058` — the EBB also terminates at a
  `NOTE_INSN_LOOP_END`, but **only when `after_loop` is 0**.
* `tools/gcc-2.7.2/cse.c:8060-8063` — and at a `NOTE_INSN_SETJMP` (not reachable
  from this function).
* `tools/gcc-2.7.2/toplev.c:2865` — cse1: `cse_main (insns, ..., 0, cse_dump_file)`
  (`after_loop = 0`).
* `tools/gcc-2.7.2/toplev.c:2922-2929` — `reg_scan` is re-run on the post-loop RTL,
  then cse2: `cse_main (insns, max_reg_num (), 1, cse2_dump_file)`
  (`after_loop = 1`). Guarded by `flag_rerun_cse_after_loop`, which `-O2` sets
  (Makefile:35 `CC_FLAGS := -O2 ...`).

### The dump evidence for B1

`pwsh tools/grinder/dump.ps1 func_80045294` with B1 in `src/`, copies banked at
`tmp/grind/func_80045294/s55/B1.cse` and `s55/B1.cse2`:

* `B1.cse` (cse1 output), block 0: `(ashift:SI (reg/v:SI 72) (const_int 4))` —
  reg 72 is the parameter pseudo, which is the operand the target prints
  (`sll $v1,$s2,4`). The `do { } while (0)` shows up as
  `(note 15 ... NOTE_INSN_LOOP_BEG)` / `(note 20 ... NOTE_INSN_LOOP_CONT)` /
  `(note 27 ... NOTE_INSN_LOOP_END)` immediately before it, with no `code_label`
  and no emitted insn.
* `B1.cse2` (cse2 output), same insn: `(ashift:SI (reg/v:SI 75) (const_int 4))` —
  reg 75 is loop 1's counter. cse2's `after_loop = 1` ignores the LOOP_END note,
  re-merges block 0 into one EBB, re-derives the equivalence from the surviving
  `i = a0` copy, and `canon_reg` rewrites the operand back.
* Emitted B1 block 0 (via `mipsel-linux-gnu-objdump -d` on
  `tmp/sandbox/func_80045294/text1a_c.o`, banked at `s55/B1.txt`):
  `sw ra,40 / sw s5,36 / sw s4,32 / sw s0,16 / move s0,s2 / sll v1,s0,0x4`
  against the target's
  `sw s0,16 / move s0,s2 / sll v1,s2,0x4 / sw ra,40 / sw s5,36 / sw s4,32`
  — six positions differ, which is exactly the score of 6.

### What this means for the search

The idx-9 residual is now attributable to a single pass invocation
(`toplev.c:2926`) and a single predicate (`cse.c:8039`). Two operands of one cse
quantity can print two registers if and only if the two insns are in different
EBBs at both cse passes, and at cse2 the only surviving EBB terminator is a
`CODE_LABEL`. The next session's whole job is to find an ordinary-C construct that
puts a zero-instruction, jump_optimize-surviving `CODE_LABEL` between block 0's
`i = a0` and block 0's `a0 << 4`.

Artifacts: `tmp/grind/func_80045294/s55/{B0_control_splitdecl.c,
B1_dowhile_after_copy.c,R1_while_loop1.c,B1.cse,B1.cse2,B1.txt,target.txt,
apply.py,dis.sh}`; rejected form banked at
`memory/grind/func_80045294/rejected/s55-dowhile0-split-undone-by-cse2.c`.

- [s55] Chassis re-confirmed: memory/grind/func_80045294/candidate.c pasted over the INCLUDE_ASM line at src/text1a_c.c:1445 gives `sandbox func_80045294 --disable all` = score 1, target_insns 83, build_insns 83. Residual is instruction idx 9 only: target `sll $v1,$s2,4`, build `sll $v1,$s0,4`.

- [s55] CORRECTION to the s54 ledger model: make_regs_eqv's crown test (tools/gcc-2.7.2/cse.c:842-857) is NOT purely global. It is a conjunction of an EBB-relative clause (uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) and a global clause (uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]).

- [s55] More decisive than the crown test: cse canonicalisation is per-EBB. cse_main clears the quantity table at each extended-basic-block boundary, so two references to one quantity that sit in DIFFERENT EBBs can print DIFFERENT registers with no crown flip at all. The s53/s54 'one crown per EBB so the two operands are deadlocked' framing holds only for insns sharing an EBB and never considered splitting it.

- [s55] tools/gcc-2.7.2/cse.c:8039 — cse_end_of_basic_block scans `while (p && GET_CODE (p) != CODE_LABEL)`: ANY code label ends the EBB regardless of LABEL_NUSES.

- [s55] tools/gcc-2.7.2/cse.c:8054-8056 — the EBB also ends at NOTE_INSN_LOOP_END, but only when after_loop == 0. tools/gcc-2.7.2/toplev.c:2865 runs cse1 with after_loop = 0; toplev.c:2926 runs cse2 with after_loop = 1. -O2 (Makefile:35) enables cse2 via flag_rerun_cse_after_loop, and toplev.c:2925 re-runs reg_scan on post-loop RTL immediately before it.

- [s55] MEASURED (new): with `do { i = a0; } while (0);` between block 0's copy and its shift, dumps/text1a_c.cse shows block 0's shift as (ashift:SI (reg/v:SI 72) (const_int 4)) — the parameter pseudo, the target's operand — and dumps/text1a_c.cse2 shows the same insn as (ashift:SI (reg/v:SI 75) ...). The lever fires at cse1 and is undone by cse2 alone.

- [s55] The do-while(0) costs zero instructions (B1 build_insns = 83 = target) and its control (same split declarations, plain assignment) measures score 1, so the construct is instruction-neutral; its 5 extra score points are a 3-insn prologue rotation plus the unchanged idx-9 operand.

- [s55] B1 emitted block 0: sw ra,40 / sw s5,36 / sw s4,32 / sw s0,16 / move s0,s2 / sll v1,s0,0x4, against the target's sw s0,16 / move s0,s2 / sll v1,s2,0x4 / sw ra,40 / sw s5,36 / sw s4,32. This exposes a previously uncatalogued free 3-insn scheduling degree of freedom in block 0's prologue cluster.

- [s55] Closed-form restatement of the residual: the target's two operands require block 0's copy and block 0's shift to be in different cse EBBs at BOTH cse passes; at cse2 the only surviving EBB terminator is a CODE_LABEL. Flipping the global crown clause instead cannot work, because clause (A) is true for the loop counter in both block 0's EBB and the loop-2 preheader's EBB, so one global comparison decides both and they need opposite answers — the same wall s53's form A measured at 11/83, now with a code-level predicate.

- [s55] Kill re-audit discharged: tools/fake_ablate.py finds no FAKE-annotated constructs in candidate.c (nothing to ablate), and the closest-to-target instance kill (s53's loop-1 control-flow respellings) re-measures at score 1, insns 83 on the current chassis.


## s56 (synthesis) — the block-0 EBB split validated at cse1, and the H-geometry schedule class-killed

**Chassis re-measured first (mandated).** `memory/grind/func_80045294/candidate.c`
(a0-as-pointer, i-first = permutation `SIVLCF`) re-measures **score=1, build_insns=83,
target_insns=83** on HEAD 2026-09-03. Residual unchanged: instruction idx 9, target
`sll $v1,$s2,4` (a0's register) vs build `sll $v1,$s0,4` (i's register).
`tools/fake_ablate.py` remains vacuous — no form in this ledger carries a FAKE construct.

### 1. Exhaustive block-0 declaration-order sweep (120 forms, all measured)

`tmp/grind/func_80045294/s56/gen.py` enumerates every permutation of block 0's six
initialised declarations — `S` (`sum = 0`), `V` (`v1 = a0 << 4`), `L` (`s4 = load`),
`I` (`i = a0`), `C` (`count = D_800A33AC`), `F` (`s5 = s4 + a1`) — subject only to the
data-dependence constraints `V < L < F`. All 120 were built and scored
(`tmp/grind/func_80045294/s56/sweep.csv`, driver `sweep.ps1`):

| score | count |
|---|---|
| 1 | 6 |
| 2 | 53 |
| 4 | 36 |
| 5 | 24 |

**No permutation reaches 0.** The six score-1 forms are exactly those ordering
`S < I < V < L < F` with `C` free (`CSIVLF, SCIVLF, SICVLF, SIVCLF, SIVLCF, SIVLFC`),
i.e. `sum` before `i` before `v1` before `s4` before `s5`; `count`'s position is
byte-irrelevant. Every `V < I` (shift-first / "H") permutation lands at 2 or worse.
Statement order inside block 0 is therefore a fully-enumerated, closed axis on this
chassis.

### 2. The H geometry's 2-instruction residual is a scheduler LUID tiebreak — class-killed

With `H_a0ptr_vfirst.c` applied, `pwsh tools/grinder/dump.ps1 func_80045294` gives
`text1a_c.sched2` (extract `tmp/grind/func_80045294/s56/H.sched2.fn`). Insn identities
in block 0: **insn 14 = `(set (reg/v:SI 3 v1) (ashift ...))`** (the shift),
**insn 22 = `(set (reg/v:SI 16 s0) ...)`** (the copy `i = a0`), insn 12 = `sum = 0`,
insn 4 = `move s2,a0`, insns 197/199/201/203/205/207/209/211 = the prologue
sp-adjust + seven callee-save stores.

sched2's own priority listing for block 0:

    ;; insn[ 197]: priority = 1   ... ;; insn[   4]: priority = 1
    ;; insn[  12]: priority = 1   ;; insn[  14]: priority = 1
    ;; insn[  22]: priority = 1   ;; insn[  19]: priority = 1
    ;; insn[  28]: priority = 2   ;; insn[  31]: priority = 2

Every block-0 insn except the two tail insns has `INSN_PRIORITY == 1`.
`rank_for_schedule` (tools/gcc-2.7.2/sched.c:2408) compares priority first
(sched.c:2418), then the last-scheduled-insn dependence class, then falls through to
**`return INSN_LUID (tmp) - INSN_LUID (tmp2);` (sched.c:2464)** — i.e. original insn
order. `priority()` (sched.c:1433-1521) is computed *only* from `LOG_LINKS`
(predecessors): `prev_priority = priority (x) + insn_cost (x, prev, insn) - 1`. Both
the shift and the copy have exactly one real predecessor, insn 4 (`move s2,a0`,
priority 1, cost 1), so both are pinned at priority 1 and the tie is always broken by
source order. Emitted order for H is therefore `... sw s1, sum=0, sw s0, sll, move s0,
sw ra ...` against the target's `... sw s1, sum=0, sw s0, move s0, sll, sw ra ...` —
insns 14 and 22 swapped, and nothing else: **score 2, and the two differing positions
are exactly those two insns.**

Because the target's block-0 shift reads the incoming parameter (`sll $v1,$s2,4`, whose
only predecessor is the parameter move), no source spelling can lengthen the shift's
predecessor chain without changing that operand, so `INSN_PRIORITY(shift)` cannot exceed
`INSN_PRIORITY(copy)` and the LUID tiebreak decides. The v1-first geometry cannot emit
the copy before the shift.

### 3. The target's own register layout proves the original had crown = i

Read off `target.txt`: in loop 2 the pointer (`$s2`, the parameter's register) is last
mentioned at `addiu $s2,$s2,0x10`, which precedes the counter's last mention
`slt $v0,$s0,$v0` at the loop-2 exit test. So `uid_cuid[regno_last_uid[i]] >
uid_cuid[regno_last_uid[a0]]`, i.e. clause (B) of `make_regs_eqv` (cse.c:854-857) is
TRUE for i in the original compilation too — **the original's cse crown was i, exactly as
on the candidate chassis**, and block 0's shift still printed `$s2`. Combined with §2
(the shift cannot precede the copy and still be emitted after it), the original's block-0
shift must have escaped `canon_reg` (cse.c:2532-2572) by sitting in a **different
extended basic block** from the copy, or by being created after cse2.

### 4. An EBB split DOES preserve the parameter at cse1 — and loop.c undoes it, not cse2

`s56/L1_label_probe.c` wraps block 0's shift in `do { v1 = a0 << 4; } while (i < a0);`
(semantics-preserving: `i == a0`, so the body runs exactly once). Measured
**score 4, build_insns 86** (the loop costs `slt` + branch + a nop), and the dumps say:

* `text1a_c.cse`: `;; Processing block from 2 to 18` then `;; Processing block from 20 to
  33`, with `(code_label 18 ...)` between them, and the shift printed as
  **`(ashift:SI (reg/v:SI 72) ...)`** — the parameter pseudo, i.e. the target's operand.
* `text1a_c.cse2`: the same `(code_label 18 ...)` is STILL PRESENT, but the shift now
  appears *above* it in the insn stream and reads **`(reg/v:SI 75)`** (the counter).

So cse2 does not ignore the label (cse.c:8039 terminates the EBB at any CODE_LABEL
irrespective of `after_loop`); what defeats the device is **loop.c hoisting the
loop-invariant shift out of the do-while into its preheader, i.e. above the label**,
between cse1 (toplev.c:2865) and cse2 (toplev.c:2926). This corrects the s55 framing,
which attributed the cse2 regression to the after_loop-gated NOTE_INSN_LOOP_END
(cse.c:8055) alone: a real CODE_LABEL survives cse2 fine — the shift is what moves.

`s56/L2_ifjoin_probe.c` tried the non-loop alternative (`if (i < a0) { sum = 1; }`
between the copy and the shift; never taken because `i == a0`). Measured **score 1,
build_insns 83** — the whole `if` is folded away before cse1 (`text1a_c.cse` shows
`;; Processing block from 2 to 46` with no `code_label` at all), so no split happens.
A non-loop label carrier therefore needs a condition `jump_optimize` cannot resolve.

### 5. Form A re-measured and decomposed

`A_reuse_i_as_idx.c` (crown flipped to a0 by reusing i as loop 2's byte offset)
re-measures **11/83**. Instruction-level diff against the target: block 0's first
seventeen instructions are byte-exact including `sll $v1,$s2,4`; the differences are
(i) the loop-1 entry guard `slt $v0,$s2,$a0` vs target `slt $v0,$s0,$a0` — one insn, the
crown's price — and (ii) a pure register-name swap through the loop-2 preheader and body
(build counter `$s1` / idx `$s0` against target counter `$s0` / idx `$s1`), which is an
*allocation* consequence of A's variable-sharing pattern (i shared between loop-1 counter
and loop-2 idx) differing from the target's (`$s0` = loop-1 counter + loop-2 counter,
`$s1` = sum + loop-2 idx). Note that adopting the target's sharing pattern re-crowns i,
because it makes i loop 2's counter and therefore the quantity's last mention.

- [s56] Chassis re-confirmed: candidate.c = score 1, target_insns 83, build_insns 83 on HEAD 2026-09-03; residual is idx 9 only.
- [s56] Exhaustive 120-permutation sweep of block 0's six initialised declarations (constraints V<L<F): scores 1x6, 2x53, 4x36, 5x24; NONE reaches 0. The six score-1 forms are exactly the orderings S<I<V<L<F with C free (sweep.csv).
- [s56] sched2 dump for the v1-first (H) geometry: every block-0 insn has INSN_PRIORITY 1 except the two tail insns (28, 31 = 2). rank_for_schedule (sched.c:2408) therefore falls through to the LUID tiebreak at sched.c:2464, so the emitted order is the RTL order; H's score-2 residual is exactly insns 14 (the shift) and 22 (the copy) swapped.
- [s56] priority() (sched.c:1433-1521) derives INSN_PRIORITY only from LOG_LINKS predecessors. The target's block-0 shift reads the incoming parameter, whose only predecessor is `move s2,a0` (priority 1, cost 1), so the shift's priority cannot be raised above the copy's without changing the shift's operand.
- [s56] The TARGET's own loop 2 mentions the pointer ($s2, the parameter) at `addiu $s2,$s2,0x10` BEFORE the counter's last mention at the exit `slt $v0,$s0,$v0`, so clause (B) of make_regs_eqv (cse.c:854-857) held for the counter in the original compilation too: the original's cse crown was i, and block 0's shift still printed $s2. The original's shift therefore escaped canon_reg by EBB placement or post-cse creation, not by a crown flip.
- [s56] MEASURED: `do { v1 = a0 << 4; } while (i < a0);` (runs once; i == a0) splits block 0 at cse1 (`Processing block from 2 to 18` / `from 20 to 33`, code_label 18 between) and the shift is printed `(ashift:SI (reg/v:SI 72))` — the target's operand — for only the second time in 56 sessions. Score 4, build_insns 86 (the loop costs slt + branch + nop).
- [s56] CORRECTION to s55: the cse2 regression is NOT cse2 ignoring the split. code_label 18 is still present in text1a_c.cse2; loop.c hoists the loop-invariant shift out of the do-while into its preheader, i.e. ABOVE the label, between toplev.c:2865 (cse1) and toplev.c:2926 (cse2). The shift then reads (reg/v:SI 75) again.
- [s56] `if (i < a0) { sum = 1; }` placed between the copy and the shift (never taken, i == a0) is folded away before cse1 — text1a_c.cse shows `Processing block from 2 to 46` with no code_label — so it costs nothing (1/83) and splits nothing. A non-loop label carrier needs a condition jump_optimize cannot resolve.
- [s56] Form A re-measured at 11/83: block 0's first seventeen instructions are byte-exact INCLUDING `sll $v1,$s2,4`; the residual is the loop-1 guard operand (1 insn) plus a pure $s0/$s1 register-name swap across the loop-2 preheader and body, which follows from A's variable-sharing pattern rather than from cse.

- [s56] Chassis re-confirmed: memory/grind/func_80045294/candidate.c pasted over the INCLUDE_ASM line at src/text1a_c.c:1445 gives sandbox func_80045294 --disable all = score 1, target_insns 83, build_insns 83 on HEAD 2026-09-03; residual is instruction idx 9 only (target sll $v1,$s2,4 vs build sll $v1,$s0,4).

- [s56] tools/fake_ablate.py still finds no FAKE-annotated constructs in this ledger, so the FAKE-ablation leg of the kill re-audit stays vacuous; the two closest-to-target instance kills (H = v1-first, A = crown-flipped) were re-measured this session at 2/83 and 11/83 respectively and both stand.

- [s56] Exhaustive enumeration: all 120 permutations of block 0's six initialised declarations satisfying v1 < s4 < s5 were built and scored (tmp/grind/func_80045294/s56/sweep.csv). Histogram 1x6, 2x53, 4x36, 5x24; no form reaches 0. The six score-1 forms are exactly the orderings sum < i < v1 < s4 < s5 with count free.

- [s56] sched2's own priority listing for block 0 on the v1-first geometry: every insn has INSN_PRIORITY 1 except insns 28 and 31 (priority 2). The shift is insn 14 and the copy is insn 22, both priority 1, so rank_for_schedule (sched.c:2408, priority compare at 2418) falls through to the INSN_LUID tiebreak at sched.c:2464 and the emitted order is the RTL order.

- [s56] priority() (sched.c:1433-1521) derives INSN_PRIORITY only from LOG_LINKS predecessors; the target's block-0 shift reads the incoming parameter whose only predecessor is `move s2,a0` (priority 1, cost 1), so the shift's priority cannot exceed the copy's without changing the shift's operand.

- [s56] In the shipped code the parameter's register $s2 is last mentioned at loop 2's addiu $s2,$s2,0x10, BEFORE the counter's last mention at the loop-2 exit slt $v0,$s0,$v0, so clause (B) of make_regs_eqv (cse.c:854-857) held for the counter in the original compilation: the original's cse crown was i and block 0's shift still printed $s2.

- [s56] MEASURED: do { v1 = a0 << 4; } while (i < a0); (runs exactly once because i == a0) splits block 0's extended basic block at cse1 — dumps/text1a_c.cse shows `Processing block from 2 to 18` then `from 20 to 33` with (code_label 18 ...) between them — and prints the shift as (ashift:SI (reg/v:SI 72)), the target's operand. Score 4, build_insns 86.

- [s56] CORRECTION to the s55 ledger: the cse2 regression is NOT cse2 ignoring the split. (code_label 18 ...) is still present in dumps/text1a_c.cse2; loop.c (toplev.c:2895) hoists the loop-invariant shift out of the do-while into its preheader — above the label — between cse1 (toplev.c:2865) and cse2 (toplev.c:2926), and the shift then reads (reg/v:SI 75) again.

- [s56] if (i < a0) { sum = 1; } between block 0's copy and its shift (never taken, since i == a0) is folded away before cse1: dumps/text1a_c.cse shows block 0 as one EBB `Processing block from 2 to 46` with no code_label. It measures 1/83, byte-identical to candidate.c. A non-loop label carrier needs a condition jump_optimize (toplev.c:2827) cannot resolve.

- [s56] Form A (A_reuse_i_as_idx) re-measured at 11/83 and decomposed for the first time: block 0's first seventeen instructions are byte-exact including sll $v1,$s2,4; one point is the loop-1 guard operand ($s2 vs $s0) and ten are a pure $s0/$s1 register-name swap through the loop-2 preheader and body, an allocation consequence of A's variable-sharing pattern rather than of cse.

- [s56] src/text1a_c.c was restored to HEAD at end of session; the working tree carries only the two updated ledger files, two new rejected/ forms, and metrics/events.jsonl. candidate.c is unchanged (still the score-1 a0-as-pointer i-first form).

- [s57] SOLVER MODALITY, FIRST PASS EVER ON THIS FUNCTION. Chassis re-verified: candidate.c applied over src/text1a_c.c:1445 measures score 1, target_insns 83, build_insns 83 on the HEAD 2026-09-03 chassis. The single differing instruction is build idx 9: ours `sll v1,s0,0x4`, target `sll v1,s2,0x4` (tmp/grind/func_80045294/s57/hon.txt, tgt.txt, produced with engine.score.normalized_insns against build/src/text1a_c.o, which carries the INCLUDE_ASM target bytes).
- [s57] THE RA LAYER IS AT TARGET AND IS NOW CLASS-KILLED. tools/ra_solver/extract.py on the candidate body gives dispositions 72:$s2 73:$s3 74:$s1 75:$s0 76:$v1 77:$s4 79:$a1 80:$s5 85:$s1 91:$v0 - precisely what the target's 82 matching instructions require. global.c:1275 assigns one hard register per pseudo function-wide, so an operand divergence on ONE reference of pseudo 75 while seventeen other references of the same pseudo are correct is unreachable in this allocator. inverse.py does return 54 two-atom vectors for the naive $s0/$s2 swap goal, but that goal breaks the eighteen matching instructions at build idx 1,2,7,8,18,24,26,38,39,41,44,46,49,61,63,64,77,79.
- [s57] TOOLING HAZARD, PIPELINE-WIDE: inverse_compose.py classify mis-types every cse/canon_reg operand-substitution residual as RA. It prints "FIRST DIVERGENCE: RA ... ours sll v1,s0,0x4 / target sll v1,s2,0x4" for func_80045294 while the RA model is provably already at the target. Cause: classify separates PRE-RA from RA by comparing REGISTER-BLANKED multisets, and blanking cannot distinguish "our pre-RA RTL names a different PSEUDO" from "the allocator chose a different hard register". Disambiguator: extract the RA model and check whether the divergent register is already correct on that pseudo's other references; if it is, the layer is PRE-RA and the RA backend has nothing to search. This survives the object path, so the 2026-08-30 --target-object escape does not fix it (contrast the func_80017848 s25 text-path artifact).
- [s57] FORM A CARRIES NO ALLOCATION DIFFERENCE. modelA.json's dispositions are identical to the candidate's in all 24 entries, so A's eleven points (build idx 18, 38, 39, 41, 45, 51, 57, 61, 63, 64, 66) are entirely a pre-RA value-to-pseudo assignment produced by A's variable sharing, and two of them (61, 66) are just loop 2's two increments in the other source order. s56 frontier item 3 is refuted: A and candidate.c are the two endpoints of one pre-RA trade, not two independent one-instruction attack surfaces.
- [s57] THE SCHEDULER LAYER IS CLASS-KILLED FROM AN INDEPENDENT ANGLE. tools/sched_solver reported parity=True on this TU (154 funcs, 560 blocks, 3618 picks). Exhaustive depth-2 enumeration of the whole scheduler input space for the H (v1-first) geometry's block 0 returns 40 goal-reaching vectors at pass 2 and 12 at pass 1; the only physically spellable class is a luid reorder, which IS the i-first geometry that loses the shift operand at cse, and every add_dep vector needs a predecessor of higher LUID than its successor. Confirms s56-H1 (sched.c:2464) by a route that does not depend on reading priorities out of a dump.
- [s57] NEW CONSTRAINT ON BOTH LIVE ROUTES: whatever closes block 0 must not change block 0's statement order, because the sched model shows statement order is the only scheduler-visible lever for the copy/shift pair and it is already at its best value in candidate.c.

- [s58] Chassis re-verified this session: memory/grind/func_80045294/candidate.c applied over the INCLUDE_ASM line at src/text1a_c.c:1445 measures score 1, target_insns 83, build_insns 83 on the HEAD 2026-09-03 chassis. The brief's 'measurement unavailable' is resolved; the ledger floor of 1 stands.

- [s58] The single residual is build idx 9: ours `sll v1,s0,0x4`, target `sll v1,s2,0x4`. Full 83-instruction streams banked at tmp/grind/func_80045294/s57/hon.txt and tgt.txt (engine.score.normalized_insns against tmp/sandbox/func_80045294/text1a_c.o and build/src/text1a_c.o, the latter carrying the INCLUDE_ASM target bytes).

- [s58] This is the first time tools/ra_solver or tools/sched_solver has been run on func_80045294 in 57 sessions - a grep of the whole ledger for inverse_compose, perturb.py and --target-object returns nothing before s57.

- [s58] Candidate-chassis global-alloc dispositions: 72:$s2 73:$s3 74:$s1 75:$s0 76:$v1 77:$s4 79:$a1 80:$s5 85:$s1 91:$v0. Form A's dispositions are identical in all 24 entries. Both equal what the target's bytes require.

- [s58] inverse.py reported 20 preference atoms mechanically FORECLOSED: $s0 and $s2 are callee-saved, so they can never appear as hard registers in this function's pre-RA RTL from any C at all, and only a forbidden register-asm pin could reach them.

- [s58] tools/sched_solver/extract.py reported parity=True on src/text1a_c.c as a whole: 154 functions, 560 blocks, 3618 picks reproduced exactly, so the H-geometry enumeration rests on a validated model of this TU specifically.

- [s58] No new C form was spelled this session and no new form beat 1; candidate.c is unchanged and src/text1a_c.c was restored to its HEAD INCLUDE_ASM state after every probe (git status shows only ledger files and metrics/events.jsonl modified).

## s59 (forensics) — instrumented-cc1 pass attribution

- [s59] Chassis re-confirmed: `memory/grind/func_80045294/candidate.c` pasted over the
  `INCLUDE_ASM` line at src/text1a_c.c:1445 gives `sandbox func_80045294 --disable all`
  = score 1, target_insns 83, build_insns 83 on HEAD 2026-09-03. Residual is instruction
  idx 9 only: target `sll $v1,$s2,4` (reg 72 = a0), build `sll $v1,$s0,4` (reg 75 = i).
  Zero FAKE constructs in the form, so the fake_ablate leg of the kill re-audit is again
  vacuous for this function.

- [s59] KILL RE-AUDIT (the closest banked score-0-attempt instance kill re-measured on the
  current chassis): `rejected/s52-a0ptr-separate-loop2-counter-collapse-80.c` re-measures
  **score 30 / build_insns 80 / target_insns 83**, identical in shape to the s48/s52
  measurement. The kill stands on today's chassis.

- [s59] Pseudo dictionary for this function (from `tmp/grind/func_80045294/s59/dumps_C/fn.rtl`):
  reg 72 = parameter a0, reg 73 = parameter a1, reg 74 = sum, reg 75 = i, reg 76 = v1,
  reg 77 = s4, reg 79 = count, reg 80 = s5. Block 0's RTL is insn 4 (`72 = $a0`),
  insn 6 (`73 = $a1`), insn 12 (`74 = 0`), insn 15 (`75 = 72`, the copy), insn 17
  (`76 = 72 << 4`, the shift), insn 22 (the s4 load), insn 25 (the count load), insn 28
  (s5 = s4 + a1), insn 31 (`slt 75, 79`, the loop-1 entry guard).

- [s59] cse1 on candidate.c rewrites insn 17's operand from reg 72 to reg 75 (`fn.rtl` and
  `fn.jump` print `(ashift:SI (reg/v:SI 72) ...)`, `fn.cse` prints
  `(ashift:SI (reg/v:SI 75) ...)`), and every later dump (loop, cse2, combine, sched, greg)
  carries reg 75. For this function the per-function cse2 region is byte-identical to the
  cse1 region, i.e. loop.c changes nothing in func_80045294 on the candidate chassis
  beyond adding notes.

- [s59] Form J (distinct loop-2 counter `j`) keeps reg 72 in the block-0 shift at every
  pass — `fn.cse`, `fn.loop`, `fn.cse2` and `fn.combine` all print
  `(ashift:SI (reg/v:SI 72) (const_int 4))` — but canon_reg moves the LOOP-1 ENTRY GUARD
  the other way: `fn.cse` prints `(lt:SI (reg/v:SI 72) (reg/v:SI 79))` for insn 31 where
  the pre-cse dump had reg 75. The loop-BOTTOM test keeps reg 75 (it is a separate
  extended block). J's emitted block 0 is
  `sw $18 / move $18,$4 / sw $19 / move $19,$5 / sw $16 / move $16,$0 / sll $3,$18,4 / ...`
  with no `move $16,$18` and with `slt $2,$18,$5` — the shift right, the guard and the copy
  wrong.

- [s59] The three instructions form J loses are attributed: reg 75 is allocated `$4` (`$a0`)
  instead of `$s0` because it crosses no call, which is `find_reg`'s call-clobbered path at
  global.c:972 (`allocno_calls_crossed[allocno] == 0` -> `used1 = fixed_reg_set`), and MIPS
  defines no `REG_ALLOC_ORDER` so the default 0..31 order reaches `$4` first. `fn.greg`
  shows `(insn 15 12 17 (set (reg/v:SI 4 a0) (reg/v:SI 18 s2)))`; by `fn.jump2` insn 15 is
  gone, because `$a0` already held the incoming parameter. The frame drops to `regs= 6/0`
  (five `$s` registers plus `$ra`) against the target's six `$s` registers, so the missing
  sw/lw pair accounts for the other two instructions.

- [s59] Form L (the s56 do-while label carrier) is the first form measured in 59 sessions in
  which cse1 prints BOTH of the target's block-0 operands at once:
  `tmp/grind/func_80045294/s59/dumps_L/fn.cse` line 44 has
  `(ashift:SI (reg/v:SI 72) (const_int 4))` (the target's `sll $v1,$s2,4`) and line 79 has
  `(lt:SI (reg/v:SI 75) (reg/v:SI 78))` (the target's `slt $v0,$s0,$a0`). Mechanism: the
  carrier's CODE_LABEL ends the extended basic block that holds the copy, so BOTH consumers
  are canonicalised against an empty table. The crown question is therefore not the real
  constraint — an extended-block split placed between the copy and the shift satisfies both
  uses simultaneously.

- [s59] Form L's failure is two-step and both steps are named. `fn.loop` shows LICM emitting
  the shift into the carrier's preheader as a fresh insn (printed with uid `-1`, still
  reading reg 72); `fn.cse2` then prints `(ashift:SI (reg/v:SI 75) ...)` because the hoisted
  insn is back inside the first extended block. Form L measures 4 / 86.

- [s59] `cse_end_of_basic_block` (tools/gcc-2.7.2/cse.c) terminates an extended basic block
  on exactly three things: a CODE_LABEL (the `while (p && GET_CODE (p) != CODE_LABEL)`
  scan condition), a `NOTE_INSN_LOOP_END` note guarded by `! after_loop`, and a
  `NOTE_INSN_SETJMP` note. Calls, block notes and loop-begin notes do not end it. Since
  cse2 is invoked with after_loop = 1 (toplev.c:2926), a surviving CODE_LABEL is the only
  splitter that works in both passes, and a CODE_LABEL without a live branch reference is
  deleted by jump_optimize before cse runs.

- [s59] `tools/loop_movables.py` on form L (banked at
  `tmp/grind/func_80045294/s59/loop_movables_L.txt`) reports the carrier loop as
  `insns 17..33: insn_count=3 loop_has_call=False threshold=122`, with insn 21 (regno 76,
  the shift) at `122*1*40 = 4880 >= 3` and insn 26 (regno 80) at `119*1*1 = 119 >= 3`. Both
  move on the middle disjunct of loop.c:1626-1631, by three orders of magnitude, so
  savings / lifetime / insn_count / threshold are all inert levers. The only remaining
  C-level lever named by the tool is `n_times_set` (loop.c:705-709): a second store to v1
  inside the carrier removes the movable, but only if the consecutive set-chain is NOT
  itself invariant, so the second store must read a value the carrier writes.

- [s59] Chassis re-confirmed at dispatch: memory/grind/func_80045294/candidate.c over src/text1a_c.c:1445 gives sandbox func_80045294 --disable all = score 1, target_insns 83, build_insns 83 on HEAD 2026-09-03; the residual is instruction idx 9 only (target sll $v1,$s2,4 = reg 72, build sll $v1,$s0,4 = reg 75). Zero FAKE constructs, so the fake_ablate leg of the kill re-audit is vacuous for this function.

- [s59] KILL RE-AUDIT: rejected/s52-a0ptr-separate-loop2-counter-collapse-80.c re-measures 30 / 80 insns on today's chassis, identical in shape to the s48/s52 result; the kill stands, and this session supplies the pass attribution it previously lacked.

- [s59] Pseudo dictionary from dumps_C/fn.rtl: reg 72 = param a0, 73 = param a1, 74 = sum, 75 = i, 76 = v1, 77 = s4, 79 = count, 80 = s5. Block 0 is insn 4 (72 = $a0), 6 (73 = $a1), 12 (74 = 0), 15 (75 = 72, the copy), 17 (76 = 72 << 4, the shift), 22 (s4 load), 25 (count load), 28 (s5 = s4 + a1), 31 (slt 75, 79, the loop-1 entry guard).

- [s59] On candidate.c, cse1 is the pass that rewrites insn 17's operand: fn.rtl and fn.jump print (ashift:SI (reg/v:SI 72) ...), fn.cse prints (ashift:SI (reg/v:SI 75) ...), and loop, cse2, combine, sched and greg all carry reg 75. The per-function cse2 region is byte-identical to the cse1 region, so loop.c changes nothing in this function on the candidate chassis.

- [s59] Form J keeps reg 72 in the block-0 shift through cse, loop, cse2 and combine, but canon_reg moves the loop-1 ENTRY guard the other way: fn.cse prints (lt:SI (reg/v:SI 72) (reg/v:SI 79)) for insn 31, and the emitted block 0 is sw $18 / move $18,$4 / sw $19 / move $19,$5 / sw $16 / move $16,$0 / sll $3,$18,4 with slt $2,$18,$5 and no move $16,$18.

- [s59] Form L is the first form in 59 sessions whose cse1 dump carries BOTH target operands: dumps_L/fn.cse line 44 (ashift:SI (reg/v:SI 72) (const_int 4)) and line 79 (lt:SI (reg/v:SI 75) (reg/v:SI 78)).

- [s59] Form L's undoing is two-step and both steps are dumped: fn.loop shows LICM emitting the shift into the carrier's preheader as a fresh insn (uid -1) still reading reg 72, and fn.cse2 then prints (ashift:SI (reg/v:SI 75) ...) because the hoisted insn is back inside the first extended block. Form L measures 4 / 86.

- [s59] MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so global allocation walks hard registers 0,1,2,...,31; a pseudo that crosses no call therefore lands on a low call-clobbered register long before any $s register.

- [s59] src/text1a_c.c was restored to HEAD at the end of the session; the working tree carries only the two ledger files and metrics/events.jsonl.
