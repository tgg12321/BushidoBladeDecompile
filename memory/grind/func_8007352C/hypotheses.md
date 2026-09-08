# Hypothesis ledger — func_8007352C

## s1 (recon, 2026-09-08)
- H1 CONFIRMED — `andi $fp,$v0,0xFFFF` after GetClut is the zero-extension of a u_short return into an s32 local: declare `extern u16 GetClut(s32,s32)` + `s32 clut`. Measured: score 3 (u16 local + u32 proto = plain move) -> 0 together with H2.
- H2 CONFIRMED — func_8003D52C("warning\n") is a ONE-argument call; the two a1 moves are AddPrim's argument duplicated by reorg (steal from target thread + relax_delay_slots redirect). Two-arg call measured +1 insn (score 3); one-arg call measured 0.
- H3 CONFIRMED — ordinary `for (i = count-1; i >= 0; i--)` with `s16 i`, named clip bounds x1/y1 computed before the &&-chain, `sp++` after AddPrim: everything else in the body matched at first measurement (the score-3 residual was entirely H1+H2).
- Frontier: none — sandbox 0 this session, candidate-ready.

## s1 re-run after layer-1 FAIL (recon, 2026-09-08)
- H4 CONFIRMED — the honest one-argument `SetSprt((s32)sp)` under `extern void SetSprt(s32)` is byte-identical to the banned two-arg form: sandbox --disable all = 0 (127/127). The `lh $a1` before the call is scratch allocation for the x0 operand, not an argument. Banned construct removed; no FAKE constructs anywhere in the body.
- H5 CONFIRMED — GetClut `u16` prototype change at src/text1b.c:409 is neutral for func_800485EC (68/68, score 0).
- Frontier: none — candidate-ready with a body that is ordinary C throughout.

## s1 third pass (recon, 2026-09-08) — after driver discard on self-vet tripwire
- H6 CONFIRMED — candidate.c re-applied to the restored stub reproduces sandbox --disable all = 0 (127/127); the body needs no change. The discard was a self-vet wording problem (CONSTRUCTS line echoed ban vocabulary), fixed by rewording; grindlib selfvet exits 0.
- Frontier: none — candidate-ready.
