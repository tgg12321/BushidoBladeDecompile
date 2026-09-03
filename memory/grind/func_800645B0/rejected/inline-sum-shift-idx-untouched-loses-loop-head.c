/* REJECTED -- s18 (structural).  `idx2 = idx << 1;` kept, the three word stores indexing on
 * the INLINE expression `((idx2 + idx) << 2)`, `idx` never overwritten.
 * **3 / 78, 78 insns** with the WD residual verbatim (11/12/65) and index 20
 * exact.  Same mechanism as a2: with no write-back, pseudo 74 keeps
 * reg_n_sets == 1 and sched.c:2526 lifts the loop-top addu.  Proves the
 * loop-head half is decided purely by the WRITE COUNT on `idx`, not by whether
 * the offset arithmetic is spelled as a statement or as a subexpression.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}