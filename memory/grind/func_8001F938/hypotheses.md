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

## [s2] The s2 P2b claim 'NO register-level construct can force the opaque shape; only a second TYPED memory view of the field works' is correct.
- mechanism: s2 only tested the NON-branched register mask `raw_or_3 = probe & 0xFFFF`, which GCC eliminates (subsumed by <<16) then folds on probe's sign-extension knowledge -> floor 8.
- probe: Tested a DIFFERENT register-level construct: a branch-PHI narrow-type (u16) truncation, which s2 never measured.
- result: FALSE. The branch-PHI u16 truncation IS a register-level (single-dereference) construct that defeats the fold and reaches floor 4. The correct statement: register-level fold-defeat exists, but it is still the signedness-split family (no semantic purpose) and still cannot byte-match without the 2nd memory load.
- verdict: KILLED
