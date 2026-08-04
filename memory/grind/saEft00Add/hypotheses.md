# Hypothesis ledger — saEft00Add

## s2 (2026-08-04, structural)

- H-s2-1: "The post--mel distance-1 is a codegen diff in saEft00Add closable by structural levers (the s1 frontier: stacked end-labels / duplicated-statement-into-arms / permuter)." — KILLED. Probe: masked per-insn diff of sandbox .o vs build/src/system.o + strip-diff + per-function size accounting + --keep-cheat-asm control (score 0). The 1 is the %lo(D_80082050) local-symbol addend shifted 0x9A0 by sibling cheat-asm stripping (func_80080828 −1408 B, tslTm2LoadImage −1044 B, marionation_Exec −12 B). saEft00Add's own instructions are 133/133 identical. The whole s1 frontier (trilemma-era) is moot: the committed body already matches; do not re-open it.

- H-s2-2: "Some saEft00Add-local C respelling can zero the sandbox score anyway." — KILLED by construction (no measurement needed): the addend is the static callback's .text offset, determined entirely by code BEFORE saEft00Add in the TU; the only dodges (de-static D_80082050 / drop the reference) contradict the bit-exact Sony provenance and are cheats-by-spelling.

- H-s2-3 (OPEN, not mine to execute): once func_80080828 + tslTm2LoadImage + marionation_Exec are pure C (their own queue items), sandbox saEft00Add --disable all reads 0 with the current committed body unchanged. Alternatively an owner-approved engine metric fix (mask local addends / scope stripping to the scored function) zeroes it immediately. Filed as ruling-request in the s2 outcome.
