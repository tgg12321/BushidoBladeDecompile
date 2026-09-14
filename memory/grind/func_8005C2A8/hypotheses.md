# Hypothesis ledger — func_8005C2A8

## s1 (recon, 2026-09-14)

H1 — declaration fix / object model (mandated first hypothesis, three flagged
symbols). CONFIRMED as a no-op: D_800EFC38, D_800EFB38 and D_800158CC all
already have correct, precedented declarations available in this TU
(src/text1b.c:2645-2646) or in src/text1a_b_pre_rodata.c:359. Adopting them
verbatim was sufficient; no declaration change was required and no symbol
remained a MISMATCH after the match. Full per-symbol verdicts in the
`OBJECT MODEL:` entry in evidence.md.

H2 — the callee's declared return type, not the local's type, decides where
GCC 2.7.2 places the narrowing sign-extension of a call result assigned to a
`short` local. CONFIRMED by measurement: flipping the local prototype of
snd_VabOpen from `s16` to `s32` moved the extension from a per-use scratch
($v1) into the local's own pseudo ($s0) and dropped the score 16 -> 13.

H3 — the physical order of the two exit blocks follows the SOURCE order of the
arms of the final `id != -1` test. CONFIRMED by measurement: putting the long
success tail in the THEN-arm and letting the printf/return-0 arm trail at the
end of the body dropped the score 13 -> 0 (it removes a redundant `j` to the
shared epilogue and flips the branch sense to the target's `bne`).

STATUS: FUNCTION IS MATCHED. `sandbox func_8005C2A8 --disable all` = 0 and
`verify-oracle` reports build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa
(== oracle) with the C body in src/text1b.c. Pure C, zero cheat constructs, no
sanctioned-family exception claimed. No open frontier remains.
