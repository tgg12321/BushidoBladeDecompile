# Hypothesis ledger — func_80044800

## s1 (2026-09-06, recon)

H1 CONFIRMED — the per-word globals are the `Unk800A9CF8Header` aggregate
(include/game.h). Spelled as struct members, the address pattern of the target
(`$s5 = &D_800A9CF8 + 2` formed from the `.unkC` address via cse.c
related_value; lui-form reloads for `.unk6` / `.unk8` where the target has
them) reproduces without naming any per-word symbol. Measured 0.

H2 KILLED (instance) — `rec` as a pointer biv (`rec++` in the latch, never
reassigned in the body) with a separate `rec2` local for the stage-0x12 block:
loop.c strength reduction spawns a `rec+96` giv, every field access becomes a
negative offset from it, $fp is consumed and `&Judge` stops hoisting. Measured
98 on v1 (chassis ff5e4ece, no FAKE constructs). Instance kill: this form on
this chassis; see rejected/biv-rec-separate-rec2-inline-products-score98.c.

H3 CONFIRMED — retargeting `rec` to the paired char-data record inside the
stage-0x12 block and restoring it afterwards (`ent = rec; rec = ... + i;
rec = ent;`) makes `rec` multi-set in the loop → not a biv → no giv, and
produces the target's `move s0,s2` / `move s2,s0` pair. 98 → 12 together with H4.

H4 CONFIRMED — the four rotation products in fresh s32 temps computed before
any store through `rec` (cos looked up first, then sin) reproduce the target's
mult/mflo interleave.

H5 CONFIRMED — `last = D_800A9CF8.unk2 - 1` (fresh local) is the only one of
six index spellings tested that survives fold's constant reassociation; with
latch order `rec++, i++` the function measures 0 (candidate.c).

Frontier: empty — matched.
