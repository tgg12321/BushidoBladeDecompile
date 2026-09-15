# Hypothesis ledger — func_8005C074

## s1 / recon — 2026-09-15
- H1 (CONFIRMED, score 0): body decoded from asm — selection-sort into order[] with u16 mask / u32 min, empty loop carrying the invariant addr assignment, walk loop with nested move loop + `return 0`, separate loop variables i/j/k. Candidate in candidate.c, applied to src/text1b.c.
- H2 (KILLED, instance): one shared s16 loop variable for all three top-level loops — measured 16 (a0/a1 seat swap from the count>0 guard onward) on the s1 chassis with no FAKE constructs present.
- H3 (KILLED, instance): `addr = ...;` written before the empty loop instead of inside it — measured 43 / 142 insns on the s1 chassis with no FAKE constructs present.
