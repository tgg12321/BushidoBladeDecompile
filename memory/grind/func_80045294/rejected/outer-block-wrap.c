/* s38 structural probe: whole-body outer `{ }` block wrap.
 *
 * Hypothesis: an added lexical scope around the entire body (post-prototype,
 * pre-decls) creates a per-block pseudo-creation context distinct from
 * function-wide RA, possibly shifting the sll/move16 sched2 tie via a
 * different global_alloc allocno ordering.
 *
 * Measurement: sandbox --disable all -> score=2, target_insns=83,
 * build_insns=83, rules_dropped=0. NEUTRAL. Verdict KILLED.
 *
 * Mechanism: consistent with s4's inner-block-defer-v1 finding -- global.c
 * allocates function-wide in one phase; block-scope lexical boundaries have
 * NO effect on pseudo LUID assignment or allocno priority. This probe is a
 * distinct structural surface (no statement-position v1 assignment, unlike
 * s4/inner-block-defer-v1) but reaches the same pass-level dead end. */
void func_80045294(s32 a0, s32 a1) {
    {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
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
        if (i < D_800A33AC) {
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
}
