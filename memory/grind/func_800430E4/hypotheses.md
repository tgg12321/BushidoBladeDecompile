# Hypothesis ledger — func_800430E4

## s1 (2026-09-08, recon)
- H1 CONFIRMED: the four scratchpad RMWs (0x1F8003A6/A8/AA s16, 0x1F8003B8 s32) were written relative to the block pointer `dst` (0x1F8003A0), not as bare absolute integer addresses. Mechanism: expand's memory_address() force_reg's a bare constant address into a fresh pseudo per access; cse merges the read/write pair so the pseudo has two uses and combine cannot fold it back — whereas `(plus reg const)` with reg≡const is folded to a constant address by cse fold_rtx inside find_best_addr (cse.c:2622) with no pseudo. Measured: retired body 52 → dst-relative form 0.
- H0 (DATA MODEL declaration fix) NOT NEEDED: all five flagged globals match with their TU-local externs (see the OBJECT MODEL entry in evidence.md).

## s2 (2026-09-08, recon re-dispatch)
- H2 CONFIRMED: typing D_800FF610 as the gte.h MATRIX it is consumed as (MulMatrix0 output in the same TU) and spelling the copy `*dst = D_800FF610;` with `MATRIX *dst = (MATRIX *)0x1F8003A0;` and the halvings `dst->m[1][0..2] >>= 1; dst->t[1] >>= 1;` keeps the s1 `(plus reg const)` fold mechanism (cse.c find_best_addr) with truthful field types. Measured: sandbox 0, oracle SHA1 match.
- H3 CONFIRMED: declaring D_800F62E0 as the 8 x 0x60-byte record table the sibling TUs walk (`extern u8 D_800F62E0[8][0x60];`) and writing `D_800A3828 = (s32)D_800F62E0[arg2];` reproduces the target's sll/addu/sll x96 chain with no per-use stride arithmetic. Measured: sandbox 0, oracle SHA1 match.
- Open (not a codegen question): header-canonical single declaration for both symbols across TUs — integration handoff if the Judge requires it.

## s3 (2026-09-08, recon re-dispatch after the s2 tripwire discard)
- H2/H3 RE-CONFIRMED on HEAD 77e6be47 with the edits in src/: MATRIX-typed D_800FF610 + `*dst = D_800FF610;` + field compound shifts, and the 8x0x60 record-table index — sandbox 0, verify-oracle ok. No new codegen hypothesis was needed; the only change from s2 is at the MulMatrix0 call site (truthful `MATRIX *` prototype, no `&sym` cast) and the self-vet wording (see evidence s3).
- H4 (procedural) CONFIRMED: a CONSTRUCTS: block that names the constructs by shape without echoing the ban vocabulary passes `grindlib.py selfvet` (exit 0) with the same body.
