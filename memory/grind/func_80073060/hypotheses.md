# Hypothesis ledger — func_80073060

## s1 (2026-09-08, recon)
- H1 KILLED (instance): one shared `j` x-variable across all three loops with a separate `i` counter (v1) — measured 23 on HEAD chassis, no FAKE. Cause: single pseudo for x forces one register (s3) for all loops and leaves `i` in s4; target has distinct x pseudos per loop and `i` sharing loop-1's counter register.
- H2 CONFIRMED: loop-3 `0x3F` stays in-loop because loop.c's move threshold (29, -3 per hoist) is crossed by a 4th movable + 2 extra insns — produced naturally by `p->x0 = 0x211 - i * 0x20` (giv). Measured: v2 sandbox 0.
- H3 CONFIRMED: shared counter `i` for all loops (loop 1 uses `i` directly as x) gives the s3 sharing and keeps `i = 0` out of the loop-1 branch delay slot. Measured: v2 sandbox 0.
Frontier: none — matched.
