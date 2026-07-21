/* HImode XOR compare: (s16)(cur ^ want) == 0. If combine folds the xor into
   the branch post-flow (eq(xor a b)0 -> eq a b) the HImode intermediate
   orphans -> phantom slot, while bytes stay beq v0,a0. */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        s16 want = a0;
        do {
            s16 cur;
            a3 = a2;
            cur = *a3;
            if (((s16)(cur ^ want)) == 0) goto found;
            i++;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        } while (i < count);
    }
found:
    if (i < D_800A33AC) {
        s32 old_a0 = D_800A33A0;
        s32 old_a4 = D_800A33A4;
        a0 = a1 - old_a0;
        old_a0 = old_a0 + a0;
        old_a4 = old_a4 - a0;
        *(s32 *)((u8 *)a3 + 8) = a0;
        D_800A33A0 = old_a0;
        D_800A33A4 = old_a4;
        return;
    }
not_found:
    InitFadePanel();
}
