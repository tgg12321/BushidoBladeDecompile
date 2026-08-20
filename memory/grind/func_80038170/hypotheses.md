# Hypothesis ledger — func_80038170

## s1 (2026-07-28, recon)
- H1 CONFIRMED: phantom +8 frame closable in pure C via same-base pair inside the conditional (phantom-frame-slots-gcc272). Probe: cc1 .frame bisect matrix. Result: vars 8->16 == target; floor 14->5; reviewer PASS.
- H1a KILLED: named u16 (HImode) temp in halfword copy loop triggers the slot — vars stays 8.
- H1b KILLED: named-pointer staging of the pair (u8 *t = &D_8008F19C + s3*2) — kills the temp, vars 8.
- H2 CONFIRMED-then-review-FAILED: chained `s1=s2=s3=0;` flips init emission to s3,s2,s1 -> floor 1 (text-artifact-only). Cheat-reviewer FAIL (order-flip family). Banked rejected/chained-zeroing-order.c. -> ruling-request filed s1.
- H3 CONFIRMED: floor-1 residual is symbol-vs-addend text artifact (D_8008F19C+1 vs D_8008F19D); linked bytes identical (word-diff proof). Sandbox 0 unreachable pre-integration by construction.

## s2 (2026-07-28, recon — post-Judge-ruling)
- H4 KILLED: Judge-sanctioned separate-statement spelling alone reaches byte parity. Probe: raw .o word diff vs build/ reference (oracle bytes). Result: save order correct, but i-init `move a3,zero` scheduled late + `sw ra` displaced — 5 words off target; engine masked score (1) conceals it.
- H5 CONFIRMED: standalone `i = 0;` before the mask load (empty for-init) restores the target schedule — sched1 emission order tracks source statement order for the independent init cluster. Probe: raw word diff. Result: 141/141 words match; only the linker-identical reloc spelling remains. Layer-1 reviewer PASS.
- H6 CONFIRMED: remaining carriers (regfix.txt:1250 + prologue_config entry) now MANGLE the correct output — rules-applied sandbox 4 vs clean 1. Must be retired at integration before any rebuild.

## [s1] The Judge-sanctioned separate-statement spelling (s3=0; s2=0; s1=0; decl order unchanged) alone reaches byte parity
- mechanism: prologue init/save pair emission order + sched1 scheduling of the pre-loop init cluster
- probe: sandbox --disable all + raw .o word diff vs build/ reference (oracle bytes), tmp/grind/func_80038170/s1/judge_form_diffcheck.sh
- result: Save/init pair order correct (s0,s3,s2,s1,ra), BUT i-init `move a3,zero` schedules after the li/lui/lw mask cluster and sw ra displaces to slot 12 — 5 words off target; the engine masked metric conceals this (still prints 1)
- verdict: KILLED

## [s1] Standalone `i = 0;` before `mask = D_80106A50;` (empty for-init) restores the target schedule
- mechanism: GCC 2.7.2 sched1 emission order tracks source statement order for the independent pre-loop init cluster; for-init clause places the i-init RTL after the mask load, standalone statement places it before
- probe: edit src, sandbox --disable all, raw word diff vs build/ reference
- result: 141/141 insn words match the oracle stream; sole remaining .o-text diff is the linker-identical reloc spelling D_8008F19C+1 vs D_8008F19D (hi 0x8009 / lo 0xF19D both ways, re-proven vs asm/funcs words); layer-1 cheat-reviewer PASS (ordinary live-statement order, store-before-jal family)
- verdict: CONFIRMED

## [s1] The two remaining cheat carriers now actively mangle the correct natural output
- mechanism: regfix.txt:1250 reorder @9-13 permutes the already-correct natural prologue window
- probe: sandbox func_80038170 with rules applied (no --disable)
- result: score 4 with rules applied vs 1 clean — carriers are harmful; a full build with them active would break the oracle, so build/ regeneration (and hence sandbox 0) is impossible until they are retired on the driver surface
- verdict: CONFIRMED

## [s2] The banked Judge-sanctioned candidate (separate s3=0;s2=0;s1=0; + standalone i=0 hoist + one-table (&D_8008F19C)[s3*2+0/1] pair), re-applied verbatim to src/code6cac_c_mid.c, reproduces the s1 measurements exactly
- mechanism: src at session start still carried the OLD cheat form (register pins a3/a1, dummy0/dummy1 __asm__ m-constraint frame coercion, two-symbol D_8008F19C/D_8008F19D spelling, for-init i=0) — the prior session's src edits did not persist to HEAD; the ledger's integration-readiness claim was unverifiable against the tree until re-applied
- probe: Edit src to the banked candidate body; sandbox func_80038170 --disable all; word-level diff vs asm/funcs oracle words (tmp/grind/func_80038170/s1/word_diff.py); address-normalized 141-insn diff vs build/src reference .o (tmp/grind/func_80038170/s2/norm_diff.py)
- result: sandbox score 1 (141/141 insns, rules_dropped 1, cheat_asm_stripped 29 file-wide); word diff vs oracle stream: ONLY the pre-link jal func_80079194 reloc placeholder (0C000000 vs resolved 0C01E465); normalized diff vs stale build/ reference: 6 insns = 2 reloc-spelling words (lui/lbu D_8008F19C+1 vs D_8008F19D, proven linker-identical, both resolve to 3C018009/9022F19D) + 4 R_MIPS_26 j-words that differ only because the sandbox .o strips 29 cheat-asm instances file-wide shifting every function's section offset by 4 bytes (stale-reference artifact, vanishes on rebuild)
- verdict: CONFIRMED

## [s3] src at HEAD reverted to the old cheat form again; re-applying the Judge-sanctioned candidate verbatim reproduces the s2 floor exactly
- mechanism: grind-session src edits are reverted by the driver between sessions; the banked candidate.c is the only durable carrier of the form
- probe: Edit src/code6cac_c_mid.c to the banked candidate body; sandbox func_80038170 --disable all
- result: score 1, target_insns 141, build_insns 141, rules_dropped 1, cheat_asm_stripped 29 — byte-identical to the s2 measurement (artifact s3/sandbox_judge_form.txt, re-confirmed after probe revert in s3/sandbox_judge_form_final.txt)
- verdict: CONFIRMED

## [s3] Rebasing the one-symbol pair on D_8008F19D ((&D_8008F19D)[s3*2-1] / [s3*2]) — the only untested one-symbol spelling class — can zero the pre-link reloc diff and reach sandbox 0 pre-integration
- mechanism: the phantom-frame temp needs a shared-index pair off ONE symbol (s1 trigger matrix); the only two one-symbol bases covering bytes 0x8008F19C..D are D_8008F19C (+0/+1 addends) and D_8008F19D (-1/+0 addends); if the -1 addend happened to be masked differently the diff could vanish
- probe: temporary src edit to the D_8008F19D-rebased pair; sandbox func_80038170 --disable all
- result: score 1 at 141/141 — identical floor; the nonzero-addend artifact just moves from out[0x43] (+1) to out[0x42] (-1). Any one-symbol pair carries a nonzero addend on one access while the stale build/ reference has addend 0 on both; two-symbol spellings kill the frame temp (s1). Therefore NO source spelling reaches sandbox 0 before build/ regenerates. Banked rejected/d8008f19d-rebased-pair.c
- verdict: KILLED

## s4 (2026-08-19, permuter modality — pre-empted by chassis re-measure)

## [s4] The s3 "sandbox 0 unreachable by construction" deadlock is chassis-relative and has been dissolved by the asm-until-matched migration
- mechanism: the deadlock had two legs — (i) the build/ reference .o was the OLD cheat-form object, so our correct source differed from it by the linker-identical D_8008F19C+1 vs D_8008F19D reloc spelling, and (ii) regfix.txt:1250's `reorder @9-13` would MANGLE the now-correct natural prologue on any real build, so build/ could never be regenerated from the correct source without first retiring a rule no grind session may touch. Commit 4faaa384 retired that rule as part of the asm-until-matched migration and moved the function to INCLUDE_ASM, removing leg (ii) entirely; splicing the banked body over the INCLUDE_ASM line then makes leg (i) self-clearing on the next build.
- probe: splice the banked candidate body over `INCLUDE_ASM("asm/funcs", func_80038170);` in src/code6cac_c_mid.c; `sandbox func_80038170 --disable all`; `engine build`; re-run the sandbox
- result: pre-build sandbox 1 (the reloc-spelling artifact only, rules_dropped 0 — confirming the regfix carrier is gone); `engine build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; post-build sandbox 0 at 141/141
- verdict: CONFIRMED

## [s4] The banked s3 candidate compiles and links as-is on the current chassis
- mechanism: n/a — assumed inheritance
- probe: `engine build` with the banked body verbatim
- result: KILLED — link failure `undefined reference to func_80079194`. The symbol does not exist; 0x80079194 is `strcpy` (asm/funcs/strcpy.s). Corrected to `strcpy(out + 4, D_8008F1C0)` using the prototype already at src/code6cac_c_mid.c:279. Also corrected two `&`-of-array index spellings to plain array indexing (`D_8008F204[i]`, `D_8008F1A8[...]`) to match their `extern u8 X[];` declarations in include/code6cac.h. All three edits are codegen-neutral; the oracle match is unaffected.
- verdict: KILLED

## [s4] The surviving tools/prologue_config.json func_80038170 entry is load-bearing for the match
- mechanism: prologue_fix replaces the emitted prologue window with a hardcoded instruction list; if the natural cc1 prologue differed, the entry would be silently carrying the match
- probe: `sandbox func_80038170 --disable all` (which STRIPS prologue_fix) against a build/ reference produced WITH prologue_fix active
- result: KILLED — score 0 with prologue_fix stripped. The natural prologue is textually identical to the hardcoded list, so the entry is a measured NO-OP and pure dead weight for `retire func_80038170` to delete.
- verdict: KILLED
