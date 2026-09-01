/* REJECTED s49: nrefs_flow(72) = 3 — the plain H1 basin, no sandbox run spent.
 *
 * Control for rejected/h1-second-loop-guard-on-a0.c.  Keeps `i = a0;` in front
 * of the second loop's guard (so the 83-instruction shape survives) but spells
 * the guard on the parameter, `if (a0 < D_800A33AC)`.  Per the owner's
 * 2026-09-01 Ruling A the reference count was checked with
 * tools/ra_solver/extract.py BEFORE spending a sandbox run:
 *   tmp/grind/func_80045294/s49/v1d.model.json -> pseudo 72 nrefs_flow = 3.
 * cse.c substitutes the guard's a0 -> i (the `i = a0` copy immediately dominates
 * it in the same basic block), so no fourth reference is created and this is
 * bit-for-bit the score-11 H1 basin.  Not sandboxed.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (a0 < D_800A33AC) {
            v1 = i << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
