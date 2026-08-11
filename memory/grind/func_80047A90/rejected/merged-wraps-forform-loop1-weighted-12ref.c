/* REJECTED s6: merged counter + wraps but FOR-FORM loop 1 — loop notes weight
 * i loop-1 refs x2 -> 12 refs/67 = pri .537, above wrap-lifted pa2 .4375 ->
 * i allocates first -> a2. Loop 1 must be NOTE-FREE (goto-form) so i stays
 * at 9 unweighted refs (.403, inside the (.25,.424) window). See s6 m1 dump. */
void func_80047A90(void) {
    s32 i;
    s32 a3;
    s32 v1;
    s32 a0;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    for (i = 0; i < 0x11; i++) {
        D_800EF59C[i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }

    i = 1;
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    do {
        a3 = 0;
    } while (0);
    do {
        pa2 = pt2;
    } while (0);
    pt3 = pt1 + 0x11;
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (i == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        pa1++;
        a3 += 4;
        pa2++;
    }
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    i++;
    pt1 += 0x11;
    if (i < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
