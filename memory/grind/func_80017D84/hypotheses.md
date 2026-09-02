# Hypothesis ledger — func_80017D84

## Session 1 (recon, 2026-09-02)
- H-s1-1 CONFIRMED: the 8-word v1/a0/a1/a2 load-store ladder at +0x14 is GCC 2.7.2's inline block
  move for a 32-byte struct assignment; writing it as a struct copy reproduces the ladder AND
  forces the `move t0,a0` param copy + a3 slot pointer with no register coercion (E-s1-1, E-s1-3).
- H-s1-2 KILLED (instance): `(MATRIX *)` cast with gte.h not included in ings.c — cc1 -w silently
  drops the statement (E-s1-2). Measured on the session-1 chassis, no FAKE constructs.
- H-s1-3 CONFIRMED: tail scheduling is a pure statement-order effect; order (w8, c, h6, wC, w10) is
  the unique zero-mismatch order of the 24 legal ones (E-s1-3). Floor 0; verify-oracle ok:true.

Frontier: none — matched. Optional refinement only: type the 32-byte block with its real
struct once func_80017A44's ledger establishes what the +0x14 region is (8 s32 vs MATRIX);
must stay byte-neutral (any 32-byte struct assignment emits the same block move).
