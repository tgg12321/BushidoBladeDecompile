# Hypothesis ledger — func_800430E4

## s1 (2026-09-08, recon)
- H1 CONFIRMED: the four scratchpad RMWs (0x1F8003A6/A8/AA s16, 0x1F8003B8 s32) were written relative to the block pointer `dst` (0x1F8003A0), not as bare absolute integer addresses. Mechanism: expand's memory_address() force_reg's a bare constant address into a fresh pseudo per access; cse merges the read/write pair so the pseudo has two uses and combine cannot fold it back — whereas `(plus reg const)` with reg≡const is folded to a constant address by cse fold_rtx inside find_best_addr (cse.c:2622) with no pseudo. Measured: retired body 52 → dst-relative form 0.
- H0 (DATA MODEL declaration fix) NOT NEEDED: all five flagged globals match with their TU-local externs (see the OBJECT MODEL entry in evidence.md).
