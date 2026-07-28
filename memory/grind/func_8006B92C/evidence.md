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
