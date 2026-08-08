/* REJECTED (session 2, probe P5): merging a0_58 and v1_58 into ONE reused
 * variable p58 loaded twice from s0+0x58 (the ledger's H2 next-probe).
 * Measured: 31 / 160 insns (baseline was 20/158) - WORSE, and a0_58 stayed $5.
 * KILLED both empirically and analytically: in target the two loads' values
 * are SIMULTANEOUSLY live (lw $a0 @57 deref'd @69; lw $v1 @62 deref'd @72),
 * so they need two registers and cannot be one GCC 2.7.2 pseudo (no live-range
 * splitting).  Any single-variable spelling either loses a load to CSE or
 * emits both loads into the same register.  Do not re-propose.
 * (Fragment - the relevant join-block shape only:) */
void p58_merged_fragment(u8 *s0, s32 arg2) {
    s32 v0_50 = *((s32 *) (s0 + 0x50));
    u16 old_kind = *((u16 *) (s0 + 0x6A));
    s32 p58 = *((s32 *) (s0 + 0x58));
    /* ... sb 0x60/0x61, a1_val, sh 0x6C ... */
    *((s16 *) (s0 + 0x6A)) = *((u8 *) p58);
    p58 = *((s32 *) (s0 + 0x58));
    *((s16 *) (s0 + 0x6E)) = *((u8 *) (p58 + 2));
}
