/* s42 (synthesis, 2026-09-05): value CHAINING -- each flag block consumes the
 * previous block's computed value instead of re-reading the flag byte, spelled
 * with two alternating named value locals (ordinary C, no self-assign).
 * Measured 24 at 45 insns: with nothing left for cse to forward, blocks 2 and 3
 * lose their `lbu` reloads entirely and the body is FOUR instructions short of
 * the target's 49.  The value allocnos' live ranges do lengthen (the
 * allocno_compare parameter s39 identified), but the price is paid in EMISSION,
 * not in allocation.  DO NOT RE-PROPOSE. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    s32 v;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            v = m | 1;
        } else {
            v = m;
        }
        *q = v;
    }

    {
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        if (c) {
            m = v | 2;
        } else {
            m = v;
        }
        *q = m;
    }

    {
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        if (c) {
            v = m | 4;
        } else {
            v = m;
        }
        *q = v;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
