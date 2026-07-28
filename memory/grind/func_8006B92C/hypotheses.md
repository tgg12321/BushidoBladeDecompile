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
