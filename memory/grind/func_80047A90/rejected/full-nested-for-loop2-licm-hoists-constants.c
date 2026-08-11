/* REJECTED session 1: loop 2 as real nested for-loops — sandbox 41 (87 insns).
 * loop.c LICM hoists 0x7D0, 0x66666667 magic, and 8 out of the inner loop and keeps
 * sll-index addressing; target keeps all constants INSIDE the loop and walks pointers.
 * Loop 2 must be goto-form (no loop notes). Loop 1 half of this form is CORRECT and kept. */
void func_80047A90(void) {
    s32 i;
    s32 j;
    s32 v1;
    s32 a0;
    s32 *temp;

    for (i = 0; i < 0x11; i++) {
        D_800EF59C[i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }

    for (i = 1; i < 9; i++) {
        for (j = 0; j < 0x11; j++) {
            a0 = 0x7D0 - (D_800EF59C[i * 0x11 + j] - D_800EF59C[(i - 1) * 0x11 + j]);
            if (a0 < 0) {
                v1 = (a0 + 0xF) >> 4;
            } else {
                v1 = a0 / 10;
            }
            D_800EF59C[i * 0x11 + j] += v1;
            if (i == 8) {
                g_snd_fade_curve[j] = v1;
            }
        }
    }

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
