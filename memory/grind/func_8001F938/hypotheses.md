# Hypothesis ledger — func_8001F938

## Live frontier (after s1 recon)

### H1 — kind-split lever (CONFIRMED, floor 11->6 alone)
`u32 kind_full = *(u16*)(a0+0x6A); u32 kind = kind_full & 0xFFFF;` provokes
target's redundant `lhu $a1,0x6A; andi $v1,$a1,0xFFFF`, fully matching the KIND
comparison region ($5/$3 split + the andi). Mechanism: two pseudos (raw + masked)
map to $a1/$v1 like target. Measured alone: floor 6. Combined with H3: floor 5.
BORDERLINE: redundant-mask / split-init family; needs a fresh cheat-reviewer PASS
(2026-06-15 reviewer called it "codegen-steering but secondary, dual-use with
sanctioned split-init-accumulation"). NEXT PROBE: run cheat-reviewer on the
kind-split in isolation; if PASS, it banks as clean.

### H2 — the +0x270 dual-load CSE wall (CONFIRMED as the residual-5 root cause)
Target emits TWO unconditional same-address loads `lh $v0,0x270; lhu $v1,0x270`.
GCC 2.7.2 CSE ALWAYS merges two same-address HImode reads into ONE load and
materializes the other signedness in-register (sign->zero = `andi`; zero->sign =
`sll;sra`). Measured this session: s16-first -> lh+move+andi; u16-first -> lhu+
sll;sra (floor 10, worse). No pure-C spelling produced two loads. The 5 residual
insns after H1+H3 ALL trace to this (nop+move for the missing 2nd load, andi-vs-
sll delay slot, coupled $v0-vs-$v1 register). NEXT PROBE: is there a semantically-
purposeful C construct that keeps two HImode loads (e.g. an intervening
memory-invalidating operation with real purpose, or a genuine union/overlapping-
field access in the entity struct)? If none exists, H2 is the reviewer-tension /
ruling question: the target provably contains the construct the reviewer's Test-1
("no semantic purpose") rejects. The only measured floor-2 form (unconditional
split-into-two-vars preload) was reviewer-FAILED and did NOT even produce two
loads — it matched more register/branch shape via the preload steering.

### H3 — branch-sense flip (CONFIRMED clean, ~1 distance)
Write the ternary as `if (probe >= 4) raw_or_3 = 3; else raw_or_3 = (u16)*p;`
(plain if/else). Flips GCC `slti;beqz` -> target `slti;bnez`. UNAMBIGUOUSLY clean.
Applied in candidate.c. Fully consumed (no further gain expected alone).

## Killed / disproven this session
- u16-first read order (rejected/u16-first-read-order.c) — floor 10, worse; single lhu.
- (inherited) unconditional dual-typed split (rejected/dual-type-probe-load.c) —
  floor 2 but reviewer-FAILED; codegen-steering, no semantic purpose.

## [s1] kind-split `u32 kind_full=lhu(0x6A); u32 kind=kind_full&0xFFFF` matches target's redundant `lhu $a1; andi $v1,$a1,0xFFFF` KIND region.
- mechanism: Two pseudos (raw + masked) allocate to $a1/$v1 like target; provokes the redundant andi target emits after the already-zero-extended lhu.
- probe: Apply kind-split alone, sandbox --disable all.
- result: floor 11 -> 6 (build_insns 108). KIND region fully matched.
- verdict: CONFIRMED

## [s1] branch-sense flip `if(probe>=4)raw_or_3=3; else raw_or_3=(u16)*p;` flips GCC slti;beqz to target's slti;bnez.
- mechanism: Ordering the >=4 arm first makes GCC branch-on-taken for the <4 case, matching target's bnez.
- probe: kind-split + branch-flip, sandbox.
- result: floor 6 -> 5. Unambiguously clean C.
- verdict: CONFIRMED

## [s1] GCC 2.7.2 emits target's TWO unconditional same-address loads (lh $v0,0x270 + lhu $v1,0x270) from some pure-C read-order/typing.
- mechanism: CSE cost asymmetry (zero->sign costs 2 insns) might trigger a reload for the 2nd signedness.
- probe: u16-first unconditional read then s16 read; disassemble.
- result: floor 10 (worse). Single lhu + sll;sra sign-extend; NEVER two loads. Both read orders collapse to one load. The 2nd lhu is unreachable in pure C.
- verdict: KILLED

## s2 structural update (2026-07-23)

### H2 — the +0x270 dual-load CSE wall — frontier item 2 EXHAUSTED / KILLED
Structural probes measured this session:
- union { s16 s; u16 u; } same-offset -> floor 5, ONE load (CSE merges). KILLED.
- clean single-read (reuse probe, no re-read) -> floor 8 (honest clean floor).
No semantically-purposeful pure-C construct produces the two adjacent same-address loads:
the target's two loads have NO intervening op (no legit memory invalidation), +0x270 is an
ordinary u16 accumulator (no dual-view field / union member / MMIO). Every two-load source is
CSE-merged OR the reviewer-FAILED dual-typed read. VERDICT: KILLED (no natural two-load source).

### H1 — kind-split: reviewer PASS this session (banked clean). floor 6 alone / part of floor 8 clean.

### H-new (s2) — the dual-typed read is reviewer-FAILED but target PROVABLY requires it -> RULING
Fresh cheat-reviewer FAILED the guarded dual-typed read (Tests 1/3/5) and the unconditional form
was FAILED prior. Yet target's .L8001FA60 has `lh $v0,0x270; lhu $v1,0x270` adjacent, which under
GCC 2.7.2 only comes from dual-typed C source. Reviewer routes this to the OWNER as a new-family
policy question. => ruling-request (frontier item 3). Honest clean floor without it = 8.

### H-s2c — u16-local-PHI truncation defeats the fold at REGISTER level (KILLED, banned family)
Probe: `u16 raw_or_3;` assigned in a branch (PHI of {3,(u16)probe}), `idx=((s32)raw_or_3<<16)>>15`.
Mechanism: the PHI blocks GCC's back-substitution of the single-value truncation, so the u16
narrowing is NOT subsumed by the <<16 (unlike the non-branched `probe & 0xFFFF` which GCC folds).
GCC can't prove the shift operand fits s16 -> no `x*2` fold -> unfolded sll16;sra15.
Result: sandbox floor 4, build_insns 106 (< clean floor 8). One dereference; the truncation
materializes in-register (andi) where target has a 2nd memory load (lhu) -> 1 insn short, cannot
reach 0 without becoming the memory dual-read (pre-banned distance-0 form). Dominated by
signed-cast-single-read.c (floor 0 cheat).
Verdict: KILLED. Fresh cheat-reviewer FAIL (Tests 1/2/3/5); zero observable effect verified
algebraically; 5th spelling of the pre-banned signedness-split family. CORRECTS the s2 P2b
over-claim ("no register-level construct can force the opaque shape"). Search-space eliminated;
dichotomy confirmed at register level. src/ kept at clean floor-8.

## [s2] A branch-PHI u16-typed intermediate (not a 2nd memory dereference) can force GCC to emit target's unfolded (raw<<16)>>15 shape at the +0x270 clamp, beating the clean floor 8.
- mechanism: u16 raw_or_3 assigned in an if/else (PHI of {3,(u16)probe}) blocks GCC back-substitution, so the truncation is NOT subsumed by the following <<16 (unlike the non-branched `probe & 0xFFFF` which GCC eliminates+folds). GCC then cannot prove the shift operand fits signed-16, so it does NOT fold (x<<16)>>15 -> x*2; it emits sll16;sra15 matching target's shape.
- probe: Applied `u16 raw_or_3; if(probe>=4)raw_or_3=3; else raw_or_3=probe; idx=((s32)raw_or_3<<16)>>15;` (single s16 dereference for probe); sandbox --disable all.
- result: sandbox floor 4, build_insns 106 (target 107) — beats clean floor 8. But only ONE dereference: the u16 truncation materializes in-register (andi) where target has a SECOND memory load (lhu), so it is 1 insn short and cannot reach 0 without becoming the memory dual-read. Fresh cheat-reviewer FAIL (Tests 1/2/3/5): the u16 type has ZERO observable effect (algebraically verified ((s32)(u16)probe<<16)>>15 == (probe<<16)>>15 for all probe), so it exists only to defeat the fold = same signedness-split/CSE-defeat intent the Judge pre-banned in ANY spelling. 5th spelling; strictly dominated by the known floor-0 memory dual-read cheat.
- verdict: KILLED

## [s3] A pure UNSIGNED single read of +0x270 (no dual view/cast/split/PHI) defeats the (raw<<16)>>15 -> raw*2 fold and beats the clean floor 8.
- mechanism: `u32 probe=*(u16*)(a0+0x270); if(probe>=4U)raw=3; else raw=probe; idx=(raw<<16)>>15;`. GCC cannot prove a u16-typed value (bit15 may be set) fits signed-16, so it does NOT fold (x<<16)>>15 to x<<1 (unlike the s16 form, which fits and folds). Emits unfolded sll16;sra15 matching target's shape.
- probe: Applied the unsigned read to the +0x270 block (rest = clean floor-8 form); sandbox --disable all + objdump.
- result: sandbox floor 6, build_insns 106 (target 107). Disasm: ONE lhu, sltiu, sll16;sll16;sra15 (fold defeated). CORRECTS the s2/s2c over-claim that "only a second TYPED memory view defeats the fold" — a single unsigned read does. BUT it forfeits target's SIGNED compare (emits sltiu, target has slti; semantically different for field>=0x8000). Floor 6 has the missing 2nd load AND the wrong compare; cannot reach 0 without adding a signed view = the pre-banned dual read. Dominated by floor-0 signed-cast-single-read.c.
- verdict: KILLED. Re-proves the dichotomy from a fresh angle: single typed read gives {signed=fold=floor8} XOR {unsigned=unfold-but-sltiu=floor6}, never both. src/ kept at clean floor-8. rejected/unsigned-single-read.c.

## [s2] The s2 P2b claim 'NO register-level construct can force the opaque shape; only a second TYPED memory view of the field works' is correct.
- mechanism: s2 only tested the NON-branched register mask `raw_or_3 = probe & 0xFFFF`, which GCC eliminates (subsumed by <<16) then folds on probe's sign-extension knowledge -> floor 8.
- probe: Tested a DIFFERENT register-level construct: a branch-PHI narrow-type (u16) truncation, which s2 never measured.
- result: FALSE. The branch-PHI u16 truncation IS a register-level (single-dereference) construct that defeats the fold and reaches floor 4. The correct statement: register-level fold-defeat exists, but it is still the signedness-split family (no semantic purpose) and still cannot byte-match without the 2nd memory load.
- verdict: KILLED

## [s3] A pure UNSIGNED single read of +0x270 (one deref, no cast/split/union/branch-PHI) defeats the (raw<<16)>>15 -> raw*2 fold and beats the clean floor 8.
- mechanism: GCC 2.7.2 cannot prove a u16-typed value (bit15 may be set) fits signed-16, so (x<<16)>>15 is NOT equivalent to x<<1 for it and it keeps the unfolded sll16;sra15 pair matching target. The signed s16 form folds because s16 provably fits signed-16 (floor 8). Read-signedness alone toggles the fold.
- probe: Applied `u32 probe=*(u16*)(a0+0x270); if(probe>=4U)raw=3; else raw=probe; idx=(raw<<16)>>15;` (rest of function = clean floor-8 form); sandbox --disable all + mipsel objdump of the sandbox .o.
- result: sandbox floor 6, build_insns 106 (target 107). Disasm: ONE `lhu 0x270`, `sltiu v0,v1,4`, `sll v0,v1,16; sll; sra v0,v0,15` (fold DEFEATED). But it emits sltiu where target has slti (signed) and is missing target's 2nd load; cannot reach 0 without adding a signed view = the pre-banned dual read; semantically different for field>=0x8000; strictly dominated by the floor-0 signed-cast-single-read.c.
- verdict: KILLED

## [s4] A decomp-permuter campaign from the clean floor-8 chassis (random type/expr mutation) finds a clean sub-8 lever or byte-match that the manual structural search missed.
- mechanism: The permuter mutates types (s16<->u16, casts), statement order, and expression shape; if any clean construct beats the folding dichotomy, ~95k iterations of simulated annealing should surface it (base score 615 -> target 0 achievable only if a closing form exists in the clean search space).
- probe: Hand-built workspace (minimal self-contained base.c, target.o at offset 0, --stack-diffs ON; base=105 insns vs target 107, permuter base score 615). Launched `permuter_campaign.py`, waited in-turn ~40 min / ~95k iters, harvested + stopped.
- result: KILLED. Best score plateaued at 320 (from 615) and never improved past it over ~85k further iters. The 320 form is NOT a match and reached 320 only via TWO cheats: `volatile short pad;` (dead frame-forcer) + `raw_or_3 = raw_or_3;` (dead self-assign). Even with the cheats the +0x270 fold gap is untouched. The randomizer never generated the banned dual-typed read (only distance-0 form). Confirms the s1-s3 dichotomy from a fresh angle: no clean sub-8 lever exists; the 8-byte frame is a coupled byproduct of the dual-read, not an independent clean lever. rejected/permuter-volatile-pad-frame.c.
- verdict: KILLED

## [s3] The s2/s2c ledger claim 'NO single-typed read defeats the fold; only a second TYPED memory view of the field works' holds.
- mechanism: s2/s2c only tested signed single-read (folds->floor8), the non-branched `&0xFFFF` register mask (GCC eliminates+folds->floor8), and the u16 branch-PHI truncation (floor4). They never tested a plain unsigned single-read comparison.
- probe: Measured the untested pure-unsigned single-read spelling directly.
- result: FALSE. A plain unsigned single read defeats the fold (floor 6, unfolded sll16;sra15) with ONE deref and no dual view. The corrected statement: read-signedness alone toggles the fold; unsigned unfolds (but forfeits the signed compare), signed folds.
- verdict: KILLED

## [s4] A decomp-permuter campaign from the clean floor-8 chassis (random type/expr/order mutation) finds a clean sub-8 lever or byte-match that the manual structural search (s1-s3) missed.
- mechanism: Simulated-annealing mutation of read signedness (s16<->u16), value casts, statement order, and expression shape; if any clean construct beats the (raw<<16)>>15 -> raw*2 fold at +0x270, ~95k iterations should surface it (base permuter score 615; a byte-match at 0 is reachable only if a clean closing form exists in the search space).
- probe: Hand-built permuter workspace (tmp/grind/func_8001F938/s4/ws): minimal self-contained base.c (int typedefs + the fn only; verified isolated compile == full-TU, base 105 insns vs target 107 == sandbox build_insns), target.o from asm/funcs at offset 0, --stack-diffs ON (target has a real 8-byte frame). Launched permuter_campaign.py, waited in-turn ~40 min / ~95k iters, harvested + --stop.
- result: Best score plateaued at 320 (from base 615) at ~11 min and never improved across ~85k further iters. The 320 form is NOT a match and reached 320 only via two cheats: `volatile short pad;` (dead frame-forcer for target's 8-byte frame) + `raw_or_3 = raw_or_3;` (dead self-assign, Lever-D). Even with both cheats the +0x270 fold gap is untouched; the randomizer never generated the banned dual-typed read (the only distance-0 form).
- verdict: KILLED
