/* PROBE — two SImode uses of the s16 local so the lh def cannot be folded
   into a single use: compare in loop + re-compare at found join (the second
   compare is semantically true==the loop exit condition mix, changes CFG —
   measuring frame effect only). */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    s16 cur;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            a3 = a2;
            cur = *a3;
            if (cur == a0) goto found;
            i++;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        } while (i < count);
    }
found:
    if (i < D_800A33AC && cur == a0) {
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
