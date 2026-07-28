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
