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

## s1 re-run (2026-09-06, recon, post layer-1 FAIL)

H6 CONFIRMED -- candidate.c still measures 0 on chassis 7d3c722f (canonical C,
sandbox --disable all 0, 202/202). The floor is 0; the open question is
review classification, not codegen.

H7 KILLED (instance) -- the layer-1 reviewer's proposed spelling: a distinct
local `ent` for the char-data record with `rec` never reassigned. Measured 70
on chassis 7d3c722f, no FAKE constructs (rejected/distinct-ent-local-rec-biv-
giv-score70.c). rec becomes a loop.c biv -> giv rec+96 -> $fp consumed; and the
block's stores land in $s0 while the target's go through $s2 with rec parked
in $s0 (asm lines 118/121/166).

H8 KILLED (instance) -- indexed `rec = table + i` per iteration with a distinct
`ent`. Measured 48 on chassis 7d3c722f, no FAKE constructs (rejected/indexed-
rec-distinct-ent-score48.c). rec becomes a giv of i, recomputed each
iteration from a reloaded `.unkC`; no latch biv.

Frontier: the matched body (candidate.c) is byte-proven and the retarget/
restore block is byte-evidenced as the original's pointer flow (evidence.md,
BYTE EVIDENCE). Both bans cover exactly this block, so `candidate-ready` is
mechanically impossible until a Judge ruling clears the candidate.c body.
This session returns `ruling-request` with the precise question. If the Judge
rules FAIL, the remaining sanctioned route is the "Variable reuse for codegen
control" family with a FAKE annotation on `ent = rec` / `rec = ent` -- note the
annotation would be on the original's own spelling and the F-family bounds
(staged-value-reused-variable bound 3: previous value dead) do NOT fit (rec's
value is live and restored), so that route needs its own ruling too.

## s1 (2026-09-06, recon, post Judge PASS)

H9 CONFIRMED -- the Judge-cleared candidate.c body (hash 52ecd6d8e091f34b)
measures 0 on chassis 4287b19d (canonical C, sandbox --disable all 0,
202/202). Submitted EXACTLY as cleared; candidate-ready.

Frontier: empty -- matched and cleared. If the driver's bytes verification
fails, the only open item is the Rec4473C typedef rename (byte-neutral per the
s1 oracle rebuild) -- re-run verify-oracle --rebuild before touching the body.
