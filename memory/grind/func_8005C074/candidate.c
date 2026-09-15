/* func_8005C074 (text1b.c) - SPU VAB compaction: sorts the resident VAB slots
 * 1..15 by SPU address (selection order into order[]), then walks them from the
 * end of slot 0; the first slot that is not already contiguous, and every slot
 * after it, is moved down with func_8005BF78. Ordinary C: no FAKE, no volatile,
 * no asm, no pin, no dead store, no pad, no alias local. `vabid` is passed by
 * the caller (func_8005C2A8) but the target never reads it.
 * Grinder s1/recon 2026-09-15: sandbox --disable all = 0.
 * The loop-invariant `addr` assignment inside the first (otherwise empty) loop
 * is what the bytes say: the target computes addr in that loop's preheader,
 * AFTER its `count > 0` guard. Assigning addr before the loop instead measures
 * 43 / 142 insns (banked under rejected/).
 */
s32 func_8005C074(s16 vabid, s32 base) {
    s16 order[16];
    s16 count;
    u16 mask;
    u32 min;
    s16 minidx;
    s16 i;
    s16 j;
    s16 k;
    s32 addr;

    count = 0;
    mask = 0;
    for (;;) {
        min = 0x7FFFF;
        minidx = -1;
        for (i = 1; i < 16; i++) {
            if (!((mask >> i) & 1) && D_800EFB38[i] != 0 && D_800EFB38[i] < min) {
                min = D_800EFB38[i];
                minidx = i;
            }
        }
        if (minidx == -1) {
            break;
        }
        order[count++] = minidx;
        mask += 1 << minidx;
    }
    for (j = 0; j < count; j++) {
        addr = D_800EFB38[0] + D_800EFC38[0][3];
    }
    for (j = 0; j < count; j++) {
        if (D_800EFB38[order[j]] == addr) {
            addr += D_800EFC38[order[j]][3];
        } else {
            for (k = j; k < count; k++) {
                addr = func_8005BF78(base, order[k], addr, D_800EFB38[order[k]]);
            }
            return 0;
        }
    }
    return 0;
}
