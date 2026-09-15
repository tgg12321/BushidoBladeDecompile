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

## s2 (recon, 2026-09-15)

H1 — declaration fix / object model (mandated first hypothesis; three flagged
symbols). CONFIRMED again as a no-op on the s2 chassis: D_800EFC38, D_800EFB38
and D_800158CC all already have correct precedented declarations in this TU
(src/text1b.c:2645-2646) or in src/text1a_b_pre_rodata.c:359. Per-symbol verdicts
in the `OBJECT MODEL:` entry in evidence.md.

H2 — the `vab` alias local (banned after s1) is load-bearing for the three
sll/sra of the promoted short parameter. KILLED. Probe: delete the local and its
assignment, read the parameter at all four former use sites, measure. Result:
score 0 — the alias bought nothing. kill_scope: instance (V2 body, s2 chassis,
no FAKE constructs present, measured 0).

H3 — the s32 return type that the s1 prototype claimed for the in-TU VAB-open
wrapper is a FABRICATION, i.e. it contradicts what the callee's own bytes say.
KILLED. Probe: read asm/funcs/snd_VabOpen.s and the definition at
src/text1b.c:2707; the return-value sll/sra at 8005C5F4 is emitted by the
explicit `(s16)` cast written in the callee's C body, so the return type is
unconstrained by that function's bytes. Changed the DEFINITION to s32 (cast kept)
and measured the callee: score 0, unchanged. Prototype and definition now agree
at s32 and func_8005C2A8 measures 0. kill_scope: instance (definition + prototype
both s32, s2 chassis, no FAKE constructs, callee measured 0 and target measured
0).

STATUS: matched again, this time with BOTH banned constructs absent rather than
respelled — the alias is deleted outright and the prototype is made true by
editing the definition it used to contradict. `sandbox func_8005C2A8
--disable all` = 0 (134/134) and `verify-oracle` build_sha1 == oracle. Self-vet
at memory/grind/func_8005C2A8/self_vet.md claims no sanctioned family and carries
no FAKE annotation. No open frontier.
