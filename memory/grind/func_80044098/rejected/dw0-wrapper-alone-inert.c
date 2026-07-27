/* REJECTED s4 (2026-07-27): do{}while(0) around store+dec+guard+loop WITHOUT
 * the cancellation pair (the sanctioned do-while-zero-exception family alone).
 * sandbox --disable all = 13 — INERT for the flip: the wrapper scales BOTH
 * pseudos' in-loop refs by the extra loop depth; the counter still wins on
 * shorter livelen. The permuter's output-0-1 carried this wrapper only as
 * random decoration — the pair is the entire mechanism (pair alone = 0). */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        do {
            *(v1 - 1) = a4;
            a4 = a4 - 1;
            if (a4 != -1) {
                do {
                    *v1 -= (s32)a6;
                    v1++;
                    a4--;
                } while (a4 != -1);
            }
        } while (0);
    }
}
