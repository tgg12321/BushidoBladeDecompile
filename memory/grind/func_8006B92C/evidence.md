# Evidence bank - func_8006B92C

## Baseline (s1, 2026-07-28)
- canonical: verdict=C, distance=15, target_insns=143 (pure-C target, well under 50 threshold)
- sandbox `--disable all`: score=15, target=143 insns, build=139 insns (4-insn shortfall)
- Current C body carries `register u32 var_v1 asm("v1");` pin (CHEAT, score-inert). Pin is stripped by cheat-invisible sandbox so 15 is the honest floor. Body starts s1 as candidate baseline (unchanged).
- No sibling duplicates found (`find_duplicates.py` empty for this func). Caller `func_8006C168` (text1b.c:15897) is the only cross-file relationship.

## Structural diff, build vs target (case-1 / case-2 arms, artifact `tmp/grind/func_8006B92C/s1/build_disasm.txt` vs `target_disasm.s`)

**Root mechanism: `jump2` find_cross_jump merged the two `D_800A34F8 = ...` stores into a shared tail. Target does NOT share.**

Target (case 1):
```
lw a0,gp; addiu v0,zero,0x4000; andi v1,a0,0xE000
bne v1,v0,.else
 lui v1,0xFFFF          <-- delay: dead-branch-scheduled first insn of else arm
# then arm (inline, own store)
lui v0,0xFFFF; ori v0,v0,0x1FFF; and v0,a0,v0
sw v0,gp                 <-- inline sw, not shared
j .after; addu a0,0,0
.else:
ori v1,v1,0x1FFF         <-- consumes the delay-slot lui
and v1,a0,v1; srl v0,a0,13; andi v0,v0,7
j .join; addiu v0,v0,1
```

Build (case 1):
```
lw a0,gp; li v0,0x4000; andi v1,a0,0xE000
bne v1,v0,.else
 nop                     <-- BUILD: no dead-branch sched, empty delay
# then arm (jumps to shared sw)
lui v0,0xFFFF; ori v0,v0,0x1FFF
j .shared_sw
 and v0,a0,v0            <-- delay
.else:
lui v0,0xFFFF; ori v0,v0,0x1FFF; and v1,a0,v0; srl v0,a0,13; andi v0,v0,7
j .join; addiu v0,v0,1
.shared_sw:
sw v0,gp                 <-- SHARED, jump2-merged
```

Same shape mirrored in case 2 (`.L8006B9D8` vs build). The 4-insn shortfall is explained by:
- 2 lui-into-delay dead-branch schedules that don't happen in build
- 2 inlined `sw D_800A34F8` stores that were merged to a single shared store

## Search space (mechanism-grounded)

1. **Cross-jump-store-tail-merge** (rule: cross-jump-store-tail-merge). The `D_800A34F8 = ...` writes are identical bytes across the two arms of each case; jump2's find_cross_jump merges them into a shared tail (build's `j .shared_sw`). Target keeps them inline per arm. Levers: mix exit forms (`goto endK` in one arm, inline `return`/`break` in another), or the sanctioned `duplicated-statement-into-arms` pattern.
2. **Dead-branch scheduling of `lui $v1`.** Target hoists the first insn of the else-arm (`lui $v1, 0xFFFF`) into the branch-taken delay slot -- classic reorg.c dead-branch-scheduling that only fires when the branch-taken path proves $v1 dead. Once the cross-jump merge is broken (item 1) and the arms become distinct, the natural scheduling may reappear without a pin.
3. **32-bit constant split.** The mask `0xFFFF1FFF` is emitted as `lui+ori` -- fine. Its low-half `ori` is what target uses in the else-arm as the first live insn; whether GCC can be steered to hoist just the `lui` is what enables the delay-slot fill.

## Constraints
- The `register u32 var_v1 asm("v1")` pin currently in src is CHEAT (inline-asm-policy expanded catalog, register-asm pins). It is score-inert -- must be removed as part of any COMPLETED-C form.
- Second `switch(idx)` (post-jal region) matches; the residual is concentrated in the pre-jal `switch(ret)` region.

- [s1] canonical verdict C, distance 15, target_insns 143

- [s1] sandbox --disable all score 15, build_insns 139, 4-insn shortfall vs target

- [s1] source line src/text1b.c:15695-15761 carries `register u32 var_v1 asm("v1")` pin (cheat, score-inert)

- [s1] find_duplicates.py returns no siblings for func_8006B92C

- [s1] objdump diff (tmp/grind/func_8006B92C/s1/build_disasm.txt vs target_disasm.s): case-1 and case-2 D_800A34F8 stores are jump2-merged in build; target inlines them per arm with `lui $v1` dead-branch-scheduled into the branch delay slot

- [s1] caller func_8006C168 (text1b.c:15897) uses forward decl `extern s32 func_8006B92C()` -- signature-affecting edits must keep that callable

- [s2] s2 baseline (pin removed, otherwise s1 body): score=15, target_insns=143, build_insns=139 — pin is inert as expected

- [s2] H1d best form: shared `do_call` label after both cases + shared `complete_store` label reachable from case-1-else via goto and case-2-else via fall-through; case-1-then and case-2-then use inline `D_800A34F8 = a0 & 0xFFFF1FFF` (or `|0x4000`) stores. score=10, build_insns=140

- [s2] Structural target-match achieved on store SHAPE: 3 sw sites (10de0 case-1-then, 10e1c case-2-then, 10e48 shared) matching target's 3 sites (5C1B4, 5C1FC, 5C228)

- [s2] Remaining 3-insn deficit is target's mask (`lui/ori/and`) computed PER-ARM inside each else block, while H1d hoists it to the shared complete_store. Target case-1-else: `ori/and v1; srl; andi; j; addiu +1` (6 insns after delay-slot lui); H1d case-1-else: `andi; j; addiu +1` (3 insns after delay-slot srl).

- [s2] Coupling: per-arm mask compute (via explicit var_v1) makes jump2 re-merge all 4 stores because the final OR result register aligns to $v0 in all paths (removes the sw-source-reg divergence H1d exploits). Confirmed by H1e disasm.

- [s2] delay-slot fill: target hoists `lui $v1,0xFFFF` (else-arm's mask lui) into the bne/bnez branch-taken delay slot as classic reorg.c dead-branch-scheduling; H1d hoists `srl $v0,$a0,0xd` (else-arm's shift) instead. Both fill the slot but with different insns.

- [s2] Second `switch(idx)` post-jal region is unchanged and matches; residual is entirely in the pre-jal switch(ret) region.

- [s2] Artifacts: tmp/grind/func_8006B92C/s2/build_h1a.txt (H1a=17 disasm), build_h1b.txt (H1b=19), build_h1d_final.txt (H1d=10, best), build_h1e_v2.txt (H1e=15 regressed)

- [s3] [s3] baseline (applied s2 candidate.c to src): score=10, target_insns=143, build_insns=140

- [s3] [s3] h2a split-init |=: score=6, build_insns=141 (added var_v1 decl + per-arm `var_v1 = a0 & 0xFFFF1FFF;` + `var_v1 |= ...; D_800A34F8 = var_v1;` at complete_store)

- [s3] [s3] Comparing h2a build (tmp/grind/func_8006B92C/s3/build_h2a_final.txt) vs target (asm/funcs/func_8006B92C.s): shared complete_store now matches target byte-for-byte (`andi v0; sll v0; or v1,v1,v0; sw v1,gp` -- source $v1). Per-arm mask compute matches structurally in else arms.

- [s3] [s3] Residual 6 = 2 insns length + ~4 register substitutions. Target has TWO `lui 0xFFFF` per case (one for else-arm mask in delay slot via dead-branch-scheduling, one redundant for then-arm mask); build shares ONE lui $v0 in delay slot (fall-through fill for then arm) that both then and else reuse. Register subst: target's else uses $v1 for mask ori (from delay-slot lui $v1); build's else uses $v0 (from shared lui).

- [s3] [s3] Fill-priority root: reorg.c fill_from_thread picks then-arm's `lui $v0` (fall-through fill) over else-arm's `lui $v1` (dead-branch fill) because fall-through comes first in the priority order when both regs are dead on the opposite thread. No pure-C structural lever tested this session flips the choice (h2b `!=` broke shared do_call; h2c temp was DCE'd).

- [s3] [s3] h2b (branch-sense flip) reproduces s2 h1c's failure mode -- KILLED family. Not attempted again with different scaffolding since h2a base already flipped from raw h1c's shape (h1c also removed shared do_call; h2b kept it and still regressed hard due to jump-threading).

- [s3] [s3] artifacts: tmp/grind/func_8006B92C/s3/build_baseline.txt (score 10), build_h2a.txt + build_h2a_final.txt (score 6)

- [s4] [s4] baseline: applied s3 candidate to src, sandbox --disable all -> score 6 target_insns 143 build_insns 141 (unchanged from s3)

- [s4] [s4] H4a (subu algebraic: `var_v1 = a0 - (a0 & 0xE000)` in else arms). Algebraically equivalent to `a0 & 0xFFFF1FFF` (subtracts exactly the bits within 0xE000). Result: score 6 -> 17 REGRESSED. Mechanism: reuses the pre-compared `$v1 = a0 & 0xE000` -- no fresh lui $v1 birth -> no dead-branch-fillable lui candidate. Also destroys the sw-source-reg divergence that H1d/h2a preserved. KILLED.

- [s4] [s4] H4b (XOR algebraic: `var_v1 = a0 ^ (a0 & 0xE000)` in else arms). Algebraically identical to H4a (XOR affects only bits within the mask, both zero out). Result: score 6 -> 17 REGRESSED. Same mechanism as H4a: reuse of $v1 kills the fresh lui birth. KILLED.

- [s4] [s4] H4c (duplicated-statement-into-arms per rule: put `func_8005C650(0,0x7F,0x7F); break;` directly in each case instead of the shared `do_call:` label). Result: score 6 -> 14 REGRESSED. Mechanism: jump2 re-merges the duplicated jal sites since args are byte-identical, but the register-allocation cascade upstream shifts (var_v0/var_v1 pseudos re-prioritize) and undoes the h2a store-source $v1 alignment. Byte-neutrality NOT preserved for this func's cross-jump interaction. KILLED.

- [s4] [s4] H4d/H4e (shift-form then-arm mask: `(a0 & 0xFFFF0000) | ((a0 << 19) >> 19)` replacing `a0 & 0xFFFF1FFF` in case-1 then arm only / both then arms). Intent: emit `andi/sll/srl/or` (no lui) in then arms so reorg loses the fall-through `lui $v0` fill candidate and falls back to else's `lui $v1`. Result: score 6 -> 7 (H4d) / 6 -> 8 (H4e) REGRESSED. combine.c either folds the shift form back to `lui+ori+and` or emits enough extra insns that fill priority doesn't flip AND the mask-insn count grows. KILLED.

- [s4] [s4] Permuter campaign s4-PERMGEN-fresh (14067 iters, ~6 min, 6 jobs, PERM_GENERAL macros around per-arm mask + shift compute in both else arms). All 5 novel finds this campaign (output-115-3, 205-1, 175-2, 155-5, 220-3) are variants of the forbidden split-load-anchor cheat: (a) `var_v1 = 0xFFFF1FFF; var_v1 = a0 & var_v1;`, (b) `var_v1 = a0 & (v = 0xFFFF1FFF);` from an earlier campaign run, (c) `new_var3 = 13; var_v0 = new_var3; var_v0 = ((a0 >> var_v0) & 7) - 1;` (same reg_n_sets-bump pattern on the shift count). All rejected per Judge s3-BINDING constraint (no-new-park-categories §"Auto-search tools ... output is PROPOSALS"). No non-cheat closing form was surfaced. Prior campaign (pre-s4) at 29388 iters converged on identical family.

- [s4] [s4] scan_hand_coded.py --single func_8006B92C: HAND_CODED tier=LOW score=0/8 (no S1/S2/S6 STRONG signals). Standing-ruling Gate 1 FAILS.

- [s4] [s4] Standing-ruling Gate 2 (SOTN precedent for the specific dual-lui + fill-priority + reg_n_sets closing family with pure-C annotation) NOT identified. cross-jump-store-tail-merge rule's own text names saEft00Add as a "documented coupled fixpoint" precedent internal to BB2 (not SOTN). No citable SOTN master-branch precedent that legitimately reproduces the reg_n_sets-driven fresh-lui + dual-store shape without the split-load-anchor idiom.

- [s4] [s4] artifacts: tmp/grind/func_8006B92C/s4/baseline_s3_body.c; permuter/func_8006B92C/output-* (20 pre-s4 + 5 s4-new, all cheat-basin); memory/grind/func_8006B92C/rejected/{h4a_subu_algebraic.c, h4b_xor_algebraic.c, h4c_dup_do_call.c, h4d_shift_mask_thenarm.c, s4_perm_split_init_115.c, s4_perm_new_var3_shift_const.c}.

- [s4] s4 baseline: applied s3 candidate to src, sandbox --disable all -> score 6, target_insns 143, build_insns 141 (unchanged from s3).

- [s4] scan_hand_coded --single func_8006B92C: tier=LOW score=0/8 (no S1/S2/S6 STRONG signals). Standing-ruling Gate 1 FAILS.

- [s4] No citable SOTN master-branch precedent for the specific dual-lui + reorg-fill-priority + reg_n_sets closing shape without the split-load-anchor idiom. cross-jump-store-tail-merge.md names saEft00Add as internal 'documented coupled fixpoint' precedent for OPEN, not closed. Standing-ruling Gate 2 FAILS.

- [s4] H4a subu / H4b XOR / H4c dup-do-call / H4d,H4e shift-mask: 4 legitimate axes measured, all regressed vs floor=6. KILLED with sandbox measurements banked in rejected/.

- [s4] Fresh-seed permuter (14067 iters this session; 29388 pre-s4): only surfaced Judge-bound split-load-anchor variants. Prior s4 attempts (Judge FAILs 18:00 and 18:26) explicitly ruled this family cheat-by-any-spelling.

- [s4] Src state restored to s3 h2a baseline (src/text1b.c:15695-15761): score 6, no pins, no cheat-asm, no rules.

- [s4] docs/grind/decisions.md updated with OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27) entry naming func_8006B92C.

- [s5] IMPORTANT INHERITANCE GOTCHA: src/text1b.c did NOT carry the s3/s4 candidate at session start — the tree was git-clean at the s1 body (with the `register u32 var_v1 asm("v1")` pin and the merged `D_800A34F8 = var_v1 | (...)` stores). s4's "src state restored to s3 h2a baseline" claim describes an intent, not the committed tree. ALWAYS apply memory/grind/func_8006B92C/candidate.c to src before measuring. After applying it, s5 re-measured the floor: sandbox --disable all score=6, target_insns=143, build_insns=141 (matches s3/s4).

- [s5] Permuter metric calibration for this function: base score 235 decomposes EXACTLY as the sandbox residual — 2 missing instructions x 100 (ins/del weight) + 7 register diffs x 5 = 235. So the permuter score is a faithful gradient here, and "no permuter improvement" is real evidence about the closing form, not metric noise.

- [s5] decomp-permuter's per-pass weights are settable from settings.toml `[weight_overrides]` keyed by the pass function's `__name__` (main.py:337 merges them over `get_default_randomization_weights(compiler_type)`; Randomizer.__init__ requires every RANDOMIZATION_PASSES entry to have a weight). This makes it possible to run a CHEAT-SUPPRESSED campaign: weight 0.0 on the passes whose OUTPUT SHAPE is itself a catalog cheat (perm_temp_for_expr, perm_split_assignment, perm_chain_assignment, perm_long_chain_assignment, perm_duplicate_assignment, perm_add_self_assignment, perm_pad_var_decl, perm_dummy_comma_expr, perm_add_mask, perm_xor_zero, perm_mult_zero, perm_refer_to_var, perm_empty_stmt, perm_ins_block). This is a reusable technique for any function whose permuter basin is a cheat family — the weights file is banked at tmp/grind/func_8006B92C/s5/nocheat_weights.toml.

- [s5] Chassis A (cheat-suppressed random mode, label s5-NOCHEATPASS, 6 jobs): 41424 iterations / 1293 s, ZERO finds — not one legitimate-family mutation ever scored below base 235. The s3 h2a form is a STRICT local minimum under the legitimate mutation set. This is the direct measurement s4's frontier item #2 ("directed-PERM permuter from h2a base can find the exact spelling") asked for: it cannot, and the reason s4's search "worked" is that its finds were all produced by cheat-shape passes.

- [s5] Chassis B (directed exhaustive spelling sweep, label s5b-DIRECTED-ALTS): 288/288 combinations, 0 compile errors, min score 235 == base. Sites swept: case-1 then-arm store {`a0 & 0xFFFF1FFF` | `D_800A34F8 &= 0xFFFF1FFF` | `a0 - 0x4000`}, case-2 then-arm store {`(a0 & 0xFFFF1FFF) | 0x4000` | `a0 | 0x4000` | `D_800A34F8 |= 0x4000`}, both counters {`((a0>>13)&7)±1` | `(a0>>13)±1`}, the complete_store OR {`((var_v0 & 7) << 13)` | `(var_v0 << 13) & 0xE000`}, and PERM_LINESWAP over each else arm's statement order. The two algebraic simplifications are exact on their own arms (case-1 else is entered only when `a0 & 0xE000 == 0x4000`; case-2 then only when `a0 & 0xE000 == 0`), so this sweep also independently re-confirms that arm-conditioned algebra does not help — consistent with s4's H4a/H4b kills, reached by a different route.

- [s5] Chassis C (directed exhaustive structural sweep, label s5c-STRUCT-ALTS): 16/16 whole-case-body combinations, min 235 == base. Per-arm duplicated `D_800A34F8` read ([[split-read-defeats-hoist]]) ties at 235 — GCC CSEs the repeated non-volatile global loads because each arm's own store to D_800A34F8 comes after both reads, so nothing invalidates them and the two `lui 0xFFFF` births re-merge exactly as before. Hoisted shared mask ([[hoist-shared-arm-computation-defeats-copy-pref]]) scores 285 (worse): the pre-branch birth removes the per-arm mask compute h2a's floor-6 depends on. Both banked in rejected/.

- [s5] All three campaigns harvested with --stop; `procs_killed` 7/0/0, no campaign left running. Artifacts: tmp/grind/func_8006B92C/s5/{chassisA_head.txt, chassisB_base.c, chassisB_scores.txt, chassisC_base.c, chassisC_scores.txt, nocheat_weights.toml}.

- [s5] Judge s3-BINDING constraint #1 DISCHARGED: both staged-value sites in src/text1b.c now carry the full staged-value-reused-variable prong-4 template — named mechanism (sched.c adjust_priority / birthing_insn_p; reg_n_sets[var_v1] != 1 disables the load-late launch priority; the `|=` keeps the final OR in var_v1's home register so the shared store's source $v1 diverges from the then-arm stores' $v0 and jump2 find_cross_jump cannot rtx_equal-merge the three sw sites) plus the lever-exhaustion citation to hypotheses.md/evidence.md sessions s1-s3. Re-sandboxed after the annotation: score still 6 (comments are codegen-neutral, as expected). candidate.c updated to match src exactly.

- [s5] INHERITANCE GOTCHA: src/text1b.c was git-clean at the s1 body (register u32 var_v1 asm("v1") pin + merged stores) at session start — s4's 'src restored to s3 h2a baseline' described intent, not the tree. Always apply memory/grind/func_8006B92C/candidate.c to src before measuring.

- [s5] Floor re-measured after applying the s3 candidate: sandbox --disable all score=6, target_insns=143, build_insns=141 (unchanged from s3/s4).

- [s5] Permuter metric is faithfully calibrated for this function: base score 235 == 2 missing insns x 100 (ins/del weight) + 7 register diffs x 5. So 'no permuter improvement' is real evidence about the closing form, not metric noise.

- [s5] decomp-permuter per-pass weights are settable from settings.toml [weight_overrides] keyed by the pass function's __name__ (main.py:337; Randomizer.__init__ demands a weight for every RANDOMIZATION_PASSES entry). This enables a reusable CHEAT-SUPPRESSED campaign for ANY function whose permuter basin is a cheat family; the weights file is banked at tmp/grind/func_8006B92C/s5/nocheat_weights.toml.

- [s5] Chassis A (cheat-suppressed random, 41424 iters / 1293 s, 6 jobs): ZERO finds. Directly answers s4's frontier item #2 ('directed permuter from h2a base can find the exact spelling'): it cannot — s4's search only 'worked' because its finds were produced by cheat-shape passes.

- [s5] Chassis B (288 exhaustive spelling combinations): min 235 == base. Chassis C (16 exhaustive whole-case structural combinations): min 235 == base, hoisted-mask family 285.

- [s5] All three campaigns harvested with --stop (procs_killed 7/0/0); permuter_campaign status shows alive=false for all three. No campaign outlives the session.

- [s5] Judge s3-BINDING constraint #1 DISCHARGED: both staged-value sites in src/text1b.c now carry the full staged-value-reused-variable prong-4 template — named mechanism (sched.c adjust_priority / birthing_insn_p; reg_n_sets[var_v1] != 1 disables the load-late launch priority; the |= keeps the final OR in var_v1's home register so the shared store's source $v1 diverges from the then-arm stores' $v0 and jump2 find_cross_jump cannot rtx_equal-merge the three sw sites) plus the lever-exhaustion citation to hypotheses.md/evidence.md s1-s3. Re-sandboxed after: score still 6. candidate.c matches src exactly.

- [s5] Judge s3-BINDING constraint #2 respected: no split-load-anchor form was proposed, re-surfaced, or measured this session; the cheat-shape mutation passes were structurally disabled so the search could not even emit one.

## [s6] FORENSICS — the residual, named exactly (cc1 -da, dumps under tmp/grind/func_8006B92C/s6/dumps_h2a/)

- [s6] Harness: `tmp/grind/func_8006B92C/s6/dump.sh <tag>` runs the real build's cpp+cc1 flags on the WHOLE src/text1b.c with `-da`, dropping every RTL dump into tmp/grind/func_8006B92C/s6/dumps_<tag>/; `tmp/grind/func_8006B92C/s6/extract.py <dir> func_8006B92C <pass>...` slices one function out of a dump (dumps are 1.5-3.6 MB each — never read one whole). Reusable for any function in any file.

- [s6] THE 2-INSN SHORTFALL IS NOT A FILL-PRIORITY DECISION. The s3/s5 frontier hypothesis (reorg.c picks the fall-through fill over the dead-branch fill) is FALSIFIED by the dbr dump. In `func_8006B92C.dbr` the case-1 branch appears as `(insn 363 (sequence[ (jump_insn 57 ... (ne v1 v0) -> label 68) (insn 314 (set (reg:SI 2 v0) (const_int -65536))) ]))` — insn 314 is the ELSE arm's `lui $v0,0xFFFF`, i.e. reorg.c ALREADY fills the slot from the branch-taken (dead-branch) thread, exactly like target. What differs is what happens next: the then-arm's own `lui` (insn 312) is GONE from the dbr dump (`grep -c "insn 312 " -> 0`), deleted by reorg.c's redundant_insn because the delay-slot insn sets the SAME hard register ($v0) to the SAME constant that the fall-through path was about to set. Target keeps both luis only because its else-arm constant lives in $v1 (`lui $v1` in the delay slot, `ori $v1` / `and $v1,$a0,$v1` in the arm) and so does not cover the then-arm's `lui $v0`.

- [s6] THE DECIDING FACT IS A LOCAL-ALLOC REGISTER CHOICE. In the .lreg/.greg dumps the else-arm mask constant is pseudo 92 (case 1) — a basic-block-local pseudo, so it is allocated by local-alloc.c, which runs BEFORE global.c. `;; Register dispositions: ... 92 in 2` = $v0. The AND's destination is pseudo 78 = var_v1, a GLOBAL pseudo (`;; 7 regs to allocate: 75 81 76 78 77 79 73`), allocated later to $v1 (`78 in 3`). Because local-alloc cannot tie a local qty to a global pseudo (combine_regs only merges qtys of block-local regs), the constant never inherits var_v1's register; local-alloc.c find_free_reg walks REG_ALLOC_ORDER and takes $v0, which is free over the constant's 3-insn range (the counter chain's temps 93/94 are born only AFTER the AND, so they do not conflict — they get $v0 too, harmlessly).

- [s6] WHY NO STATEMENT-ORDER LEVER REACHES IT. sched1 (sched.c, runs before local-alloc) normalises the else block to mask-chain-first regardless of source order: in the P6b .lreg dump the mask chain carries insn numbers 316/317/76 while the shift chain carries 71/72/79 — i.e. expand emitted the shift FIRST (lower uids) and the scheduler moved the mask above it. This is the mechanical reason s5's PERM_LINESWAP sweep over the two else-arm statements tied at base, and why any "compute the counter first" spelling is inert: the constant is always the block's leading birth, so nothing is ever live in $v0 across it.

- [s6] P6a (ONE function-scope temp `t` shared by both else arms, holding the shift value): score 6 -> 12 BUT build_insns 141 -> 143 == target_insns for the first time in this function's history. Sharing the temp across two blocks makes it a GLOBAL pseudo (lands in $a1), so the else block's leading insn after sched1 is the `srl`, reorg fills the delay slot with `srl $v0,$a0,0xd`, and the then-arm `lui $v0` is no longer redundant and survives. Proves the count closes the moment the delay-slot insn stops duplicating the then-arm's constant in the same hard register. Rejected on registers (counter split across $a1 where target keeps it in $v0). Banked: rejected/s6_shared_global_temp_t.c.

- [s6] P6b (per-arm block-local temps t1/t2, shift computed before the mask): score 6, build_insns 141 — identical to the h2a base. .lreg shows t1 = pseudo 79 live [72..79], constant pseudo 95 live [316..76]: disjoint, no conflict, constant keeps $v0. KILLED. Banked: rejected/s6_perarm_local_temps.c.

- [s6] P6c (counter hoisted above the `if` so the GLOBAL var_v0 is live across the branch and across the mask): score 6 -> 19, build_insns 139. global.c cannot reserve a register against local-alloc (it runs after), and the pre-branch `li v0,0x4000` pushes var_v0 to $a1 anyway; with the counter gone from the arms the shared `lui $v0` returns to the delay slot and BOTH then-arm luis die (4 short). KILLED. Banked: rejected/s6_hoisted_counter_prebranch.c.

- [s6] Consequence for the frontier: the only remaining shape that produces target's bytes is "else-arm mask constant allocated to $v1". Under local-alloc's rules that requires either (a) a BLOCK-LOCAL value in $v0 live across the constant's range — which sched1 structurally prevents by hoisting the mask chain to the head of the block — or (b) the constant and var_v1 being the SAME pseudo, which is precisely the split-load-anchor form the Judge banned by any spelling. Every legitimate lever must therefore attack sched1's decision to hoist the mask chain above the shift chain in the else block, NOT the arm's statement order, NOT the arithmetic spelling, and NOT reorg.c.

- [s6] Src restored to the s5/h2a candidate before finishing; re-measured sandbox --disable all = score 6, target_insns 143, build_insns 141.

- [s6] Forensics harness built and banked: tmp/grind/func_8006B92C/s6/dump.sh runs the build's exact cpp+cc1 flags over the whole src/text1b.c with -da; tmp/grind/func_8006B92C/s6/extract.py slices one function out of a 1.5-3.6 MB dump. Reusable for any function in any file.

- [s6] dbr dump, case 1: `(insn 363 (sequence[ (jump_insn 57 ... (ne (reg v1) (reg v0)) -> label 68) (insn 314 (set (reg:SI 2 v0) (const_int -65536))) ]))` — the delay slot is filled from the ELSE thread, matching target's fill choice. `grep -c 'insn 312 '` on the dbr dump returns 0: the then-arm lui was deleted as redundant.

- [s6] greg dump, case 1: `;; 7 regs to allocate: 75 81 76 78 77 79 73`; `;; Register dispositions: 73 in 17  75 in 3  76 in 3  77 in 2  78 in 3  79 in 16 ... 92 in 2`. Pseudo 92 = the else-arm mask constant (block-local, $v0); pseudo 78 = var_v1 (global, $v1); pseudo 77 = var_v0 (global, $v0, `;; 77 preferences: 2`).

- [s6] sched1 normalises the else block to mask-chain-first: in dumps_p6b/func_8006B92C.lreg the mask chain has uids 316/317/76 and the shift chain 71/72/79, i.e. the lower-uid (earlier-expanded) shift chain sits BELOW the mask chain after scheduling.

- [s6] P6a measured: sandbox score 12, build_insns 143 == target_insns 143 (first count parity ever recorded for this function). P6b measured: score 6, build_insns 141 (no change). P6c measured: score 19, build_insns 139.

- [s6] Three disproven forms banked: memory/grind/func_8006B92C/rejected/{s6_shared_global_temp_t.c, s6_perarm_local_temps.c, s6_hoisted_counter_prebranch.c}, each carrying its measurement and its GCC-pass explanation.

- [s6] src/text1b.c restored to the s5/h2a candidate before finishing; re-measured sandbox --disable all = score 6, target_insns 143, build_insns 141. candidate.c header updated with the s6 mechanism summary; no C substance changed.

- [s6] Closing constraint derived from the dumps: the only shape that yields target's bytes is 'else-arm mask constant allocated to $v1'. local-alloc can reach that only via (a) a BLOCK-LOCAL value live in $v0 across the constant's range — which sched1 structurally prevents by hoisting the mask chain to the head of the block — or (b) the constant and var_v1 being the SAME pseudo, i.e. the split-load-anchor form the Judge banned by any spelling. So the next legitimate lever must act on sched1's hoist decision, not on statement order, arithmetic spelling, or reorg.c.

## [s7] forensics -- measured local-alloc quantity table for func_8006B92C (h2a base, floor 6)
Tooling note for future sessions: tmp/gccdbg/cc1 is the instrumented compiler.
The production tools/gcc-2.7.2/build/cc1 is a May-18 binary that PREDATES the
BB2_* env probes in the sources (local-alloc.c BB2_QTY_DEBUG, sched.c
BB2_RANK_DEBUG, global.c BB2_FINDREG_DEBUG, all added early July) and prints
nothing -- s7 lost a turn to that. Running tmp/gccdbg/cc1 WITHOUT -quiet makes
it echo each function name to stderr, which is what lets the QTYDBG stream be
sliced to one function (harness: tmp/grind/func_8006B92C/s7/qty2.sh).

Every block-local quantity in the two switch cases, with the hard register
local-alloc gave it (got=), from tmp/grind/func_8006B92C/s7/qty_h2a/qty_func.log:

  blk=3 (case-1 compare)   qty1 reg1=89 birth=8  death=10 refs=2 got=2   ($v0 = the 0x4000 constant)
  blk=3                    qty0 reg1=88 birth=6  death=10 refs=2 got=3   ($v1 = a0 & 0xE000)
  blk=4 (case-1 THEN arm)  qty0 reg1=90 birth=2  death=8  refs=4 got=2   (constant TIED to and-dest -> one $v0 qty)
  blk=5 (case-1 ELSE arm)  qty1 reg1=94 birth=10 death=14 refs=4 got=2   (srl+andi counter chain)
  blk=5                    qty0 reg1=92 birth=4  death=8  refs=2 got=2   (lui/ori mask constant)

blk=3 already matches target exactly ($v1 for the andi temp, $v0 for the compare
constant) and does so purely by the priority formula in local-alloc.c
qty_compare_1: priority = floor_log2(refs)*refs*size / (death-birth). The 0x4000
constant (2 refs over 2 slots, pri 1.0) is allocated before the andi temp (2 refs
over 4 slots, pri 0.5) and takes $v0, pushing the andi temp to $v1. That is the
same mechanism that hands the else-arm mask constant $v0.

Decisive structural fact: local-alloc.c find_free_reg computes
used = OR over ins in [birth,death) of regs_live_at[ins], then walks hard
registers 0,1,2,... (mips.h defines no REG_ALLOC_ORDER) and returns the first
free one. regs_live_at holds HARD registers only. var_v0, var_v1 and a0 are all
GLOBAL pseudos, assigned in global.c which runs AFTER local-alloc, so they can
never reserve a register against a block-local quantity. The else arms contain
no call and no hard-register reference. Therefore the ONLY way to deny the mask
constant $v0 is another BLOCK-LOCAL quantity whose live range covers it.

s7 measured that this is unreachable. P7a (counter chain lengthened to 4 insns,
mask still first) and P7b (counter chain lengthened AND moved first) both leave
the two chains emitted contiguously, so the quantity ranges are disjoint in both
orders ([4,8) vs [10,16) in P7a; [4,10) vs [12,16) in P7b) and the constant gets
$v0 either way. P7a is score-identical to the base (6 / 141); P7b regresses to
12 / 140. This kills the s6 live-frontier lever ("lengthen the counter's
dependence path / shorten the mask's") with a direct measurement instead of an
inference, and it re-explains s6's P6b result without appealing to scheduler
hoisting: even when the counter IS emitted first, the ranges never overlap,
because nothing makes the scheduler interleave two independent 1-cycle ALU
chains on r3000.

The contrast that shows the shape target needs: in the THEN arm (blk=4) there is
no separate constant quantity at all -- combine_regs merged the constant pseudo
into the and-destination pseudo because BOTH are block-local, so the arm emits
lui $v0 / ori $v0 / and $v0,$a0,$v0 out of ONE quantity. Target's else arm is
exactly that shape one register over: lui $v1 / ori $v1 / and $v1,$a0,$v1. In our
build the else arm's and-destination is var_v1, a global pseudo, and combine_regs
cannot tie a block-local quantity to a global one -- so the constant must be its
own quantity and, by the rule above, must be $v0. The two-instruction gap follows
directly: reorg.c already fills each bne/bnez delay slot from the else
(dead-branch) thread exactly like target (s6 finding), but because the
delay-slot lui writes the SAME hard register with the SAME constant as the then
arm's lui, redundant_insn deletes the then-arm one.

Artifacts: tmp/grind/func_8006B92C/s7/{qty2.sh, qty_h2a/, qty_p7a/, qty_p7b/}.

- [s7] tmp/gccdbg/cc1 is the instrumented compiler; tools/gcc-2.7.2/build/cc1 is a May-18 binary that predates the BB2_QTY_DEBUG / BB2_RANK_DEBUG / BB2_FINDREG_DEBUG probes in the sources and prints nothing. Run the instrumented cc1 WITHOUT -quiet so it echoes function names to stderr -- that is what lets a QTYDBG stream be sliced to one function (harness: tmp/grind/func_8006B92C/s7/qty2.sh).

- [s7] Measured local-alloc quantity table (h2a base): blk=3 qty1 reg1=89 birth=8 death=10 refs=2 got=2 / qty0 reg1=88 birth=6 death=10 refs=2 got=3; blk=4 qty0 reg1=90 birth=2 death=8 refs=4 got=2; blk=5 qty1 reg1=94 birth=10 death=14 refs=4 got=2 / qty0 reg1=92 birth=4 death=8 refs=2 got=2.

- [s7] local-alloc.c qty_compare_1 priority = floor_log2(refs)*refs*size / (death-birth); higher priority is allocated first. This alone explains blk=3 matching target: the 0x4000 constant (pri 1.0) is allocated before the andi temp (pri 0.5), takes $v0, and pushes the temp to $v1.

- [s7] mips.h defines no REG_ALLOC_ORDER, so find_free_reg scans hard registers in numeric order; $v0 (2) is the first allocatable GPR, which is why every uncontested block-local quantity in this function lands in $v0.

- [s7] regs_live_at in local-alloc holds HARD registers only. global.c runs after local-alloc, so a global pseudo (var_v0, var_v1, a0) can never reserve a register against a block-local quantity -- this independently re-kills s6's P6c 'hoist the counter to give var_v0 a spanning range' family and explains why P6b was inert.

- [s7] P7a (counter chain lengthened, mask still first) is exactly score-inert: 6 / 141, identical to base. P7b (counter chain lengthened and emitted first) regresses to 12 / 140. Neither changes the constant's register.

- [s7] The scheduler emits the arm's two independent 1-cycle ALU dependence chains contiguously in both source orders; there is no interleaving, so the two block-local quantity ranges are always disjoint.

- [s7] Session start floor and end floor are both 6 (target_insns 143, build_insns 141); candidate.c body is unchanged and re-verified in src at the end of the session.

## [s8] rederive — MATCH. floor 6 -> 0. func_8006B92C is COMPLETED-C.

- [s8] PROVENANCE. The driver's log shows three s8 sessions started and all three
  were discarded as INVALID ("no outcome file / unparseable JSON"), then the
  grinder circuit-broke (commit ff0792b2, docs/grind/INCIDENT.md, "3x transient
  API 500"). The FIRST of those three (14:39-14:47) had already derived and
  written the closing form into memory/grind/func_8006B92C/candidate.c before it
  died; that file was the only survivor (the driver reverted src/text1b.c to the
  s1 body, pin included). THIS s8 session did not inherit the claim on trust: it
  re-applied candidate.c to src/text1b.c and re-measured everything from scratch.

- [s8] MEASURED THIS SESSION with the form in src/text1b.c:15693-15761:
  `sandbox func_8006B92C --disable all` -> score 0, target_insns 143,
  build_insns 143, scorable true, rules_dropped 0, cheat_asm_stripped 324
  (file-wide, none in this function). `canonical func_8006B92C` -> verdict C,
  distance 0, asm_insns 0. `verify-oracle` -> "ok": true, i.e. the full clean
  build+link SHA1 still equals the oracle
  62efab4f73f992798c43e8c730aa43baa10bb4fa WITH the new body in place. The
  `register u32 var_v1 asm("v1")` pin that the tree carried since s1 is GONE,
  and so are both /* FAKE */ staged-value annotations, because the construct
  they annotated no longer exists. Zero regfix/asmfix rules, zero inline asm.

- [s8] THE CLOSING FORM (full body banked in candidate.c). Delete the shared
  `complete_store:` label and the two function-scope staging variables
  (`var_v1` / `var_v0`) that every session s3-s7 was built on. In each else arm
  declare TWO BLOCK-LOCAL variables and do the OR/store per arm:
      } else {
          u32 m = a0 & 0xFFFF1FFF;
          s32 c = ((a0 >> 13) & 7) + 1;   /* - 1 in case 2 */
          m |= (c & 7) << 13;
          D_800A34F8 = m;
      }
  The shared `do_call:` label is RETAINED (s4 H4c measured that duplicating the
  `func_8005C650(0, 0x7F, 0x7F)` call into each case regresses 6 -> 14).

- [s8] WHY IT CLOSES — the s7 QTYDBG account, inverted. s7 measured that the
  whole residual was local-alloc.c's hard-register choice for the else-arm mask
  constant, and s7's own conclusion named the untested class exactly: "make the
  else arm's and-DESTINATION block-local so combine_regs ties the constant to it
  as in blk=4, while still producing three distinct store sites." That is what
  `u32 m` does. In the THEN arm (blk=4) local-alloc's combine_regs TIES the
  lui/ori constant pseudo to the AND's destination pseudo because BOTH are
  block-local, which is why the then arm emits lui/ori/and out of ONE register.
  In the h2a else arm the AND's destination was `var_v1`, a function-scope
  (global) pseudo, and combine_regs merges only block-local quantities — so the
  constant became its own 2-ref quantity and find_free_reg (no REG_ALLOC_ORDER
  in mips.h, hence a linear scan from $v0) handed it $v0, the SAME register the
  then arm used, so reorg.c's redundant_insn deleted the then-arm `lui $v0`
  after the delay slot was filled from the dead-branch thread. With `m`
  block-local the tie is restored AND `m`'s live range now genuinely SPANS the
  counter chain (born at the AND, dead at the `sw`, with all of `c` in between)
  — the covering range s6 P6b and s7 P7a/P7b failed to manufacture by
  lengthening or reordering chains. The denser counter quantity is allocated
  first and takes $v0; `m`'s covering range finds $v0 busy and takes $v1 =
  target's register. Both luis then survive, closing the 2-insn gap.

- [s8] The store-tail trilemma that the shared `complete_store` existed to solve
  dissolves: the two else arms store from $v1 and the two then arms from $v0, so
  jump2's find_cross_jump can rtx_equal-merge only the two else tails with each
  other, leaving exactly target's three `sw` sites (two inline then-arm stores +
  one shared else store). This is why the same form both fixes the register
  identity AND keeps the store shape s2's H1d discovered.

- [s8] METHODOLOGICAL LESSON for the pipeline. The lever that closed this
  function was named, verbatim, in the s7 frontier — but it was only reachable
  by DELETING the accumulated s3-s7 chassis (shared label + function-scope
  staging vars) rather than perturbing it. s4-s7 measured ~350 perturbations of
  that chassis (permuter 55k+ iters, 288+16 directed combos, 5 RA levers, 2
  chain-length levers) and every one was inert or worse, precisely because the
  chassis itself was the constraint. When a `rederive` modality is mandated,
  throwing away the incumbent form is the point, not a risk.

- [s8] Artifacts: tmp/grind/func_8006B92C/s8/{apply.py, orig_body.c,
  sandbox_s8.json, canonical_s8.json}. apply.py is reusable: it swaps
  func_8006B92C's body (plus its two preceding extern decls) in src/text1b.c for
  any candidate .c file, stripping a leading block comment and preserving LF.
