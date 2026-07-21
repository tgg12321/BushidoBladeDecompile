/* s16 first param + s16 local compare (probe function-local frame only) */
void saSeMain_80045600(s16 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            s16 cur;
            a3 = a2;
            cur = *a3;
            if (cur == a0) goto found;
            i++;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        } while (i < count);
    }
found:
    if (i < D_800A33AC) {
        s32 old_a0 = D_800A33A0;
        s32 old_a4 = D_800A33A4;
        s32 diff;
        diff = a1 - old_a0;
        old_a0 = old_a0 + diff;
        old_a4 = old_a4 - diff;
        *(s32 *)((u8 *)a3 + 8) = diff;
        D_800A33A0 = old_a0;
        D_800A33A4 = old_a4;
        return;
    }
not_found:
    InitFadePanel();
}
