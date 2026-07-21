/* s2 REJECTED: dead index-consumers of cur are DCE'd cleanly BEFORE reload —
   vars=0, body byte-IDENTICAL to baseline. The "zero-cost rider via a
   fold-eliminated second consumer" route is measured dead: a consumer that
   vanishes leaves no orphan (x1 dead &a3[cur] address-compute; x2 dead
   indexed load s16 t2 = a3[cur]). Also x5: a live STORE-VALUE consumer
   (*(s32*)((u8*)a3+8) = *a3) does not fire either (vars=0, +2 diff lines).
   NB these dead-use spellings would ALSO be cheats-by-spelling if proposed
   as candidates; they were scratch measurements only. */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            s16 cur;
            s16 *t;
            a3 = a2;
            cur = *a3;
            t = &a3[cur]; /* dead — DCE'd pre-reload, no orphan */
            if (cur == a0) goto found;
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
