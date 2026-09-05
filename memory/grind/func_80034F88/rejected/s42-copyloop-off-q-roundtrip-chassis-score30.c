/* s42 (synthesis, 2026-09-05) RE-AUDIT of s37's h12/h13 on the POST-s39
 * round-trip chassis: feeding the trailing copy loop off `q` (`*(q - 3 + i)`)
 * still measures 30 at 48 insns, exactly as it did on the pre-round-trip
 * chassis.  The loop gives up its own lui %hi(D_80106A70) / addu at,at,v1 /
 * sb %lo(D_80106A70)($at) materialisation -- three instructions the target
 * DOES contain -- so the reference-count lift on q can never pay for itself.
 * DO NOT RE-PROPOSE. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
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
            c = m | 1;
        } else {
            c = m;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(q - 3 + i) = *((u8 *)p + i + 0x17);
    }
}
