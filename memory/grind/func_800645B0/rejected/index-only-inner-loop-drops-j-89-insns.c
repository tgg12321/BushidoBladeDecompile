/* REJECTED — func_800645B0, s14. Companion of
 * induction-variable-index-costs-five-insns.c (variant IV2): the inner loop
 * iterates the slot index directly and `j` is deleted entirely.
 * Measured 25 / 78 at 89 build insns — the computed `i + 4` bound and the
 * lost shared `i + j` addu cost eleven instructions. See that file for the
 * full mechanism and the target-stream evidence. */
s32 func_800645B0(void) {
    s32 i;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {

        for (idx = i; idx < i + 4; idx++) {
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
