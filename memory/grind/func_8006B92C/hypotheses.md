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
