# Hypothesis ledger — func_8003F824

## s1 (2026-09-08, recon)
- H1 CONFIRMED: `sc->quads[sc->count] = rec->quad;` (16-byte struct assignment) produces the target's single address compute + lw a0..a3 / sw block move; per-word copies re-derive the address each time (+12 insns). Measured 70 -> 29 together with H2.
- H2 CONFIRMED: `while (*cmds++ != -2) {}` reproduces the target's skip loop (pre-test on the hoisted -2 reg + inner loop with a fresh -2, no addiu -2/+2 compensation); `while (*cmds != -2) cmds++; cmds++;` leaves reorg's peel compensation pair.
- H3 KILLED (instance): preloading `c` before the loop and reloading it at the body end (s16 or s32, u16-cast or not) did not give the target's separate lh/lhu pair on this body — cse merges the two `(mem:HI (reg cmds))` loads in the same block (measured 29 with s32; draft 1 context with s16). measured_on: draft 2/3 chassis (struct copy + post-inc skip loop), no FAKE constructs.
- H4 KILLED (instance): `(c = *cmds) != -3` in the for-condition makes c the compare operand; cse_set_around_loop hoists the top's sign-extend shift into the loop-test reg (score 31). measured_on: draft 3 chassis, no FAKE constructs.
- H5 CONFIRMED: `s16 c = *cmds;` as the FIRST statement of the loop body with `for (i = 0; *cmds != -3; i++)` yields distance 0 (cse_set_around_loop copy + combine PARALLEL split; see evidence.md). FUNCTION MATCHED; oracle SHA1 confirmed.
