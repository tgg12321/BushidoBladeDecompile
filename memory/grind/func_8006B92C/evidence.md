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
