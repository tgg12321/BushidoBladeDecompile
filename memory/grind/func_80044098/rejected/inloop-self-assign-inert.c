/* REJECTED s4 (2026-07-27): in-loop `v1 = v1;` self-assign
 * (dead-store-fake-exception family, the sanctioned spelling of a ref-lift).
 * sandbox --disable all = 13 — completely INERT: the no-op move is discarded
 * before flow counts refs (pointer stays 12 refs / 21176), unlike the
 * cancellation pair which is two REAL sets cse can't fold. This kills the
 * hope of landing the s4 pair-lift mechanism via the sanctioned dead-store
 * family. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                v1++;
                a4--;
                v1 = v1;
            } while (a4 != -1);
        }
    }
}
