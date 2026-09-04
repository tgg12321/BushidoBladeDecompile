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

## [s5] A decomp-permuter campaign seeded from the STRUCTURALLY-DIFFERENT unsigned floor-6 chassis (never explored by s4's floor-8 basin) finds a clean sub-8 lever or byte-match.
- mechanism: The unsigned read already defeats the (raw<<16)>>15 -> raw*2 fold (floor 6) but forfeits target's signed compare (sltiu vs slti). Seeding the random search HERE explores a different neighborhood than s4's floor-8 signed basin; a mutation restoring the signed compare while keeping the unfolded index would, if clean, reach a match. Base permuter score 705 (106-insn chassis).
- probe: Hand-built s5 workspace (unsigned floor-6 base.c, 106 insns; s4 target.o at offset 0; --stack-diffs ON). Launched permuter_campaign.py (pid 405), drove the fresh-seed window in-turn (~19 min / 36,031 iters) via wait+monitor loops, harvest --stop. Inspected best form's +0x270 region.
- result: Best plateaued at 505 (from 705), flat for the whole window. The best form's +0x270 crux is UNTOUCHED (still unsigned+sltiu); the 505 gain came from unrelated branch/frame noise. Randomizer never restored the signed compare and never generated the banned dual-typed read. Confirms from a SECOND basin that no clean sub-8 lever exists; the signedness-split dichotomy is inescapable by random search. Dominated by / == rejected/unsigned-single-read.c.
- verdict: KILLED

## [s5] A decomp-permuter campaign seeded from the STRUCTURALLY-DIFFERENT unsigned-read floor-6 chassis (never explored by s4, which only ran the clean floor-8 signed basin) finds a clean sub-8 lever or byte-match that the floor-8-basin search missed.
- mechanism: The unsigned read (`u32 probe=*(u16*)(a0+0x270)`) already defeats the (raw<<16)>>15 -> raw*2 fold (floor 6) but forfeits target's signed compare (emits sltiu vs target slti). A random search seeded HERE explores a different neighborhood than the floor-8 signed basin: mutations that restore the signed compare while keeping the unfolded index would, if any clean one exists, reach a match. Base permuter score 705 (106-insn chassis vs target 107); -j 8.
- probe: Built tmp/grind/func_8001F938/s5/ws (minimal self-contained base.c = unsigned floor-6 chassis, verified compiles to 106 insns; reused s4's target.o at offset 0; --stack-diffs ON). Launched permuter_campaign.py (pid 405), drove the fresh-seed window in-turn (~19 min, 36,031 iterations) via wait/monitor loops, then harvest --stop. Inspected the best form's +0x270 region and disasm-relevant structure.
- result: Best score plateaued at 505 (from base 705) and stayed flat for the entire observation window (5 stale samples / ~7 min continuous, observer loop saw best=505 from t=5s to campaign death). The best form's +0x270 crux is UNTOUCHED: still the plain unsigned read with sltiu; the fold stays defeated but the signed compare stays forfeited. The 505 gain over 705 traces entirely to unrelated branch-address / frame scheduling noise, NOT to closing the +0x270 gap. The randomizer NEVER restored the signed compare and NEVER generated the banned dual-typed read (the only distance-0 form) in 36k iters.
- verdict: KILLED

## [s6] The fold (raw<<16)>>15 -> raw*2 that keeps the clean floor-8 form from matching target's unfolded +0x270 index is performed by the `combine` pass, not cse/loop.
- mechanism: Instrumented cc1 dumps: in RTL/cse/jump/flow the index is TWO insns `(ashift X 16)`+`(ashiftrt Y 15)` in BOTH the clean (A) and distance-0 (C) forms. Only in the combine dump does A collapse to a single `(ashift (reg/v 125) 1) 181{ashlsi3}`, while C keeps `(ashift (reg/v 124) 16)`+`(ashiftrt (reg 128) 15) 191{ashrsi3}` intact. The collapse appears at exactly one pass boundary = combine.
- probe: Compiled A_clean.c (single s16 read) and C_dist0.c (u16 read + (s16) cast) through cpp|cc1 -O2 -G0 with -dr -dj -ds -dc -dl -dg -dR -df; grep ashift/ashiftrt across the per-pass dumps.
- result: A folds to `sll,1` at combine (floor-8 divergence); C's pair survives combine unfolded and byte-matches target .L8001FA60. cse/flow keep the pair in both.
- verdict: CONFIRMED

## [s6] combine folds only when the shift operand has >16 sign-bit copies; a signed (lh) read gives 17 -> fold, an unsigned (lhu) read gives exactly 16 -> fold refused. This IS the s1-s5 signedness dichotomy, at the pass level.
- mechanism: combine.c simplify_shift_const rewrites (ashiftrt (ashift X 16) 15)->(ashift X 1) iff num_sign_bit_copies(X) > 16. The only difference between reg/v 125 (A, folds) and reg/v 124 (C, does not) is the else-arm value of raw_or_3: probe=sign_extend(mem)=17 copies vs u=zero_extend(mem)=16 copies (bit15 significant). The {3,else} PHI keeps the min; 3 exceeds 16 so the read signedness alone toggles the fold at the ==16 boundary.
- probe: Compared the folded (A) vs unfolded (C) combine-dump insns and traced each shift operand's provenance to lh(sign_extend) vs lhu(zero_extend) in the RTL dump.
- result: Signed single read => 17 => fold => floor 8 (wrong, folded index). Unsigned single read => 16 => unfold but sltiu compare (s3 floor 6). Only a dual/typed-cast view supplies both 16-copy-index AND signed-compare = distance 0 = the pre-banned family.
- verdict: CONFIRMED

## [s6] The prior ledger claim that 'GCC 2.7.2 CSE always merges two same-address HImode reads into ONE load' is imprecise; the second load in the distance-0 form is manufactured by combine, and cse never merges anything (clean sources have only one deref).
- mechanism: Per-pass count of mem refs at +0x270 (const_int 624) in C_dist0: rtl=1 jump=1 cse=1 flow=1 | combine=2 lreg=2 greg=2 sched2=2. combine simplifies the (s16)u compare operand (sign_extend-via-shifts of the zero-extended lhu) into a direct (sign_extend:SI (mem:HI)) second lh, cheaper than shifting. cse holds a single ref throughout.
- probe: grep -c '(const_int 624)' and zero_extend/sign_extend mem:HI across C_dist0's per-pass dumps.
- result: The load count goes 1->2 exactly at combine; cse/flow keep 1. The empirical dichotomy is unchanged; only the pass attribution (cse -> combine) is corrected.
- verdict: CONFIRMED

## [s7] The combine fold gate is num_sign_bit_copies of the SHIFT OPERAND, not the memory LOAD's signedness (s6's framing is imprecise).
- mechanism: simplify_shift_const folds (ashiftrt (ashift X 16) 15)->(ashift X 1) iff num_sign_bit_copies(X)>16, where X is the shift operand AS combine sees it after its own simplifications. The load's signedness only sets this indirectly.
- probe: New control variant B_u16phi (signed `s16` lh read for the compare, but a `u16`-typed branch-PHI raw_or_3 as the index operand) compiled through instrumented cc1; compared combine RTL + emitted asm + mem-ref count vs A_clean/C_dist0. s6 had only dumped A and C.
- result: CONFIRMED. B_u16phi emits a SIGNED `lh` load (mem-count stays 1, byte-identical load to A) yet the index UNFOLDS: combine operand `(ashift:SI (subreg:SI (reg/v:HI 125) 0) 16)` is a HImode subreg = 16 sign copies -> fold refused, same as C. asm `lh $2,624; move $3,$2; slt; bne; sll $3,16; sra 15` = floor 4. A signed load that unfolds decouples fold-refusal from load-signedness -> the gate is the operand's sign-copies. Reconfirmed by D_andmask (branched `probe & 0xFFFF` -> operand `(and reg 0xFFFF)` = 16 copies -> unfold, single lh + andi). D also clarifies s2 P2b: the &0xFFFF folds ONLY in the non-branched inline form (combine drops the redundant AND, recovers 17 copies); across a PHI join the AND survives -> 16 copies -> unfold. rejected/branched-and-mask-register-fold-defeat.c.
- verdict: CONFIRMED

## [s7] Reaching target's distance-0 byte shape requires the 16-sign-copy shift operand to be a SECOND MEMORY LOAD, obtainable only from a second typed MEMORY view; register-level fold-defeat caps at floor 4.
- mechanism: Target's byte-exact .L8001FA60 is `lh;lhu;…;sll16` — the 16-copy index operand is a 2nd memory load (lhu). combine manufactures that 2nd load ONLY by rewriting the sign-extend-via-shifts of a zero-extended lhu (the `(s16)u` dual TYPED memory view) into a direct `(sign_extend:SI (mem:HI))`=2nd lh (mem-count 1->2 at combine). A register-level 16-copy value is materialized by `move`/`andi` (mem stays 1) and can never become the 2nd lhu.
- probe: Compared mem-ref counts + operand provenance across A_clean (fold, 1 load), C_dist0 (unfold, 2 loads), B_u16phi (unfold, 1 load), D_andmask (unfold, 1 load).
- result: CONFIRMED. Distance-0 requires BOTH (i) 16-copy operand (unfold) AND (ii) that operand delivered as a 2nd memory load. (i) alone -> floor 4 (B/D, register-level). (i)+(ii) jointly satisfiable ONLY by a 2nd typed memory view of +0x270 = the pre-banned signedness-split family. Pass-level proof that the shipped bytes required two typed memory views. No clean sub-8 lever exists; disposition unchanged (owner FAILed the family twice; keep INCOMPLETE; only the non-structural SOTN census remains).
- verdict: CONFIRMED

## [s7] The combine fold gate is num_sign_bit_copies of the SHIFT OPERAND, not the memory LOAD's signedness (s6's 'signed load->fold / unsigned load->no-fold' framing is imprecise).
- mechanism: simplify_shift_const folds (ashiftrt (ashift X 16) 15)->(ashift X 1) iff num_sign_bit_copies(X)>16, X = the shift operand as combine sees it after its own simplifications; the load signedness only sets it indirectly.
- probe: New control B_u16phi (signed s16 lh read for the compare + u16-typed branch-PHI raw_or_3 as index operand) through instrumented cc1; compared combine RTL, emitted asm, and const_int-624 mem-ref count vs A_clean/C_dist0 (s6 dumped only A and C). Plus D_andmask (branched probe & 0xFFFF).
- result: B_u16phi emits a SIGNED lh (mem-count stays 1, byte-identical load to A) yet UNFOLDS: combine operand (ashift (subreg:SI (reg/v:HI 125) 0) 16) is a HImode subreg = 16 sign copies -> fold refused; asm lh;move;slt;bne;sll16;sra15 = floor 4. A signed load that unfolds decouples fold-refusal from load signedness. D_andmask reconfirms (operand (and reg 0xFFFF)=16 copies -> unfold, single lh + andi) and clarifies s2 P2b: &0xFFFF folds only in the NON-branched inline form (combine drops the redundant AND, recovers 17 copies); across a PHI join the AND survives -> unfold.
- verdict: CONFIRMED

## [s7] Reaching target's distance-0 byte shape requires the 16-sign-copy shift operand to be a SECOND MEMORY LOAD, obtainable only from a second typed MEMORY view; register-level fold-defeat caps at floor 4.
- mechanism: Target .L8001FA60 = lh;lhu;...;sll16 — the 16-copy index operand is a 2nd memory load. combine manufactures it ONLY by rewriting the sign-extend-via-shifts of a zero-extended lhu (the (s16)u dual typed memory view) into a direct sign_extend(mem:HI)=2nd lh (const_int-624 count 1->2 at combine). A register-level 16-copy value is materialized by move/andi (mem stays 1) and can never become the 2nd lhu.
- probe: Compared mem-ref counts + operand provenance across A_clean (fold, 1 load), C_dist0 (unfold, 2 loads), B_u16phi (unfold, 1 load), D_andmask (unfold, 1 load).
- result: Distance-0 requires BOTH (i) a 16-copy operand (unfold) AND (ii) that operand delivered as a 2nd memory load. (i) alone -> floor 4 (B/D). (i)+(ii) jointly satisfiable ONLY by a 2nd typed memory view of +0x270 = the pre-banned signedness-split family. Pass-level proof that the shipped bytes required two typed memory views of the field.
- verdict: CONFIRMED

## [s7] s6 FINDING 1 (fold is a combine event) and FINDING 2 (the 2nd same-address load is a combine byproduct, not a cse merge) reproduce exactly on an independent recompile.
- mechanism: verify-opus-handoff-claims: s6 (opus) corrected three prior sessions' 'CSE' attribution to 'combine'; that correction itself warranted independent confirmation.
- probe: Fresh cpp|cc1 -O2 -G0 with -dr -dj -ds -dc -dl -dg -dR -df on A_clean and C_dist0; checked ashift/ashiftrt collapse per pass and const_int-624 mem-ref count per pass.
- result: A_clean collapses to (ashift (reg/v:SI 125) 1) at combine; C_dist0 keeps (ashift 124 16)+(ashiftrt 128 15); cse/jump/flow keep the pair in both. C_dist0 mem-ref count = 1 through cse/flow, 2 at combine (lreg/greg/sched2). Byte-identical to s6's committed dumps. s6 confirmed.
- verdict: CONFIRMED

## [s8] The func_8009AA68 mechanism (a CLEAN gcc2.7.2 scratch emits adjacent lh+lhu of a single s16 field) transfers to func_8001F938's +0x270, producing the two loads from a single typed view without the banned dual-typed read.
- mechanism: decomp.me census found CW0dj func_8009AA68 (score 0, matched) emits `lh $v0,0x6($s2); lhu $v1,0x6($s2)` of ONE `s16 angle` field from ordinary pure C: `if(entity->angle<0x1000){ s16 n=entity->angle+0x800; ...}` — compare -> lh+slti, `+0x800` stored to s16 -> lhu. Hypothesis: mirror this on +0x270 (separate re-reads, no reused local) to get GCC to reload the index with lhu while the compare uses lh, all from a single `*(s16*)` view.
- probe: Applied `if(*(s16*)(+0x270)>=4) raw=3; else raw=*(s16*)(+0x270); idx=(raw<<16)>>15;` (clean floor-8 chassis otherwise); sandbox --disable all + objdump.
- result: sandbox score 9, build_insns 104 (WORSE than clean floor 8). Disasm: ONE `lh v1,624`; the two re-reads CSE-merged; combine FOLDED (raw<<16)>>15 -> `sll,1` (move v0,v1; sll v0,v0,1). No 2nd load. KILLED. WHY it can't transfer: `(field<<16)>>15` is sign-SENSITIVE (bit15 of the field becomes the RESULT's sign; s3: signed read folds->floor8, unsigned read gives sltiu->floor6), so a signed-compare view CANNOT also serve the index via lhu — the two views carry different values out of range and are non-interchangeable. func_8009AA68's `angle+0x800->sh` second use is sign-INSENSITIVE (narrowing store keeps only low 16 bits), which is the ONLY reason GCC there was free to load lhu for the second use from a single view. So the two clean corpus dual-loads (func_8009AA68 narrowing; func_800A3320 multi-width word+halfword) are BOTH sign-insensitive; func_8001F938's sign-sensitive index has no such freedom and genuinely needs a programmer-authored 2nd typed memory view = the pre-banned signedness-split family (s7 combine theorem, dump-proven).
- verdict: KILLED

## [s8] A structurally-different overall C shape (fresh m2c / decomp.me / Kengo transplant) reaches the +0x270 byte target without the pre-banned signedness-split dual-typed read.
- mechanism: The prior structural search (s1-s3) all shared candidate.c's control-flow framing. A genuinely different decompilation (m2c's switch+nested-if, a decomp.me sibling, or a Kengo source-shape transplant) might compute the index via a construct outside the banned family.
- probe: (1) fresh m2c decompile; (2) decomp.me whole-function similarity + clamp-idiom corpus scan + dual-load census; (3) Kengo match lookup.
- result: KILLED. (1) m2c INDEPENDENTLY reconstructs the +0x270 block with TWO typed views (a `(u16)` cast index read + a signed compare read) despite a totally different overall shape — the crux is structure-invariant. (2) No structurally-similar corpus function (top 0.082); clamp idiom appears in 0 clean scratches; the only clean corpus dual-loads are sign-insensitive (narrowing/multi-width), no precedent for the sign-sensitive redundant split. (3) Kengo match is size-only-ambiguous (insn-count coincidence, 24 candidates, opseq 0.24), no semantic sibling. The overall structure is orthogonal to the +0x270 crux; no rederived shape escapes the banned family.
- verdict: KILLED

## [s8] The func_8009AA68 mechanism (a CLEAN score-0 gcc2.7.2 decomp.me scratch emits adjacent lh+lhu of a single s16 field from ordinary pure C) transfers to func_8001F938's +0x270, producing the two loads from a single typed view without the banned dual-typed read.
- mechanism: func_8009AA68 (CW0dj) emits `lh $v0,0x6($s2); lhu $v1,0x6($s2)` of one `s16 angle` field: `angle<0x1000` compare -> lh+slti; `angle+0x800` stored to s16 -> lhu. Mirror it on +0x270 with separate re-reads (no reused local) so GCC reloads the index with lhu while the compare uses lh.
- probe: Applied `if(*(s16*)(+0x270)>=4) raw=3; else raw=*(s16*)(+0x270); idx=(raw<<16)>>15;` on the clean floor-8 chassis; sandbox --disable all + mipsel objdump.
- result: sandbox score 9, build_insns 104 (WORSE than clean floor 8). Disasm: ONE `lh v1,624`; the two re-reads CSE-merged; combine FOLDED (raw<<16)>>15 -> `sll,1`. No 2nd load. Cannot transfer because `(field<<16)>>15` is sign-SENSITIVE (bit15 -> result sign; s3: signed folds/floor8, unsigned sltiu/floor6), so a signed-compare view cannot also serve the index via lhu -- unlike func_8009AA68's `angle+0x800->sh` narrowing second use which is sign-INSENSITIVE.
- verdict: KILLED

## [s8] A structurally-different overall C shape (fresh m2c / decomp.me sibling / Kengo transplant) reaches the +0x270 byte target without the pre-banned signedness-split dual-typed read.
- mechanism: Prior structural search (s1-s3) shared candidate.c's framing; a genuinely different decompilation might compute the index outside the banned family.
- probe: Fresh m2c decompile; decomp.me whole-function similarity + clamp-idiom scan + dual-load census over 3754 gcc2.7.2 scratches; Kengo match lookup.
- result: KILLED. m2c INDEPENDENTLY reconstructs the +0x270 block with TWO typed views ((u16) index read + signed compare read) despite a switch+nested-if shape -> crux is structure-invariant. No similar corpus function (top 0.082); 0 clean clamp-idiom scratches; the only clean corpus dual-loads (func_8009AA68 narrowing, func_800A3320 word+halfword) are sign-INSENSITIVE, no precedent for the sign-sensitive redundant split. Kengo match is size-only-ambiguous (insn-count coincidence, opseq 0.24), no semantic sibling.
- verdict: KILLED

## [s9] +0x270 is a genuine dual-view / multi-width / union field, so target's lh;lhu is a legitimate multi-purpose access (like s8's clean corpus precedents func_800A3320 word+halfword and func_8009AA68 narrowing store) rather than the banned signedness-split -- which would permit a clean distance-0 form.
- mechanism: If +0x270 were two overlapping fields or a multi-width field, reading it signed for the compare and unsigned for the index would carry real semantic purpose (two fields) and escape the no-semantic-purpose cheat test, unlike a redundant split of one field.
- probe: BB2-INTERNAL write-site census of the +0x26E/+0x270/+0x272 cluster (distinct from s8's external decomp.me/Kengo corpora): grep all read/write/address-taken sites in src/ and read the field types at the write sites.
- result: Write sites in src/code6cac_b.c (func_80027438, per-status-byte damage accumulator) are `*(u16*)(a0+0x270)+=a2` / `+0x272` / `+0x26E` -- SINGLE u16 accumulator fields, no union, no multi-width. Address-taken at text1a.c:142 as a table base. Target reads the IDENTICAL single u16 field both signed (lh->slti compare) and unsigned (lhu->sll index); the roles differ only in signedness of one field. Sign-SENSITIVE with no second field, unlike the clean corpus precedents (distinct sub-fields / sign-insensitive narrowing).
- verdict: KILLED

## [s9] header-type-correction-from-use-sites applies: the field is genuinely u16, so correcting its type and reading it consistently u16 yields a clean byte-match.
- mechanism: A one-extern-edit signedness correction is a sanctioned lever when the correct type produces target bytes at every use site.
- probe: Combine the census (field proven u16) with s3's measured pure-unsigned read of +0x270 (floor 6): does the correct u16 type close the function?
- result: Reading +0x270 consistently u16 forfeits target's SIGNED compare -- s3 measured floor 6 (emits sltiu; target has slti). Target's compare is intrinsically signed AND its index intrinsically unsigned OF THE SAME FIELD; no single type serves both roles. header-type-correction is a one-edit remedy; this needs two simultaneous typed views of one field = the banned construct. The correct type cannot close it.
- verdict: KILLED

## [s10] A genuinely un-tried pure-C lever exists that drops the honest clean floor below 8 without the pre-banned signedness-split dual-typed read of +0x270.
- mechanism: Distance-0 requires combine to keep the (x<<16)>>15 index UNFOLDED (16-sign-copy shift operand) AND to deliver that operand as a 2nd memory load (target's lhu); s6/s7 dump-proof shows (i)+(ii) are jointly reachable only via a 2nd typed memory view of the field = the banned family, and register-level fold-defeat caps at floor 4.
- probe: Re-surveyed the full ledger (s1-s9) across structural/permuter/forensics/rederive; re-measured the clean floor-8 chassis live (sandbox --disable all = 8, build_insns 105 vs target 107); ran scan_hand_coded (tier LOW 0/8); confirmed the byte-match is held by 13 regfix rules (0 asmfix, 0 cheat-asm).
- result: No un-tried lever. Every fold-defeat spelling is either CSE/combine-folded back to floor 8 or is the signedness-split family; the distance-0 form (rejected/signed-cast-single-read.c) is that family and is SOTN NOT ESTABLISHED. The floor is a proven dichotomy, not a search gap.
- verdict: KILLED

## [s11] The owner directive's recommended solver modality (ra_solver / sched_solver) can attack this residual.
- mechanism: the queue item's owner directive (2026-08-24, escalation-not-parked) recommends classifying RA/scheduler-tiebreak residuals with the solver suite before a deep re-grind. `inverse_compose.py classify` triages a residual into PRE-RA / RA / SCHED by comparing register-blanked instruction multisets: only an RA or SCHED verdict is in the solvers' territory.
- probe: `bash tools/ra_solver/mkasm_honest.sh code6cac` then `inverse_compose.py classify code6cac func_8001F938`; plus a hand-written opcode-multiset diff of the honest stream vs `asm/funcs/func_8001F938.s` with maspsx spelling aliases folded (bnez/beqz<->bne/beq, sltiu<->sltu-imm, addiu<->addu-imm, jr<->j $ra).
- result: KILLED (directive executed, verdict FORECLOSED). NOTE the tool trap: `classify` derives its "target" stream from `src/` + regfix, so with the candidate applied to src/ it compared the honest build against ITSELF and printed "IDENTICAL" — a vacuous answer for an asm-until-matched function. The hand diff is the real answer: honest 105 insns vs target 107, with target carrying +1 `lhu`, +1 `sra`, +1 `addiu` and the honest stream +1 `nop`. The MULTISETS DIFFER, which is `classify`'s own definition of PRE-RA: no register renaming or reordering can reach it. ra_solver and sched_solver are structurally inapplicable to this function; the directive is discharged.
- verdict: KILLED

## [s11] An opaque constant holder (`s32 three = 3;` feeding the clamp arm) lowers the min sign-bit-copy count across the PHI and so defeats the combine fold that pins the clean form at floor 8.
- mechanism: s6/s7 proved the fold `(x<<16)>>15 -> x*2` fires iff num_sign_bit_copies(shift operand) > 16, and that the operand is the PHI of {3, probe}. A constant-holder local whose value combine cannot see would drop the PHI minimum below 17 and refuse the fold — the sanctioned constant-holder family (`.claude/rules/named-local-fake-exception.md`) rather than a signedness split.
- probe: applied `s32 three = 3; ... raw_or_3 = three;` to the clean floor-8 chassis; sandbox --disable all.
- result: KILLED. score 8, build_insns 105 — byte-identical to the chassis without it. cse/constant-propagation folds the plain constant local away long before combine, so the PHI still sees a literal 3 (30 sign copies) and the fold fires. The constant-holder family buys nothing here; it is not a fold lever.
- verdict: KILLED

## [s11] Target's TWO same-address loads at +0x270 (`lh $v0` + `lhu $v1`) can only come from a SECOND TYPED MEMORY VIEW in the C (the pre-banned signedness-split family) — the s7 theorem and the whole s1-s10 floor-8 dichotomy.
- mechanism: s7 concluded distance 0 needs (i) a 16-sign-copy shift operand and (ii) that operand delivered as a second memory load, and that (i)+(ii) are jointly reachable only by a dual typed view, register-level fold-defeat capping at floor 4.
- probe: put the clamped value in a `s16` (HImode) LOCAL initialised from the single existing signed read, and scale with `* 2`: `s16 raw_or_3 = *((s16 *)(arg0 + 0x270)); if (raw_or_3 >= 4) { raw_or_3 = 3; } idx = raw_or_3 * 2;`. Measured with sandbox --disable all, then normalised-diffed the honest stream against asm/funcs/func_8001F938.s.
- result: **FALSE — the theorem's premise (ii) is wrong.** cc1 keeps `raw_or_3` as a HImode pseudo and materialises it with `lhu $v1,624($a0)`, while the `>= 4` comparison needs a sign-extended SImode operand which cc1 supplies with a SEPARATE `lh $v0,624($a0)`. ONE C dereference of ONE type produces BOTH of target's loads. Sandbox score **0**, build_insns 107 == target_insns 107; normalised asm diff vs target is empty except for local label names; full-build `verify-oracle` exit 0 (SHA1 == oracle). The 8-byte frame that s4's permuter could only reach with a `volatile short pad` cheat is the HImode local's own stack slot and appears naturally. The floor-8 "dichotomy" of s1-s10 was a SEARCH GAP: every session held the clamped value in an `s32` and spelled the scale as `(raw << 16) >> 15`, which forced the fold question that the banned family answered. `difficult-is-not-impossible` vindicated.
- verdict: KILLED (the hypothesis; the function is MATCHED)

## [s11] Intermediate rungs measured on the way to 0 (recorded so the next session never re-walks them).
- mechanism: n/a — measurement record.
- probe: sandbox --disable all on each spelling of the +0x270 block, clean chassis otherwise.
- result: `s32 probe` + `s16 raw_or_3` PHI + `* 2` => floor 4 (106 insns; region matches target except `nop; move $v1,$v0` where target has `lhu $v1,624`). `s16 raw_or_3` written in BOTH arms from duplicate signed reads => floor 2 (108 insns; frame now matches, still `move` not `lhu`). `s16 raw_or_3` initialised from ONE read then clamped in place => floor 0 (107). For the 0x6A kind-split, measured alternatives that do NOT reproduce target's `andi $v1,$a1,0xFFFF`: `(u16)kind_full` cast => `move`, floor 1; single `u16 kind` local used for both the `==` set and the range checks => floor 16 (106); `kind_full` alone with no mask => floor 16 (106); a second `*(u16*)` read into a `u16` local => floor 16 (106). The `kind_full` raw + `kind = kind_full & 0xFFFFU` split is the only measured spelling that reaches 0.
- verdict: CONFIRMED

## [s11b] The s11 score-0 form (`s16` local at +0x270) is genuinely OUTSIDE the pre-banned signedness-split / dual-typed-view family, and the 2026-08-25 layer-1 FAIL is a family-scope error rather than a correct family match.
- mechanism: The pre-ban and the 2026-07-23 ruling define the family by a SECOND typed C-level view of one address whose only effect is to stop CSE collapsing two expression nodes ("the second dereference changes nothing about what the program computes"). The s11 form has ONE dereference of ONE type; target's second (`lhu`) load comes from cc1's HImode-pseudo materialisation of the `short` local, while the `>= 4` compare needs a sign-extended SImode operand supplied by a separate `lh`. If the family is read at its stated scope, the construct is ordinary C.
- probe: (1) re-applied the form to src/code6cac.c on the live chassis and re-measured `sandbox --disable all`; (2) ran a full `verify-oracle`; (3) ran `scan_hand_coded --single`; (4) censused this repo's OWN COMPLETED-C corpus for the shape `s16 <name> = *(s16 *)(<base>+<off>);` and checked the enclosing functions' rule/queue/canonical status; (5) renamed the local `raw_or_3` -> `dmg` (T6 hygiene) and re-measured.
- result: (1) score 0, build_insns 107 == target_insns 107, rules_dropped 0 — INDEPENDENTLY REPRODUCED this session (the s11 claim is not taken on trust). (2) `verify-oracle` ok=true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked, with zero regfix/asmfix rules and zero cheat-asm. (3) tier LOW 0/8 — endgame-lock gate (a) fails, as it must when a pure-C form exists. (4) The shape ships in zero-rule byte-matched COMPLETED-C: src/code6cac_b.c:377 inside func_8002798C (0 regfix, 0 asmfix, absent from engine/queue.json and inline_asm_canonical.txt) and src/code6cac.c:777 + :792 inside func_8001B478; 135 narrow-typed locals across src/. (5) rename is codegen-neutral, sandbox 0 both ways. NOT SUBMITTED as candidate-ready: the exact statement is in state.json banned_constructs, and a re-declaration under a different identifier is the same construct — that path is an invalid session by contract. Filed as a RULING REQUEST on family scope instead.
- verdict: CONFIRMED (measurements); the family-scope question itself is OPEN and is the ruling request.

## [s11b] The owner directive's solver modality and the endgame-lock disposition both remain discharged/unavailable for this function.
- mechanism: the 2026-08-24 owner directive recommends ra_solver/sched_solver before deep re-grind; the escalation modality otherwise mandates the endgame-lock AND-gates.
- probe: re-read the s11 solver record (classify verdict PRE-RA, tool trap documented) and re-ran gate (a) `scan_hand_coded --single func_8001F938`.
- result: Solver directive stays DISCHARGED — the residual is PRE-RA (opcode multisets differ), structurally outside both solvers, and in any case the residual is now CLOSED at distance 0 so there is nothing left for a solver to classify. Gate (a) = tier LOW 0/8; gate (b) is moot because the closing construct is claimed as ORDINARY C requiring no family sanction, not as a coercion family needing SOTN precedent. The endgame-lock disposition is therefore NOT the right outcome this session: the floor is not flat (8 -> 0) and the open question is a decidable scope question, not a request to lower a standard (which would be auto-reject class).
- verdict: CONFIRMED

## [s11 / session 11] 2026-08-25 — escalation modality

- H-s11-1: "The 2026-08-25 23:08 layer-1 FAIL on the single-`s16`-local form is a live
  binding constraint, so the function must be disposed of via an endgame-lock packet."
  - mechanism: the dispatch digest lists that FAIL among judge_constraints and the driver
    banned the exact statement text.
  - probe: read docs/grind/decisions.md for the answer to s11b's ruling request; read
    state.json `banned_constructs`.
  - result: the ruling landed **PASS** at 2026-08-25 23:20 and explicitly narrowed the ban to
    exclude this shape; `banned_constructs` is now `[]`.
  - verdict: **KILLED**. The constraint is superseded; disposition is not owed.

- H-s11-2: "With the ruling in hand, the s11 body reaches honest distance 0 and a full-build
  byte match on the CURRENT chassis (the dispatch chassis check reported the HEAD floor
  measurement as unavailable, so every banked conclusion had to be re-measured)."
  - mechanism: one `s16 dmg = *(s16 *)(arg0 + 0x270);` local — cc1 materialises the HImode
    pseudo with `lhu` while the `>= 4` compare needs a sign-extended SImode operand supplied
    by a separate `lh`, reproducing target's two same-address loads from ONE C dereference;
    `* 2` on the HImode pseudo emits `sll 16 ; sra 15` (combine cannot fold to `sll 1`
    because the HImode subreg carries exactly 16 sign-bit copies — the s7 gate), and reorg
    steals the `sll` into the branch delay slot as in target.
  - probe: install candidate.c into src/code6cac.c; `sandbox func_8001F938 --disable all`;
    full `verify-oracle`; re-measure after the header-comment refresh.
  - result: score 0, build_insns 107 == target_insns 107, rules_dropped 0; verify-oracle
    ok=true with build_sha1 == original_sha1_locked; second sandbox run also 0.
  - verdict: **CONFIRMED**. Floor 8 -> 0. Artifacts: tmp/grind/func_8001F938/s11/measurements.txt.

- H-s11-3: "The owner's 2026-08-24 directive (solver modality before deep re-grind) has
  un-executed work owed on this function."
  - mechanism: directive targets RA/scheduler-tiebreak residuals.
  - probe: compare the directive's scope against the s6/s7 pass attribution for the residual.
  - result: the residual was attributed to `combine`/`simplify_shift_const` (a fold gate on
    `num_sign_bit_copies` of the shift operand), not to a register seat or an emission-order
    tie; and it is now closed at distance 0.
  - verdict: **KILLED** (moot). Directive acknowledged in the ledger; no ra_solver/sched_solver
    run is owed for this function.

## s11b (2026-08-25) — escalation / disposition (post-layer-1-FAIL rollback)

- H-s11b-1: "The 2026-08-24 owner directive's recommended solver modality (ra_solver /
  sched_solver) can attack the +0x270 residual."
  - mechanism: both solvers replicate a GCC pass over a fixed instruction set — ra_solver
    models `global.c` / `local_alloc` seat assignment, sched_solver models `sched.c`
    ordering. Both are count-preserving.
  - probe: read both toolkits' scope statements (tools/ra_solver/README.md:1-10,
    tools/sched_solver/README.md:1-16) against the measured residual shape
    (`build_insns` 105 vs `target_insns` 107 — a COUNT deficit: the missing second
    same-address load at .L8001FA60 plus its consumer).
  - result: NOT APPLICABLE. Neither model can create an instruction, so neither can express
    this residual. Directive discharged for this function.
  - verdict: **KILLED**
- H-s11b-2: "An endgame-lock AND-gate might have flipped since s10 (chassis drift, a new
  scanner tier, or a newly-indexed SOTN precedent)."
  - mechanism: both gates are re-measurable in one command each, and the chassis has changed
    under this function before.
  - probe: `python3 tools/scan_hand_coded.py --single func_8001F938`; grep
    `docs/reference/sotn-construct-index.md` for signedness / dual-typed / same-address.
  - result: gate (a) still tier LOW 0/8 with all of S1..S8 clear; gate (b) still zero in-hand
    citations — the index has no detector class for the family at all. Both FAIL exactly as
    at s10.
  - verdict: **KILLED**
- H-s11b-3: "The 2026-08-25 23:20 narrowing ruling reopened a grindable path."
  - mechanism: the ruling claimed the pre-ban did not reach the single-`s16`-local spelling.
  - probe: the layer-1 cheat-reviewer's 23:29 re-review, plus the driver's banned-construct
    list at s11b dispatch.
  - result: the reviewer held that a frozen family is owner-only to extend
    (.claude/rules/judge-sole-gate.md) and that a driver-side narrowing is not a legitimate
    authorization; the driver has since banned both the construct AND the 23:20 entry
    mechanically. No spelling remains: by the ban's own "in ANY spelling" wording, EVERY C
    form that causes cc1 to emit a second load of +0x270 sits inside the frozen family.
  - verdict: **KILLED**

FRONTIER AFTER s11b — exactly one, and it is not a grind modality:
  An OWNER ruling on the frozen signedness-split / redundant dual-typed-read family. If it
  ever moves, func_8001F938 closes to COMPLETED-C with zero further search from the body in
  `rejected/layer1-fail-0825-2329.c` (measured distance 0, full-build oracle verified
  2026-08-25). Until then the correct state is honest INCOMPLETE at clean floor 8 with src/
  at INCLUDE_ASM. A future session should NOT re-open structural / permuter / forensics /
  rederive / solver here — all five are measured dead and the reasons are banked above.

## [s11] The 2026-08-24 owner directive's recommended solver modality (ra_solver / sched_solver) can attack the +0x270 residual.
- mechanism: Both solvers replicate a GCC pass over a FIXED instruction set - ra_solver models global.c / local_alloc seat assignment ('which register', tools/ra_solver/README.md:1-10), sched_solver models sched.c ordering ('which order', tools/sched_solver/README.md:1-16). Both are count-preserving by construction.
- probe: Compared both toolkits' scope statements against the measured residual shape: clean form installed gives build_insns 105 vs target_insns 107, and the missing pair is target's second same-address load at .L8001FA60 plus its consumer (asm/funcs/func_8001F938.s:82-83, lh $v0,0x270($a0) then lhu $v1,0x270($a0)).
- result: NOT APPLICABLE. The residual is an instruction-COUNT deficit - a load our C never causes cc1 to emit. Neither model can create an instruction, so neither can express it. No solver run was spent; the directive is discharged for this function and no future session should spend one here. (Earlier s11 prose called the directive 'moot' because the function was believed closed at distance 0; that closure has since been revoked by two layer-1 FAILs, so the directive needed and now has a real scope answer.)
- verdict: KILLED

## [s11] An endgame-lock AND-gate might have flipped since s10 (chassis drift, a new scanner tier, or a newly-indexed SOTN precedent).
- mechanism: Both gates are re-measurable in one command each, and the chassis has changed under this function before, so no gate verdict may be quoted from the ledger.
- probe: python3 tools/scan_hand_coded.py --single func_8001F938; grep docs/reference/sotn-construct-index.md for signed / signedness / dual-typed / same-address, plus a read of its detected-class table.
- result: Gate (a) FAIL: tier=LOW score=0/8 (107 insns), 'no strong hand-coded indicators', S1..S8 all clear - identical to the 2026-07-23 reading. Gate (b) FAIL: zero hits, and the index (1056 lines, sotn-decomp master aa53500226ee84be763f3e8702b27de06456b3a7, 1911 files scanned) has no signedness-split / dual-typed / same-address detector class at all, so it offers no citation; the F2 census of 2026-07-01 independently returned NOT ESTABLISHED. Both gates FAIL exactly as at s10.
- verdict: KILLED

## [s11] The 2026-08-25 23:20 narrowing ruling reopened a grindable path to distance 0.
- mechanism: That entry claimed the standing +0x270 pre-ban did not reach the single-s16-HImode-local spelling, which measures distance 0 and full-build-verifies.
- probe: The layer-1 cheat-reviewer's 23:29 re-review of the same body, plus the driver's banned-construct list at s11b dispatch.
- result: Closed. The reviewer held that a frozen family is owner-only to extend (.claude/rules/judge-sole-gate.md) and that a driver-side narrowing is not a legitimate authorization; the driver has since made BOTH the construct and the 23:20 entry mechanically banned for this function. No spelling remains: by the ban's own 'in ANY spelling' wording, every C form that causes cc1 to emit a second load of +0x270 sits inside the frozen family. The pure-C search space is not merely exhausted, it is fully characterised and gated on an owner ruling.
- verdict: KILLED

## [s12] The +0x270 fold is an open-port compiler-fork artifact — PsyQ's own cc1psx (GCC 2.7.2.SN.1), the compiler that actually built the target, would emit the second `lhu 0x270` load from the CLEAN floor-8 C.
- mechanism: s6/s7 attributed the missing second load to GCC 2.7.2 `combine` / `simplify_shift_const`
  folding `(x << 16) >> 15` to `x << 1` when `num_sign_bit_copies(op) == 16`. That gate lives in
  combine.c, and the SN Systems fork (cc1psx 2.7.2.SN.1) is a DIFFERENT build of 2.7.2 from
  decompals/mips-gcc-2.7.2 (which is kmc-tailored). If SN's combine did not take the fold, the
  target's two-load shape would be ordinary codegen for the clean C, the whole "signedness-split
  family" premise would be void, and the residual would be a compiler-fidelity/routing question
  rather than a C-form question. This axis had NEVER been probed on this function (zero occurrences
  of "cc1psx" anywhere in memory/grind/func_8001F938/ before this session), and it is the direct
  analogue of the axis the owner's ruling-10 return-to-active was discharged with on CD_datasync
  (docs/grind/decisions.md 2026-08-30 CD_datasync entry) and func_80072CD4.
- probe: dual-fork harness `tmp/grind/func_8001F938/s12/dualfork.sh` — ONE `mipsel-linux-gnu-cpp`
  pass over `src/code6cac.c` with the clean floor-8 body installed (exact Makefile CPP_FLAGS/CPP_DEFS),
  the resulting `code6cac.i` fed to BOTH `tools/gcc-2.7.2/build/cc1` (open port, exact CC_FLAGS
  incl. `-mel`) and `tools/cc1psx_wrapper.sh` (PsyQ cc1psx via dosemu2; calibration-only per
  `.claude/rules/no-compiler-divergence.md`, never a build path). func_8001F938's body extracted from
  each (`openport.fn.s`, `cc1psx.fn.s`), labels normalised, diffed.
- result: **KILLED, three ways.**
  (1) cc1psx TAKES THE SAME FOLD. Its +0x270 block is character-identical to the open port's:
      `lh $2,624($4)` / `move $3,$2` / `slt $2,$3,4` / `bne ... ; sll $2,$3,1` / `li $3,3 ; sll $2,$3,1`
      / `addu $2,$2,$4` — ONE load of 0x270 and the folded `sll 1`, exactly like the open port, and
      exactly unlike the target's `lh 0x270` + `lhu 0x270` + `sll 16 ; sra 15`.
  (2) WHOLE-FUNCTION AGREEMENT. After label normalisation the two forks' bodies are 192/192 lines
      and differ by exactly ONE line: a `li $2,0x00000011` scheduled two positions earlier in the
      open port's prologue. Same instruction count, same register assignment, same branch structure,
      same everything else. There is no fork-shaped search space here.
  (3) The open port is therefore VALIDATED as a faithful oracle for this function, which independently
      strengthens (not weakens) every s1-s11 measurement made on it.
  Consequence: the target's second load is NOT reachable from the clean C under EITHER 2.7.2 build,
  so it is a property of the ORIGINAL SOURCE's C form, not of our toolchain — which puts it right back
  inside the frozen signedness-split family that only the owner can move. Artifacts:
  tmp/grind/func_8001F938/s12/{dualfork.sh,openport.fn.s,cc1psx.fn.s,a.norm,b.norm}.
- verdict: KILLED

## [s12] An endgame-lock AND-gate flipped since s11.
- mechanism: both gates are one command each and must never be quoted from the ledger.
- probe: `python3 tools/scan_hand_coded.py --single func_8001F938`; grep of
  `docs/reference/sotn-construct-index.md` for signed / dual / same-address / sign_bit.
- result: gate (a) FAIL — `tier=LOW score=0/8 (107 insns)`, "no strong hand-coded indicators",
  S1..S8 every one clear (identical to s10/s11 and to the 2026-07-23 reading). gate (b) FAIL —
  ZERO hits in the construct index for any signedness / dual-typed / same-address class; the index
  still has no such detector class at all, so it cannot yield a file:line citation.
- verdict: KILLED

FRONTIER AFTER s12 — unchanged and still not a grind modality: an OWNER ruling on the frozen
signedness-split / redundant dual-typed-read family. Every grindable axis (structural, permuter,
forensics, rederive, solver, compiler-fork) is now measured dead with reasons banked. A future
session should open NONE of them.

## [s12] The missing second lhu of +0x270 is an open-port compiler-fork artifact: PsyQ's own cc1psx (GCC 2.7.2.SN.1), the compiler that actually built the target, would emit it from the CLEAN floor-8 C, voiding the whole signedness-split-family premise and converting the residual into a compiler-fidelity/routing question.
- mechanism: s6/s7 attributed the divergence to GCC 2.7.2 combine / simplify_shift_const folding (x<<16)>>15 to x<<1 when num_sign_bit_copies(shift operand) == 16. That gate lives in combine.c, and cc1psx is a DIFFERENT 2.7.2 build (SN Systems fork) from decompals/mips-gcc-2.7.2 (kmc-tailored). If SN's combine declined the fold, the target's two-load shape would be ordinary codegen for our clean C. The axis had never been probed here (zero occurrences of 'cc1psx' anywhere in memory/grind/func_8001F938/ before s12), and it is the same axis that discharged owner ruling 10 on the sibling items CD_datasync and func_80072CD4.
- probe: tmp/grind/func_8001F938/s12/dualfork.sh -- ONE mipsel-linux-gnu-cpp pass over src/code6cac.c (exact Makefile CPP_FLAGS/CPP_DEFS) with the clean floor-8 body installed, feeding the resulting code6cac.i to BOTH tools/gcc-2.7.2/build/cc1 (open port, exact CC_FLAGS incl. -mel) and tools/cc1psx_wrapper.sh (PsyQ cc1psx under dosemu2; calibration-only per .claude/rules/no-compiler-divergence.md, never a build path); func_8001F938's body extracted from each, labels normalised, diffed.
- result: KILLED three ways. (1) cc1psx TAKES THE SAME FOLD: its +0x270 block is character-identical to the open port's -- lh $2,624($4) / move $3,$2 / slt $2,$3,4 / bne .. ; sll $2,$3,1 / li $3,3 ; sll $2,$3,1 / addu $2,$2,$4 -- ONE load of 0x270 and the folded sll 1, exactly unlike the target's lh + lhu + sll 16 ; sra 15. (2) WHOLE-FUNCTION AGREEMENT: normalised, the two forks' bodies are 192 vs 192 lines and differ by exactly ONE line, a li $2,0x00000011 scheduled two positions earlier in the open port's prologue -- same instruction count, same registers, same branch structure. (3) The open port is therefore VALIDATED as a faithful oracle for this function, strengthening every s1-s11 measurement taken on it. Consequence: the target's second load is a property of the ORIGINAL SOURCE's C form, not of our toolchain, which places it back inside the frozen signedness-split family that only the owner can move.
- verdict: KILLED

## [s12] An endgame-lock AND-gate has flipped since s11 (chassis drift, new scanner tier, or a newly-indexed SOTN precedent).
- mechanism: Both gates are one command each and must never be quoted from the ledger; the chassis has changed under this function before.
- probe: python3 tools/scan_hand_coded.py --single func_8001F938 ; grep docs/reference/sotn-construct-index.md for signed / signedness / dual / same-address / sign_bit.
- result: Gate (a) FAIL: tier=LOW score=0/8 (107 insns), 'no strong hand-coded indicators', S1..S8 every one clear -- identical to the 2026-07-23, s10 and s11 readings; the STRONG-tier bar (S1/S2/S6) is not approached. Gate (b) FAIL: ZERO hits; the construct index still has no signedness / dual-typed / same-address detector class at all, so it cannot yield a file:line citation, and the 2026-07-01 F2 census over SOTN master independently returned NOT ESTABLISHED.
- verdict: KILLED

## 2026-09-01 — operator census-of-record note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

STAYS FORECLOSED — Ruling E executed: a first-hand five-stage sweep of sotn-decomp master (HEAD db41b28e, PSX-config membership verified per hit) found ZERO opposite-signedness same-address 16-bit read pairs; closest exhibits (servant/tt_002/faerie.c:1362 'needed for match' cast respelling; weapon/w_017.c:142 / w_032.c:16 union-view slot reuse) are all SAME-signedness. This supersedes the 2026-08-30 index census (which was a detector-coverage artifact — the index has no class for this shape) as the gate-(b) census of record. The F2 refusal stands; the banked distance-0 body (rejected/layer1-fail-0825-2329.c) stays rejected. Re-activation now requires an actual SOTN-master exhibit appearing in a future checkout or an owner family ruling.

## [s13] The 105-vs-107 instruction-count deficit is the missing 8-byte stack frame, not the second +0x270 load.
- mechanism: Normalised full-function diff of our build vs asm/funcs/func_8001F938.s. Target's `.L8001FA60` block emits 8 insns before the `addu` (lh, lhu, slti, bnez, sll16, addiu3, sll16, sra15); ours emits 8 (lh, nop, move, slti, bnez, sll1, li3, sll1). Count-neutral: the target's 2nd load occupies the load-delay slot maspsx fills with a nop, and its `sra $v0,$v0,15` is paid for by our `move $v1,$v0`. The only count difference in the whole function is `addiu $sp,$sp,-0x8` (delay slot of the first beq, asm/funcs/func_8001F938.s:11) + `addiu $sp,$sp,0x8` (epilogue, :117), which our build does not emit at all.
- probe: tmp/grind/func_8001F938/s13/cmp.py over tmp/grind/func_8001F938/s13/ours.txt (objdump of the sandbox .o) and the target .s.
- result: score 8 decomposes as 6 (block .L8001FA60 shape) + 2 (missing frame). This CORRECTS the H2/candidate.c/decisions.md attribution carried since s3, which recorded the count deficit as the second load and therefore never named the frame as a sub-residual. Chassis floor re-measured live: score 8, build_insns 105, target_insns 107, rules_dropped 0.
- verdict: CONFIRMED

## [s13] The target's 8-byte frame is a phantom slot produced by a signed `short` local assigned on more than one path and later used in a sign-extending context.
- mechanism: cc1's own `.frame ... # vars=` comment is get_frame_size(). Isolated micro-suite (tmp/grind/func_8001F938/s13/ft.c, ft2.c, ft3.c): `s16 x = load; if (x>=4) x=3; use x` => vars=8 (probes t2,u1,u3,u4,u5); the same without the conditional assignment (u2,t5) => vars=0; assigned from a computed SImode expression (v1) => vars=0; unsigned short with no sign-extending use (v4) => vars=0; `u16` kind-split HImode bitwise (t1) => vars=0. Sign-extension of a non-MEM HImode operand is expanded at tools/gcc-2.7.2/config/mips/mips.md:2340 (extendhisi2). The slot is never referenced: the .rtl/.cse/.flow dumps of the distance-0 body contain zero virtual-stack-vars refs — the [[phantom-frame-slots-gcc272]] artifact.
- probe: cpp+cc1 with the project's CC_FLAGS on the micro-suite; tmp/grind/func_8001F938/s13/frame.sh on the whole file.
- result: t2 (the banned clamp) reproduces BOTH the frame (vars=8) and the target's `lh`+`lhu` pair in eight instructions, in isolation, from one C statement.
- verdict: CONFIRMED

## [s13] Narrowing any OTHER local of the clean floor-8 body buys the target's 8-byte frame.
- mechanism: If some other local could be given a narrow type that GCC 2.7.2 gives a phantom frame slot, the 2-point frame sub-residual would be attackable without touching the +0x270 block at all.
- probe: six variants installed one at a time in src/code6cac.c and measured with tmp/grind/func_8001F938/s13/frame.sh — A_u16kind (u16 kind_full + u16 kind), B_s16val (s16 val), C_s16a2 (s16 a2), D_s16factor (s16 factor), E_s16idx (s16 idx), F_AB (A+B). Files in tmp/grind/func_8001F938/s13/variants/.
- result: all six measured `.frame $sp,0,$31 # vars= 0`. None buys the frame. Within this body the only value that is narrow AND assigned on more than one path AND later sign-extended is the clamped +0x270 value, so on this chassis the frame and the second load are one construct.
- verdict: KILLED
- kill_scope: instance
- measured_on: live chassis 2026-09-04, candidate.c clean floor-8 body (score 8, build_insns 105), zero FAKE constructs present, six single-declaration retypings measured via cc1 `.frame` (get_frame_size).

## [s13] The banked distance-0 body is still distance-0 after all chassis drift.
- mechanism: Re-measurement, not a new form. rejected/layer1-fail-0825-2329.c's function body installed verbatim.
- probe: sandbox func_8001F938 --disable all + frame.sh, 2026-09-04.
- result: score 0, target_insns 107, build_insns 107, rules_dropped 0, `.frame $sp,8,$31 # vars= 8`. One `short` declaration buys both the frame and the block. src/ restored to INCLUDE_ASM immediately; nothing submitted (the body is a mechanically banned construct).
- verdict: CONFIRMED

## s13 frontier
The structural axis produced a real result this session, but it is not a new form: it
is the discovery that the residual has TWO halves and that both halves are bought by
the same single `short` declaration, plus the measured elimination of every other
narrowing in the function. The next move is a `ruling-request` carrying the frame
evidence — see the s13 outcome JSON's ruling_question. Do NOT re-run the six
retypings; they are measured dead above.

## s13b (2026-09-04, structural) — CONFIRMED

- hypothesis: The body banked at `rejected/layer1-fail-0825-2329.c` (`s16 dmg =
  *((s16 *)(arg0 + 0x270)); if (dmg >= 4) dmg = 3; idx = dmg * 2;` plus the s2-PASSED
  kind-split) compiles to distance 0 on the CURRENT chassis and links to the oracle SHA1,
  and is submittable as ordinary C under the 2026-09-04 12:39 Judge clearance.
  - mechanism: one source-level dereference at one declared type; GCC 2.7.2 lowers the
    signed `short` local's sign-extending use via `extendhisi2`
    (`tools/gcc-2.7.2/config/mips/mips.md:2340`), which emits the target's second `lhu` and
    the `sll 16 ; sra 15`, and the multi-path HImode local buys the target's 8-byte phantom
    frame (`asm/funcs/func_8001F938.s:11`, `:117`) via `get_frame_size()`. Both target
    fingerprints therefore come from ONE declaration.
  - probe: install the body verbatim into `src/code6cac.c` via
    `tmp/grind/func_8001F938/s13b/install.py`; `sandbox func_8001F938 --disable all`;
    `verify-oracle`.
  - result: score 0, target_insns 107, build_insns 107, rules_dropped 0; verify-oracle
    `"ok": true, "build_matches": true`. CONFIRMED.
  - verdict: CONFIRMED

## s13c (2026-09-04, SYNTHESIS modality) — merged attack, kill re-audit, sibling sweep

### [s13c] The 8-byte phantom frame (2 of the 8 residual points) is buyable by narrowing some OTHER local of the clean floor-8 body — s13 tested only six of them.
- mechanism: s13 CONFIRMED that the target's frame comes from `get_frame_size()` reserving a
  slot for a signed `short` local, and killed six retypings (A_u16kind, B_s16val, C_s16a2,
  D_s16factor, E_s16idx, F_AB). It never touched the defaultpath/multpath locals
  (`sum_or_3`, `sum`, `vv0`, `vv1`, `f`, `raw_or_3`), which are exactly the values that are
  small, clamped and index-multiplied — the same shape as the +0x270 value. If any of them
  buys `vars= 8` the frame sub-residual decouples from the banned +0x270 construct and the
  floor drops 8 -> 6 without touching the frozen family at all.
- probe: nine new whole-function variants, each installed alone into src/code6cac.c and
  compiled with the project's cpp+cc1 (`tmp/grind/func_8001F938/s13c/screen.sh`, which reads
  cc1's own `.frame ... # vars=` comment):
  P0_mul2 (drop the artificial `(x<<16)>>15`), P1_s16sum_or_3 (`s16 sum_or_3` written on two
  paths by if/else, used as `sum_or_3 * 2`), P2_s16vv (`s16 vv0`, `s16 vv1` loaded from
  +0x26E/+0x272), P3_s16f (`s16 f` from +0x274), P4_s16sum (`s16 sum = vv0+vv1` then clamped
  in place), P5_s16raw_or_3 (`s16 raw_or_3` written on two paths from the SImode +0x270 probe),
  P6 (P0+P1), Q1_s16f_multipath (`s16 f` written on BOTH arms of an if/else, one arm the
  +0x274 memory load, semantics preserved via the 0x1000 identity multiplier),
  Q3_s16f_initload_rewrite (`s16 f = *(s16*)(a0+0x274); if (0x26C != 0) f = 0x1000;`),
  Q4_s16sum_initload (`s16 sum = *(s16*)(a0+0x26E); sum += *(s16*)(a0+0x272); if (sum>=4) sum=3;`).
- result: ALL of them measured `.frame $sp,0,$31 # vars= 0` — no frame, floor unchanged at 8.
  Control CTRL_banked (the distance-0 body from candidate.c) measured `vars= 8` in the same
  harness run, so the harness is live and discriminating.
- verdict: KILLED
- kill_scope: instance
- measured_on: live chassis 2026-09-04, clean floor-8 body (sandbox score 8, build_insns 105,
  target_insns 107, rules_dropped 0), zero FAKE constructs present; `vars=` read from cc1's
  `.frame` comment for each of the nine variants plus base and CTRL_banked.

### [s13c] The frame trigger is "a signed `short` local assigned on more than one path and later sign-extended" (the s13 statement of the mechanism).
- mechanism: s13 stated the gate that broadly. If it is right, several of the nine s13c
  variants (P1, P4, P5, Q1, Q3, Q4 all have exactly that shape) should have bought the frame.
- probe: same nine-variant screen, plus the sharper probe Q5_probe_only
  (`s16 sum = *(s16*)(arg0+0x26E); if (sum >= 4) sum = 3; idx = sum * 2;` — semantically WRONG,
  it drops the +0x272 term, and is therefore a mechanism probe only, never a candidate).
- result: the s13 statement is too broad and is hereby REFINED. P5 (short written on two paths
  from an SImode value) => vars= 0. Q4 (short initialised from a HImode MEM load but with an
  intervening `+=` before the clamp) => vars= 0. Q3/Q1 (short initialised from / assigned on
  both arms with a HImode MEM load arm, but consumed by `(a2*f)>>12` rather than by an index
  multiply) => vars= 0. Q5, which is the exact shape `short x = <HImode MEM load>; if (x>=4)
  x=3; <index> = x*2;` at a DIFFERENT address (+0x26E), => vars= 8. So the frame needs ALL of:
  (i) the short initialised DIRECTLY by a HImode memory load, (ii) re-stored with a constant on
  a second path with no intervening SImode arithmetic, (iii) consumed by the `*2` index
  expression that feeds the `addu` base+offset. The address itself is irrelevant.
- verdict: CONFIRMED

### [s13c] There is a SEMANTICALLY TRUTHFUL C form of the defaultpath (+0x26E/+0x272) block that has the frame-buying shape, so the 2-point frame can be bought away from +0x270.
- mechanism: Q5 proves the shape works at +0x26E. If the defaultpath block could be written so
  its clamped value is initialised directly by ONE HImode load, the frame would come from a
  site the ban does not reach.
- probe: read the target's own defaultpath block, asm/funcs/func_8001F938.s:97-108
  (`lh $v0,0x26E ; lh $v1,0x272 ; addu $v1,$v0,$v1 ; slti ; bnez ; sll $v0,$v1,1 ; li 3 ;
  sll $v0,$v1,1 ; addu`), and compare against the P4/Q4/Q5 codegen.
- result: KILLED. The block's clamped value is by construction the SUM of two loads, so
  condition (i) of the refined gate cannot hold without deleting one of the two loads —
  which is what Q5 does and why Q5 is semantically wrong (and why its own block loses the
  second `lh` and the `addu`, contradicting the target). Every truthful spelling of the sum
  (P4 `s16 sum = vv0+vv1`, Q4 `s16 sum = load; sum += load`) measured vars= 0 AND additionally
  corrupted the block (P4/Q4 diffs show `move`+`sll 16`+`sra 16`+`sll 16`/`sra 15` replacing
  the target's folded `sll 1` pair, and Q4 also flips both `lh` to `lhu`). On this chassis the
  only value in this function that satisfies the refined gate is the single load at +0x270 —
  i.e. the frame and the .L8001FA60 block remain ONE construct, as s13 concluded, now on
  nine more measurements and with the mechanism named exactly.
- verdict: KILLED
- kill_scope: instance
- measured_on: live chassis 2026-09-04, clean floor-8 body, zero FAKE constructs; nine variants
  + Q5 + CTRL_banked measured via cc1 `.frame` and per-variant asm diffs in
  tmp/grind/func_8001F938/s13c/*.s.

### [s13c] The clean floor-8 chassis still needs the artificial `((raw_or_3 << 16) >> 15)` shift pair it has carried since s1.
- mechanism: The shift pair was introduced in s1 as an attempt to defeat combine's
  simplify_shift_const fold. s6/s7 proved combine folds it to `sll 1` regardless. If it is
  codegen-inert it is a no-semantic-purpose construct sitting in the blessed reference form,
  and owner ruling 2026-08-31 Ruling 1(4) (.claude/rules/ordinary-c-judge-decidable.md:61)
  says the simplest byte-exact form lands.
- probe: P0_mul2 (`idx = raw_or_3 * 2;`) vs base; extracted `func_8001F938` bodies of
  tmp/grind/func_8001F938/s13c/base.s and P0_mul2.s diffed; sandbox re-measured live.
- result: KILLED (the shift pair is inert). The two asm bodies diff EMPTY — byte-identical
  codegen — and the sandbox with P0_mul2 installed measured score 8, target_insns 107,
  build_insns 105, rules_dropped 0. The clean chassis is therefore simplified and re-banked at
  memory/grind/func_8001F938/clean_floor8.c; `((raw_or_3 << 16) >> 15)` is retired from the
  reference form.
- verdict: KILLED
- kill_scope: instance
- measured_on: live chassis 2026-09-04, clean floor-8 body with `idx = raw_or_3 * 2`, zero FAKE
  constructs, sandbox score 8.

### [s13c] SIBLING SWEEP (mandated) — CD_datasync holds a transplantable spelling for a block this function shares.
- mechanism: The brief lists CD_datasync (src/system.c, foreclosed, floor 2 since its s50) as
  UNSPENT. Its s59 candidate.c header names its own residual and its levers.
- probe: read memory/grind/CD_datasync/candidate.c header (s59 update) and compare its residual
  and levers against ours.
- result: KILLED (no transplant exists). CD_datasync's entire 59-session residual is ONE
  scheduling-order fact — `sll $a0,$a0,2` emitted two slots too early in a
  `sll/addu/sll/lw` window (its candidate.c header item 1) — and its levers are luid-insertion
  via an address-carrier local (item 2, which it also disproved). func_8001F938 has no
  address-carrier pseudo, no shared block, no shared global, and a residual that is a COUNT
  deficit (105 vs 107) plus a HImode-fold shape, not an order flip; its own s55 sweep already
  recorded that the two functions share no block. Nothing to transplant in either direction.
- verdict: KILLED
- kill_scope: instance
- measured_on: read of memory/grind/CD_datasync/candidate.c (written 2026-09-04 12:21) against
  the live func_8001F938 chassis (score 8, build_insns 105).

### [s13c] The Judge PASS clearance of 2026-09-04 12:39 is usable by a future session as recorded.
- mechanism: The brief says the driver skips layer-1 for a body whose hash matches a recorded
  clearance, and that a body layer-1 FAILed goes straight to the Judge on resubmission.
- probe: `python3 tools/grinder/grindlib.py body-hash <root> func_8001F938 <path>` on all three
  banked copies of the distance-0 body.
- result: KILLED as recorded — a bookkeeping defect, not a code fact. candidate.c,
  rejected/layer1-fail-0825-2329.c and rejected/layer1-fail-0904-1246.c ALL hash
  `9f1177d269cd17e7` (the hash carrying the 12:46 layer-1 FAIL). The clearance in
  state.json judge_clearances is keyed `f56d218136d69273`, which matches NO artifact in the
  ledger, so the clearance can never fire and layer-1 will keep bouncing the body on the
  ground that "no reviewer has ever passed the body being submitted". Independently, the
  Judge's own PASS text (docs/grind/decisions.md:22276) ends "unban_construct clears the
  mechanical tripwire on the construct itself" — but state.json banned_constructs still
  carries entry #3, the verbatim clamp statement, so a candidate-ready re-declaring it is
  discarded by the driver before any review runs. Both facts together are why s13c returns
  `ruling-request` rather than `candidate-ready`.
- verdict: KILLED
- kill_scope: instance
- measured_on: state.json + grindlib body-hash, 2026-09-04, live chassis.

## s13c frontier (reset)
The pure-C search is not the bottleneck and has not been for six sessions: the distance-0
body exists, is banked, re-measures 0 on the current chassis, links to the oracle SHA1, and
has a Judge PASS ruling on its text. What blocks it is two mechanical records — a clearance
keyed to a hash no artifact carries, and a banned_constructs entry the same Judge ruling
directed be removed. Everything else this session touched is now measured dead:
nine narrowing variants, the sibling transplant, and the last no-semantic-purpose construct
in the clean chassis (removed, byte-neutral). A future session should NOT re-open structural /
permuter / forensics / rederive / solver / compiler-fork, and should NOT re-run any of the
fifteen retypings (s13's six + s13c's nine).
