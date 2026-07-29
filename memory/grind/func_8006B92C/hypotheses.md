# Hypothesis ledger - func_8006B92C

## Live frontier (opened s1)

### H1 - defeat cross-jump-store-tail-merge on `D_800A34F8 = ...` in `switch(ret)` arms
- Mechanism: jump2 find_cross_jump merges the identical `D_800A34F8 = X` stores at the end of each case's then/else arms into a single shared `sw` reached by `j` from both arms. Target inlines the store per arm so the then-arm has its own `sw+j`. Costs 2 insns (one sw per case) plus prevents dead-branch scheduling of the lui into the branch delay slot.
- Probe: (a) restructure each case to write via distinct exit forms per arm (e.g. `goto` from then-arm, inline fallthrough from else) per [[cross-jump-store-tail-merge]] rule; (b) if that fails or introduces new residue, try the `duplicated-statement-into-arms` sanction with FAKE annotation.
- Status: OPEN. Both levers documented; s1 did not attempt (recon-only).

### H2 - dead-branch-scheduled `lui $v1` in bne/bnez delay slot
- Mechanism: reorg.c fill_from_thread hoists the first insn of the fallthrough arm into the delay slot when it clobbers a reg dead-along-the-taken-path. Target hoists `lui $v1, 0xFFFF` (the else-arm's first insn) into the `bne v1,v0,.else` delay slot in case 1 and into the `bnez v0,.else` delay slot in case 2. Build emits `nop` instead. Likely a downstream effect of H1 (once the arms are distinct enough, the natural sched fires).
- Probe: verify by measuring the residual AFTER an H1 candidate is in place; if dead-branch fill still misses, look for arm-length symmetry that reorg.c's cost model may be rejecting (compare length of `then` vs `else` slot chain).
- Status: OPEN, downstream of H1.

### H3 - the pinned `register u32 var_v1 asm("v1")` is doing nothing legit
- Mechanism: pins are score-inert under `--disable all` (sandbox strips them) and forbidden by inline-asm-policy. Removing it is a NO-OP for sandbox score today.
- Probe: remove the pin, re-sandbox. Should stay at 15. If it moves, that's meaningful evidence.
- Status: OPEN, cheap first-probe for s2.

## Rejected
(none yet)

## [s1] Baseline pure-C distance is 15 with 143 target insns / 139 build insns (4-insn shortfall)
- mechanism: canonical verdict C, sandbox --disable all reports 15 with existing `register u32 var_v1 asm("v1")` pin stripped
- probe: wteng main canonical + sandbox --disable all
- result: distance=15, target_insns=143, build_insns=139
- verdict: CONFIRMED

## [s1] No sibling/duplicate analogs to transplant from
- mechanism: tools/find_duplicates.py signature match
- probe: find_duplicates.py --out-leads; grep 8006B92C
- result: no lead entries produced
- verdict: CONFIRMED

## [s1] The 4-insn shortfall is jump2 cross-jump merging the D_800A34F8 stores in switch(ret) case-1 and case-2 arms into a shared tail; target inlines per arm
- mechanism: objdump diff of tmp/sandbox/.../text1b.o vs asm/funcs/func_8006B92C.s: target has inline `sw v0,gp` + `j .after` in then-arm plus dead-branch-scheduled `lui $v1` in bne/bnez delay slot; build emits `nop` in delay and shares the sw via a merged j target
- probe: wsl objdump -d --disassemble=func_8006B92C; artifacts banked in tmp/grind/func_8006B92C/s1/
- result: cross-jump merge confirmed as the mechanism; explains all 4 missing insns (2 sw inlined per case + 2 dead-branch-sched lui per case)
- verdict: CONFIRMED

## [s2] H3: `register u32 var_v1 asm("v1")` pin is score-inert and forbidden by inline-asm-policy
- mechanism: cheat-invisible sandbox strips register-asm pins before scoring
- probe: remove pin, re-sandbox --disable all
- result: score unchanged at 15 (pin was inert)
- verdict: CONFIRMED

## [s2] H1d: routing case-1-then, case-2-then to inline stores and case-1-else, case-2-else to a shared `complete_store` label (both cases funnel through a shared `do_call` label) creates 3 target-shaped sw sites
- mechanism: then-arms compute `a0 & 0xFFFF1FFF` with result in $v0 (sw $v0,gp); shared complete_store computes final via OR to $v1 (sw $v1,gp). Store-source register differs (v0 vs v1) so jump2 find_cross_jump cannot rtx_equal-merge the then-arm stores with the shared-else store. Structural shape now matches target (3 stores: two then-inline + one shared-else).
- probe: H1d edit in src/text1b.c, sandbox --disable all
- result: score 15 -> 10, target_insns 143, build_insns 140 (3-insn deficit)
- verdict: CONFIRMED

## [s2] H1e: adding explicit `var_v1 = a0 & 0xFFFF1FFF;` in each else arm to force per-arm mask compute REGRESSES to 15
- mechanism: With var_v1 explicit, the shared complete_store's `var_v1 | ((var_v0 & 7) << 13)` allocates final OR result to $v0 (var_v1 in $v1 | shift in $v0 -> $v0). All 4 store sites now `sw $v0,gp` -> rtx_equal -> jump2 remerges to a single shared sw. The store-tail merge trilemma is coupled: forcing per-arm mask compute breaks the sw-source-register divergence that H1d exploits.
- probe: H1e edit adding var_v1 declaration + per-arm assignment + shared-block var_v1 use; sandbox
- result: score jumped from 10 to 15; disasm showed 4 stores all collapsed to one shared `sw $v0` at address of shared complete_store
- verdict: CONFIRMED

## [s2] H1c: flipping branch sense with per-case func_8005C650 calls (no shared do_call) regresses badly
- mechanism: duplicated jal sites cross-jump-merge with adverse delay-slot fill effects
- probe: H1c variant, sandbox
- result: score 26 (worse than baseline 15)
- verdict: KILLED

## [s2] H1f: fully inlining both else stores (no shared complete_store) also regresses to 15
- mechanism: identical per-arm final compute reintroduces the 2-arm store-tail merge within each case
- probe: sandbox
- result: score 15
- verdict: KILLED

## [s3] Split-init form `var_v1 = a0 & 0xFFFF1FFF; ... var_v1 |= ((var_v0 & 7) << 13); D_800A34F8 = var_v1;` breaks the H1e coupling: reassigning through the SAME lvalue keeps the OR result in $v1 (not $v0), preserving sw-source-reg divergence AND per-arm mask compute simultaneously.
- mechanism: H1e wrote `D_800A34F8 = var_v1 | ...` in one statement -- GCC treats the OR result as a fresh pseudo which allocno gets $v0 (both operands live). Rewriting as `var_v1 |= ...` reuses var_v1's home register ($v1) for the result. Store then uses $v1 -- matches target's shared complete_store sw source.
- probe: s3: h2a edit -- add `u32 var_v1;` decl + per-arm `var_v1 = a0 & 0xFFFF1FFF;` in else arms + rewrite complete_store as `var_v1 |= ((var_v0 & 7) << 13); D_800A34F8 = var_v1;`. sandbox --disable all.
- result: score 10 -> 6, build_insns 140 -> 141, per-arm mask compute preserved, shared complete_store's sw source is $v1 (matches target)
- verdict: CONFIRMED

## [s3] Flipping branch sense (`==` -> `!=`) to invert taken/fall-through so reorg dead-branch-fills the delay slot with else-arm's `lui $v1` regresses hard from h2a base (same family as s2 h1c).
- mechanism: Intended: `!= 0x4000` makes then-arm the taken path, else the fall-through; reorg should then fill delay slot with else's lui $v1 (matching target). Actual: GCC jump-threading collapses the != form to a different bne+j shape that breaks shared do_call and duplicates jal sites.
- probe: s3: h2b_branch_sense_flip_from_h2a -- from h2a base, flipped both `==` compares to `!=` with then/else swap. sandbox.
- result: score 26 (regressed from 6, +20 insns cascade). Consistent with s2 h1c KILLED.
- verdict: KILLED

## [s3] Introducing a fresh temp `u32 t1 = a0 & 0xFFFF1FFF; D_800A34F8 = t1;` in the then arm does not disrupt delay-slot merging.
- mechanism: Hoped: extra local would force GCC to allocate a distinct pseudo for then-arm's mask, preventing lui-share via delay-slot fill. Actual: temp DCE'd -- semantically identical to direct store, same RTL.
- probe: s3: h2c -- replaced then-arm `D_800A34F8 = a0 & 0xFFFF1FFF;` with `u32 t1 = ...; D_800A34F8 = t1;`. sandbox.
- result: score 6 unchanged (temp DCE'd as expected)
- verdict: KILLED

## [s4] H4a: subu algebraic mask (`var_v1 = a0 - (a0 & 0xE000)`) in else arms produces same value while reusing pre-compared $v1, flipping reorg fill priority to close residual.
- mechanism: a - (a & M) == a & ~M when subtracted bits are all within M (no borrow). Reusing $v1 saves a lui birth; if fill_from_thread then falls back to else-arm's other candidates, delay-slot fill may shift to target's shape.
- probe: s4: replaced both else arm mask compute with `a0 - (a0 & 0xE000)`; sandbox --disable all.
- result: score 6 -> 17 REGRESSED. Reuse of $v1 killed the fresh lui birth AND destroyed the h2a store-source $v1 alignment (sw-source-reg divergence lost).
- verdict: KILLED

## [s4] H4b: XOR algebraic mask (`var_v1 = a0 ^ (a0 & 0xE000)`) is algebraically identical to H4a and may emit differently through combine.c.
- mechanism: combine.c may or may not fold XOR-with-and back to and-with-not; if it emits xor primitive, RTL shape differs from subu and may steer reorg differently.
- probe: s4: replaced both else arm mask compute with `a0 ^ (a0 & 0xE000)`; sandbox --disable all.
- result: score 6 -> 17 REGRESSED. combine folds identically to subu form; same failure mode.
- verdict: KILLED

## [s4] H4c: duplicated-statement-into-arms per rule: inline `func_8005C650(0,0x7F,0x7F); break;` directly in each case removing the shared `do_call:` label to change register-allocation cascade upstream.
- mechanism: Duplicated jals may re-merge via cross-jump but the intermediate RA pass sees two independent successor blocks and may re-color var_v0/var_v1 pseudos differently, potentially preserving the h2a store-source $v1 alignment while adding surface for reorg to flip fill choice.
- probe: s4: replaced `goto do_call; ... do_call: func_8005C650(0,...); break;` with per-case inline `func_8005C650(0,...); break;`. Sandbox --disable all.
- result: score 6 -> 14 REGRESSED. Jump2 does re-merge the jal sites but the RA cascade shifts adversely; h2a store-source $v1 alignment lost.
- verdict: KILLED

## [s4] H4d/H4e: shift-form then-arm mask (`(a0 & 0xFFFF0000) | ((a0 << 19) >> 19)`) emits `andi/sll/srl/or` instead of `lui/ori/and`, removing the then-arm's lui candidate and forcing reorg dead-branch fill from else-arm's `lui $v1`.
- mechanism: If then-arm's first insn is not a lui, fall-through fill can't pick a lui; the only lui candidate is else-arm's `lui $v1` (dead-branch fill), which matches target.
- probe: s4: applied to case-1 then only (H4d) and both cases (H4e). Sandbox --disable all.
- result: H4d 6 -> 7; H4e 6 -> 8. combine.c either folds the shift form back to lui/ori/and OR emits enough extra insns that fill priority doesn't flip AND per-arm mask insn count grows.
- verdict: KILLED

## [s4] H4perm: directed PERM_GENERAL permuter campaign with fresh seeds finds a non-cheat closing form.
- mechanism: The permuter's random-mode search over PERM_GENERAL-decorated mask/shift compute in both else arms may reach a spelling that closes the residual without the split-load-anchor idiom.
- probe: s4: launched s4-PERMGEN-fresh (6 jobs, --stack-diffs, base_score 235, PERM_GENERAL macros around per-arm mask and shift compute). Ran ~6 min / 14067 iters across 4 wait windows.
- result: 5 novel finds (output-115-3, 205-1, 175-2, 155-5, 220-3) — every one a variant of the forbidden split-load-anchor pattern (`var_v1 = 0xFFFF1FFF; var_v1 = a0 & var_v1;`, `var_v1 = a0 & (v = 0xFFFF1FFF);`, `new_var3 = 13; var_v0 = new_var3; var_v0 = ((a0 >> var_v0) & 7) - 1;`). Rejected per Judge s3-BINDING (no-new-park-categories §Auto-search tools output is PROPOSALS). Pre-s4 campaign at 29388 iters converged to identical family. Cheat basin is the closing-form center of mass.
- verdict: KILLED

## [s5] H5a: the s4 cheat basin is an ARTIFACT OF THE MUTATION SET, not of the search space — disabling the permuter passes whose output shape is itself a catalog cheat will steer the search into a legitimate closing form.
- mechanism: Every s4 find (`v = C; v = a & v;`, `new_var3 = 13; var_v0 = new_var3; ...`) is produced by a specific decomp-permuter randomization pass — `perm_temp_for_expr`, `perm_split_assignment`, `perm_chain_assignment`, `perm_duplicate_assignment`. Those passes are the ONLY ones that manufacture an extra assignment to a value (the reg_n_sets bump / extra lui-ori birth cluster the residual wants). `settings.toml [weight_overrides]` accepts a 0.0 weight per pass name (`Randomizer.__init__` looks each `method.__name__` up in the merged weight map), so the cheat-shape producers can be structurally removed from the search while leaving the legitimate structural family (reorder_stmts / reorder_decls / condition / commutative / add_sub / inequalities / compound_assignment / cast_simple / randomize_internal_type / factor_shift / factor_mult / expand_expr / remove_ast / sameline).
- probe: s5 chassis A — fresh workspace `permuter/f8006B92C_s5/` (clean h2a base.c, no PERM macros → whole-function random region) with 19 passes weighted 0.0 (all of perm_temp_for_expr, perm_split_assignment, perm_chain_assignment, perm_long_chain_assignment, perm_duplicate_assignment, perm_add_self_assignment, perm_pad_var_decl, perm_dummy_comma_expr, perm_add_mask, perm_xor_zero, perm_mult_zero, perm_refer_to_var, perm_empty_stmt, perm_ins_block, perm_inline, perm_randomize_external_type, perm_randomize_function_type, perm_float_literal, perm_struct_ref). 6 jobs, --stack-diffs, label s5-NOCHEATPASS.
- result: 41424 iterations / 1293 s, ZERO output-* dirs — not one mutation in the legitimate family ever scored below the base. (Contrast s4: the cheat-enabled search produced 5 novel finds in ~6 min.) The h2a form is a STRICT local minimum under legitimate mutation.
- verdict: KILLED

## [s5] H5b: some legitimate ALTERNATIVE SPELLING of the mask / counter / OR expressions closes the residual — the s4 random search may simply not have sampled the right combination.
- mechanism: A directed (manual-mode) permuter enumerates ALL combinations of hand-supplied alternatives instead of sampling, so a 5-site cross-product is exhaustively decided rather than probabilistically explored. Each alternative was chosen to be an ordinary C respelling with a genuinely different RTL tree (compound assignment on the global, algebraic simplification licensed by the arm's own branch condition, dropped-redundant-mask on the counter, mask-after-shift vs shift-after-mask OR, statement order in the else arms).
- probe: s5 chassis B — `permuter/f8006B92C_s5b/` with PERM_GENERAL alternative sets at 5 sites plus PERM_LINESWAP over each else arm's two statements: case-1 then `{a0 & 0xFFFF1FFF | &= 0xFFFF1FFF | a0 - 0x4000}` (the subtraction is exact on that arm since `a0 & 0xE000 == 0x4000`), case-2 then `{(a0 & 0xFFFF1FFF) | 0x4000 | a0 | 0x4000 | D_800A34F8 |= 0x4000}` (exact since `a0 & 0xE000 == 0`), counter `{((a0>>13)&7)±1 | (a0>>13)±1}` (equal because complete_store re-masks with `& 7`), OR form `{((var_v0 & 7) << 13) | (var_v0 << 13) & 0xE000}`. 288 combinations.
- result: all 288 enumerated, 0 compile errors, minimum score 235 == base (3 combinations tie, the rest are 335..2444). No legit respelling of the arithmetic improves on h2a.
- verdict: KILLED

## [s5] H5c: a whole-case STRUCTURAL alternative — duplicating the `D_800A34F8` read into the arms, or hoisting the shared mask above the if — changes the lui-birth/CSE picture enough to flip reorg.c's delay-slot fill choice.
- mechanism: The residual's core is that target computes `lui 0xFFFF` TWICE per case (once per arm) while our build computes it once and shares it via a fall-through delay-slot fill. Two sanctioned levers attack that directly: [[split-read-defeats-hoist]] (duplicate the global read into each arm so the two mask operands are distinct pseudos and CSE cannot fold the two births) and [[hoist-shared-arm-computation-defeats-copy-pref]] (hoist the genuinely-shared mask above the if so the then-arm has no lui at all and the fall-through fill candidate disappears).
- probe: s5 chassis C — `permuter/f8006B92C_s5c/` with PERM_GENERAL over the ENTIRE case-1 and case-2 bodies, 4 whole-body variants each (shared `a0` read = h2a; per-arm duplicated global read; hoisted shared `var_v1` mask; mixed — shared `a0` for the compare/counter but a fresh global read for the else-arm mask). 16 combinations.
- result: all 16 enumerated, minimum 235 == base. The duplicated-read variants tie at 235 (GCC CSEs the repeated non-volatile loads — nothing between the reads invalidates them, since each arm's store to D_800A34F8 comes after both reads); the hoisted-mask variants score 285 (worse — the pre-branch birth removes the per-arm mask compute that h2a's floor-6 depends on and the delay slot is then filled by the compare's `andi`). Banked as rejected/s5_perarm_duplicated_global_read.c and rejected/s5_hoisted_shared_mask.c.
- verdict: KILLED

## [s5] The s4 permuter cheat basin (split-load-anchor family) is an artifact of WHICH mutation passes are enabled, not of the search space — zeroing the passes whose output shape is itself a catalog cheat will steer the search into a legitimate closing form.
- mechanism: Every s4 find (`v = C; v = a & v;`, `new_var3 = 13; var_v0 = new_var3; ...`) is manufactured by a specific decomp-permuter randomization pass: perm_temp_for_expr, perm_split_assignment, perm_chain_assignment, perm_duplicate_assignment. Those are the only passes that add an extra assignment to a value (the reg_n_sets bump / extra lui-ori birth cluster the residual wants). settings.toml [weight_overrides] is keyed by each pass function's __name__ (main.py:337 merges over get_default_randomization_weights), so the cheat-shape producers can be removed from the search while the legitimate structural family (reorder_stmts/reorder_decls/condition/commutative/add_sub/inequalities/compound_assignment/cast_simple/randomize_internal_type/factor_shift/factor_mult/expand_expr/remove_ast/sameline) keeps running.
- probe: Chassis A: fresh workspace permuter/f8006B92C_s5/ (clean h2a base.c, no PERM macros so the whole function is the randomization region) with 19 passes weighted 0.0; 6 jobs, --stack-diffs, label s5-NOCHEATPASS; waited in-turn across two 9-minute windows.
- result: 41424 iterations / 1293 s, ZERO output-* dirs — not one legitimate-family mutation ever scored below base 235. (s4's cheat-enabled search produced 5 novel finds in ~6 min.)
- verdict: KILLED

## [s5] Some legitimate alternative SPELLING of the mask / counter / OR expressions closes the residual; s4's random search simply never sampled the right combination.
- mechanism: Manual-mode (directed) permuter enumerates ALL combinations of hand-supplied alternatives rather than sampling, so a 5-site cross-product is exhaustively decided. Each alternative was chosen for a genuinely different RTL tree: compound assignment straight on the global, algebraic simplification licensed by the arm's own branch condition, dropped-redundant-mask on the counter (legal because complete_store re-masks with & 7), mask-after-shift vs shift-after-mask in the OR, and statement order within each else arm.
- probe: Chassis B: permuter/f8006B92C_s5b/ with PERM_GENERAL alternative sets at 5 sites + PERM_LINESWAP over each else arm — case-1 then {a0 & 0xFFFF1FFF | D_800A34F8 &= 0xFFFF1FFF | a0 - 0x4000}, case-2 then {(a0 & 0xFFFF1FFF) | 0x4000 | a0 | 0x4000 | D_800A34F8 |= 0x4000}, counters {((a0>>13)&7)±1 | (a0>>13)±1}, OR {((var_v0 & 7) << 13) | (var_v0 << 13) & 0xE000}. 288 combinations, label s5b-DIRECTED-ALTS.
- result: 288/288 enumerated, 0 compile errors, minimum score 235 == base (3 ties, rest 335..2444). No legit respelling of the arithmetic beats h2a. Independently re-confirms s4's H4a/H4b algebraic kills by a different route.
- verdict: KILLED

## [s5] A whole-case STRUCTURAL alternative — duplicating the D_800A34F8 read into the arms, or hoisting the genuinely-shared mask above the if — changes the lui-birth/CSE picture enough to flip reorg.c's delay-slot fill choice from fall-through to dead-branch.
- mechanism: The residual's core is that target computes `lui 0xFFFF` twice per case (once per arm) while our build computes it once and shares it via a fall-through delay-slot fill. split-read-defeats-hoist says duplicate the read into each arm so the two mask operands are distinct pseudos and CSE cannot fold the births; hoist-shared-arm-computation-defeats-copy-pref says hoist the shared mask above the if so the then-arm has no lui at all and the fall-through fill candidate disappears.
- probe: Chassis C: permuter/f8006B92C_s5c/ with PERM_GENERAL over the ENTIRE case-1 and case-2 bodies, 4 whole-body variants each (shared a0 read = h2a; per-arm duplicated global read; hoisted shared var_v1 mask; mixed shared-a0-for-compare + fresh global read for the else mask). 16 combinations, label s5c-STRUCT-ALTS.
- result: 16/16 enumerated, minimum 235 == base. Duplicated-read variants tie at 235 (GCC CSEs the repeated non-volatile loads — each arm's store to D_800A34F8 comes after both reads, so nothing invalidates them). Hoisted-mask variants score 285, worse (the pre-branch birth removes the per-arm mask compute h2a's floor-6 depends on; the delay slot is then filled by the compare's andi). Both banked in rejected/.
- verdict: KILLED

## [s6] The 2-insn shortfall is reorg.c's delay-slot FILL PRIORITY choosing the fall-through thread over the dead-branch thread (the s3/s5 frontier hypothesis).
- mechanism: reorg.c fill_slots_from_thread weighs thread cost + register liveness; s3 concluded the fall-through `lui $v0` wins over the else-arm `lui $v1`.
- probe: s6 forensics — cc1 -da on the real build's text1b.i (h2a body in src), extract func_8006B92C from the .dbr dump, read the SEQUENCE wrapping the case-1 `bne`.
- result: FALSIFIED. The slot is filled with insn 314 = the ELSE arm's lui (dead-branch / taken-thread fill), exactly as target does. The shortfall comes from the NEXT step: reorg.c's redundant_insn deletes the then-arm's identical `lui $v0` (insn 312 is absent from the dbr dump) because the delay-slot insn already sets the same hard register to the same constant. Target is immune only because its else-arm constant is in $v1.
- verdict: KILLED (hypothesis disproven; replaced by the local-alloc account below)

## [s6] The deciding input is local-alloc.c's hard-register choice for the else-arm mask constant: it takes $v0 (same as the then arm) where target takes $v1.
- mechanism: the constant is a basic-block-local pseudo (92 in case 1) allocated by local-alloc.c, which runs BEFORE global.c. Its AND destination is var_v1 = global pseudo 78 ($v1); local-alloc's combine_regs can only tie qtys of block-local regs, so the constant cannot inherit var_v1's register, and find_free_reg takes the first REG_ALLOC_ORDER entry whose regs_live_at scan is clear over the constant's 3-insn range — $v0.
- probe: read `;; Register dispositions` + conflict lists in the extracted .greg and the pseudo-level RTL of the else arm in the extracted .lreg.
- result: CONFIRMED. 92 in 2 ($v0), 78 in 3 ($v1), counter temps 93/94 also in $v0 (disjoint ranges, no conflict). Same picture in case 2.
- verdict: CONFIRMED

## [s6] Making the shift value live ACROSS the mask compute inside the else arm blocks $v0 and forces the constant to $v1.
- mechanism: find_free_reg ORs regs_live_at[ins] over the qty's whole range; a block-local pseudo born before the constant and dying after it marks $v0 busy, pushing the constant to the next REG_ALLOC_ORDER entry ($v1 = var_v1's register = target's shape).
- probe: s6 P6b — separate per-arm temps t1/t2 holding `(a0 >> 13) & 7`, written before the mask statement in both else arms; sandbox + cc1 -da dumps (dumps_p6b).
- result: KILLED. score 6 / 141 insns, unchanged. .lreg shows sched1 hoists the whole mask chain (uids 316/317/76) ABOVE the shift chain (uids 71/72/79) — the shift was emitted first and moved down — so t1's range [72..79] never overlaps the constant's [316..76]. Source statement order inside the arm is structurally inert (independently explains s5 chassis B's PERM_LINESWAP tie).
- verdict: KILLED

## [s6] Hoisting the counter above the `if` gives the GLOBAL var_v0 a live range across the branch that blocks $v0 in the else arm.
- mechanism: a longer live range for var_v0 (which carries a $v0 preference in the .greg) would cross the constant's range.
- probe: s6 P6c — `var_v0 = ((a0 >> 13) & 7) +/- 1;` computed unconditionally before each case's `if`, else arms reduced to mask + goto; sandbox.
- result: KILLED. score 19, build_insns 139. global.c runs AFTER local-alloc so a global pseudo can never reserve a register against a local one; the pre-branch `li v0,0x4000` also pushes var_v0 to $a1. With the counter gone from the arms the shared `lui $v0` returns to the delay slot and both then-arm luis die.
- verdict: KILLED

## [s6] Breaking the delay-slot insn's identity with the then-arm constant closes the INSTRUCTION COUNT (evidence that the count gap is redundant_insn, not fill priority).
- mechanism: if the else block's leading insn is not "same constant, same hard register" as the then arm's, redundant_insn cannot delete the then-arm lui.
- probe: s6 P6a — one function-scope temp `t` shared by both else arms (global pseudo -> $a1) holding the shift value, written before the mask.
- result: CONFIRMED on count, rejected on registers: build_insns 141 -> 143 == target for the first time, but score 6 -> 12 (delay slot carries `srl $v0` where target carries `lui $v1`; counter chain splits across $a1 where target keeps it in $v0).
- verdict: CONFIRMED (count mechanism); form banked in rejected/s6_shared_global_temp_t.c

## [s6] The 2-insn shortfall is reorg.c's delay-slot FILL PRIORITY choosing the fall-through thread over the dead-branch thread (the s3/s5 frontier hypothesis).
- mechanism: reorg.c fill_slots_from_thread weighs thread cost plus register liveness; s3 inferred from disassembly that the fall-through `lui $v0` beats the else-arm `lui $v1` as fill candidate.
- probe: cc1 -da over the real build's preprocessed src/text1b.c with the h2a body in src; extracted func_8006B92C from the .dbr dump (tmp/grind/func_8006B92C/s6/dumps_h2a/func_8006B92C.dbr) and read the SEQUENCE wrapping the case-1 bne.
- result: FALSIFIED. The delay slot holds insn 314 = the ELSE arm's `lui $v0,0xFFFF` — reorg already fills from the branch-taken (dead-branch) thread exactly like target. The then-arm's own lui (insn 312) is absent from the dbr dump: reorg.c's redundant_insn deleted it because the delay-slot insn sets the SAME hard register to the SAME constant the fall-through path needed. Target keeps both luis only because its else-arm constant lives in $v1.
- verdict: KILLED

## [s6] The deciding input is local-alloc.c's hard-register choice for the else-arm mask constant: it takes $v0 (identical to the then arm) where target takes $v1.
- mechanism: The constant is a basic-block-local pseudo (92 in case 1) allocated by local-alloc.c, which runs BEFORE global.c. The AND's destination is var_v1 = global pseudo 78 ($v1); local-alloc's combine_regs only merges qtys of block-local regs, so the constant can never inherit var_v1's register, and find_free_reg walks REG_ALLOC_ORDER taking the first entry whose regs_live_at scan is clear over the constant's 3-insn range — $v0.
- probe: Read `;; Register dispositions` + conflict lists in the extracted .greg and the pseudo-level else-arm RTL in the extracted .lreg (dumps_h2a).
- result: CONFIRMED: `92 in 2` ($v0), `78 in 3` ($v1), counter temps 93/94 also in $v0 with disjoint ranges (no conflict). Same picture in case 2. This is the single fact that separates our build from target.
- verdict: CONFIRMED

## [s6] Making the shift value live ACROSS the mask compute inside each else arm blocks $v0 and forces the constant to $v1 (target's register).
- mechanism: find_free_reg ORs regs_live_at[ins] over the qty's whole range; a block-local pseudo born before the constant and dying after it marks $v0 busy over that range, pushing the constant to the next REG_ALLOC_ORDER entry.
- probe: P6b — separate per-arm block-local temps t1/t2 holding `(a0 >> 13) & 7`, written before the mask statement in both else arms. sandbox --disable all + a second cc1 -da run (dumps_p6b).
- result: KILLED. score 6 / 141 insns, identical to base. The .lreg dump shows sched1 hoists the whole mask chain (uids 316/317/76) ABOVE the shift chain (uids 71/72/79) — expand emitted the shift first and the scheduler moved the mask up — so t1's range [72..79] never overlaps the constant's [316..76]. Source statement order inside the arm is structurally inert, which independently explains why s5 chassis B's PERM_LINESWAP tied at base.
- verdict: KILLED

## [s6] Hoisting the counter above the `if` gives the GLOBAL var_v0 a live range across the branch that blocks $v0 in the else arm.
- mechanism: var_v0 carries a $v0 preference in the .greg; a live range spanning the branch would cross the constant's range and deny it $v0.
- probe: P6c — `var_v0 = ((a0 >> 13) & 7) +/- 1;` computed unconditionally before each case's if, else arms reduced to mask + goto. sandbox --disable all.
- result: KILLED. score 6 -> 19, build_insns 139. global.c runs AFTER local-alloc, so a global pseudo can never reserve a register against a local one; the pre-branch `li v0,0x4000` pushes var_v0 to $a1 anyway. With the counter gone from the arms the shared `lui $v0` returns to the delay slot and BOTH then-arm luis die (4 short).
- verdict: KILLED

## [s6] Breaking the delay-slot insn's identity with the then-arm constant closes the INSTRUCTION COUNT (direct test that the count gap is redundant_insn, not fill priority).
- mechanism: If the else block's leading insn is not 'same constant, same hard register' as the then arm's, redundant_insn has nothing to delete and the then-arm lui survives.
- probe: P6a — ONE function-scope temp `t` shared by both else arms (therefore a GLOBAL pseudo) holding the shift value, written before the mask. sandbox --disable all + objdump of the sandbox object.
- result: CONFIRMED on count, rejected on registers. build_insns 141 -> 143 == target_insns for the first time in this function's history (t lands in $a1, the else block now opens with the `srl`, reorg fills the slot with `srl $v0,$a0,0xd`, both then-arm luis survive). Honest distance regressed 6 -> 12 because the counter chain splits across $a1 where target keeps it entirely in $v0 and the delay slot carries the srl where target carries `lui $v1`.
- verdict: CONFIRMED

## [s7] BB2_QTY_DEBUG measurement of local-alloc's per-block quantity ordering: the else-arm mask constant is a 2-ref block-local quantity whose live range is ALWAYS disjoint from the counter chain's, so find_free_reg's linear scan always returns $v0.
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc.c find_free_reg (line 2182) walks hard regs 0,1,2,... and returns the first not in "used", where used = OR of regs_live_at[ins] over [qty_birth, qty_death). $v0 (regno 2) is the first allocatable GPR ($zero and $at are fixed). Only BLOCK-LOCAL quantities already allocated in this block, and genuine HARD-register live ranges, can set a bit in regs_live_at; global pseudos (var_v0, var_v1, a0) are invisible here because global.c runs AFTER local-alloc. The else arms contain no call and no hard-register reference, so the only possible blocker is another block-local qty whose range COVERS the constant's.
- probe: instrumented cc1 (tmp/gccdbg/cc1, BB2_QTY_DEBUG=1) over the real preprocessed src/text1b.c, sliced to func_8006B92C by running cc1 WITHOUT -quiet so it prints function names to stderr (tmp/grind/func_8006B92C/s7/qty2.sh; log tmp/grind/func_8006B92C/s7/qty_h2a/qty_func.log).
- result: CONFIRMED. blk=5 (case-1 else arm): "qty=1 reg1=94 birth=10 death=14 refs=4 got=2" (the srl+andi counter chain, tied into ONE qty) and "qty=0 reg1=92 birth=4 death=8 refs=2 got=2" (the lui/ori mask constant). Disjoint ranges [4,8) and [10,14) -> both get $v0. blk=4 (case-1 THEN arm) shows the contrast: "qty=0 reg1=90 birth=2 death=8 refs=4 got=2" -- there is NO separate constant qty because local-alloc's combine_regs TIED the constant pseudo 91 to the and-destination pseudo 90 (both block-local), which is exactly why the then arm emits lui $v0 / ori $v0 / and $v0,$a0,$v0 out of one register. In the else arm that tie is impossible: the and's destination is var_v1, a GLOBAL pseudo, and combine_regs only merges quantities of block-local regs.
- verdict: CONFIRMED

## [s7] The s6 frontier lever -- lengthen the counter's dependence chain inside the arm so it is scheduled ahead of the mask and stays live in $v0 across the constant -- does not work: the two chains are always emitted CONTIGUOUSLY, so their live ranges are disjoint whichever order they are in.
- mechanism: both chains are pure 1-cycle ALU dependence chains; the r3000 scheduler has no latency reason to interleave them, so schedule_block emits one complete chain then the other. Interleaving is the ONLY shape that would give a block-local value a range spanning the constant's birth and death.
- probe: P7a -- keep the mask statement first but move the redundant "& 7" out of the shared complete_store into both else arms (counter chain 3 -> 4 insns, qty refs 4 -> 6); sandbox --disable all + BB2_QTY_DEBUG (qty_p7a/). P7b -- same but ALSO put the counter statement first in both arms (qty_p7b/).
- result: KILLED, both. P7a: score 6 / 141 insns, identical to the h2a base; blk=5 counter qty reg1=95 birth=10 death=16 refs=6 got=2, constant reg1=92 birth=4 death=8 refs=2 got=2 -- the higher-priority counter qty is allocated FIRST but over a range that starts after the constant has died, so $v0 is free for the constant anyway. P7b: the emission order DID flip (counter qty reg1=94 birth=4 death=10 refs=6 got=2 allocated first, constant reg1=95 birth=12 death=16 refs=2 got=2 second) -- and the constant STILL got $v0, because the ranges are again disjoint, merely swapped. Score regressed 6 -> 12 / 140 insns (counter-first order also loses the h2a store-source-register divergence). Banked as rejected/s7_arm_masked_counter.c and rejected/s7_counter_first_masked.c.
- verdict: KILLED

## [s7] Under the measured local-alloc rule, target's "lui $v1 / ori $v1 / and $v1,$a0,$v1" in the else arm is only reachable if the mask constant and var_v1 share a register, which requires them to be the same QUANTITY -- and the only C spellings that produce that tie for a global var_v1 are the Judge-banned split-load-anchor family.
- mechanism: for a distinct block-local constant quantity, find_free_reg deterministically returns $v0 (see the two entries above): global pseudos cannot reserve a register against it, there is no hard-register live range in the arm, and no block-local quantity can cover its range because the scheduler emits dependence chains contiguously. The only way the constant lands in var_v1's register is combine_regs tying them, as it does in the THEN arm where the and's destination is block-local. Tying to a GLOBAL var_v1 requires the constant to be written THROUGH var_v1 -- i.e. "var_v1 = 0xFFFF1FFF; var_v1 = a0 & var_v1;" -- which the s3-BINDING Judge constraint forbids by any spelling, and which is precisely the family the s4 permuter campaign converged on and the s5 cheat-suppressed campaign proved is the only improving neighborhood (41424 iterations, zero legitimate-family finds).
- probe: derivation from the s7 QTYDBG measurements above plus the s4/s5 campaign records; no new build.
- result: CONFIRMED as a coherent account of every measurement s1-s7. NOT a proof of impossibility: one class is still untested -- making the else arm's and-DESTINATION block-local so combine_regs ties the constant to it exactly as in blk=4, while still keeping three distinct store sites. See the s7 frontier.
- verdict: CONFIRMED

## [s7] The else-arm mask constant's hard register is decided by local-alloc.c find_free_reg's linear hard-register scan over the OR of regs_live_at across the quantity's live range, and nothing in the arm can make $v0 busy over that range.
- mechanism: mips.h defines no REG_ALLOC_ORDER, so find_free_reg (local-alloc.c:2182) walks hard regs 0,1,2,... and returns the first not in 'used' = OR of regs_live_at[ins] over [qty_birth, qty_death). $v0 (regno 2) is the first allocatable GPR. regs_live_at holds HARD registers only, so only an already-allocated BLOCK-LOCAL quantity or a genuine hard-register live range can block it; var_v0, var_v1 and a0 are all GLOBAL pseudos assigned later in global.c and are invisible to local-alloc. The else arms contain no call and no hard-register reference.
- probe: tmp/gccdbg/cc1 (the instrumented compiler; the production tools/gcc-2.7.2/build/cc1 is a May-18 binary predating the BB2_* probes and prints nothing) run with BB2_QTY_DEBUG=1 and WITHOUT -quiet over the real preprocessed src/text1b.c, sliced to func_8006B92C via the function-name echo on stderr (tmp/grind/func_8006B92C/s7/qty2.sh).
- result: blk=5 (case-1 else arm): mask constant qty reg1=92 birth=4 death=8 refs=2 got=2 ($v0); counter chain qty reg1=94 birth=10 death=14 refs=4 got=2 ($v0). Ranges disjoint. blk=4 (case-1 THEN arm) has NO separate constant qty -- qty0 reg1=90 birth=2 death=8 refs=4 got=2 -- because combine_regs tied the constant pseudo to the and-destination pseudo (both block-local), which is why the then arm emits lui/ori/and all in $v0. blk=3 (the compare block) already matches target ($v1 andi temp, $v0 for 0x4000) by the same qty_compare_1 priority arithmetic.
- verdict: CONFIRMED

## [s7] s6 frontier lever: lengthening the counter's dependence path inside the else arm (or shortening the mask's) makes the counter schedule ahead of the mask, leaving a block-local value live in $v0 across the constant and pushing the constant to $v1 (target's register).
- mechanism: Both chains are pure 1-cycle ALU dependence chains, so schedule_block has no latency reason to interleave them and emits one complete chain then the other. Interleaving is the only shape that would give a block-local quantity a range spanning the constant's birth and death.
- probe: P7a: move the redundant '& 7' out of the shared complete_store into both else arms (counter chain 3 -> 4 insns, qty refs 4 -> 6), mask statement still first. P7b: same, plus the counter statement moved first in both arms. Both measured with sandbox --disable all and with BB2_QTY_DEBUG (tmp/grind/func_8006B92C/s7/qty_p7a/, qty_p7b/).
- result: P7a: score 6 / 141 insns -- bit-identical to the h2a base; blk=5 counter qty reg1=95 birth=10 death=16 refs=6 got=2, constant reg1=92 birth=4 death=8 refs=2 got=2 (higher-priority counter allocated first but over a range starting after the constant dies). P7b: emission order DID flip (counter qty reg1=94 birth=4 death=10 refs=6 got=2 allocated first; constant reg1=95 birth=12 death=16 refs=2 got=2) and the constant STILL got $v0 -- disjoint ranges, merely swapped; score regressed to 12 / 140. Banked as rejected/s7_arm_masked_counter.c and rejected/s7_counter_first_masked.c.
- verdict: KILLED

## [s7] Target's else-arm shape (lui $v1 / ori $v1 / and $v1,$a0,$v1) requires the mask constant and var_v1 to be the SAME local-alloc quantity, and for a GLOBAL var_v1 the only C spellings that produce that tie are the Judge-banned split-load-anchor family.
- mechanism: combine_regs ties a constant pseudo to an and-destination only when both are block-local (measured in blk=4, the then arm). Our else arm's and-destination is var_v1, a global pseudo, so no tie is possible and the constant must be its own quantity -- which the first hypothesis shows is deterministically $v0. Writing the constant THROUGH var_v1 ('var_v1 = 0xFFFF1FFF; var_v1 = a0 & var_v1;') would create the tie, and that is exactly the split-load-anchor pattern the s3-BINDING Judge constraint forbids by any spelling and the one the s4 permuter converged on / s5's cheat-suppressed 41424-iteration campaign showed is the only improving neighborhood.
- probe: Derivation from the s7 QTYDBG measurements plus the banked s4/s5 campaign records; no new build.
- result: Coherent account of every measurement s1-s7, and it explains why every algebraic, statement-order and whole-case respelling measured in s4-s7 is inert. It is NOT an impossibility proof: one class is still untested (make the else arm's and-DESTINATION block-local so combine_regs ties the constant to it exactly as in blk=4, while still producing three distinct store sites).
- verdict: CONFIRMED
