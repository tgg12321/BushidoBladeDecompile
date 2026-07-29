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
