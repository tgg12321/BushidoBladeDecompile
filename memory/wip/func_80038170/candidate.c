/* WIP candidate: func_80038170 (code6cac_c_mid.c)
 * Honest distance: 12 (all from +8-byte frame delta; masked Levenshtein --disable all)
 * Pure-C form — no register pins, no dummy "m" frame-coercion, no reorder rule.
 * Declaration order s1,s2,s3 is the logical (original HEAD) order.
 * s3,s2,s1 reversed order was tried and FAILed cheat-reviewer (prologue-save-order
 * manipulation, param-local-alias-prologue-pair-flip family).
 * Frame gap (vars=8 natural vs vars=16 target) cannot be closed without
 * forbidden constructs (dead-vars-local-array detector refuses completion).
 */

void func_80038170(u8 *out) {
    s32 s1 = 0, s2 = 0, s3 = 0;
    s32 i;
    s32 mask;
    s32 bit;

    mask = D_80106A50;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = (&D_8008F204)[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    func_80079194(out + 4, &D_8008F1C0);

    out[0x22] = (&D_8008F1A8)[s1 * 2 + 0];
    out[0x23] = (&D_8008F1A8)[s1 * 2 + 1];
    out[0x3C] = (&D_8008F1A8)[s2 * 2 + 0];
    out[0x3D] = (&D_8008F1A8)[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = (&D_8008F19C)[s3 * 2];
        out[0x43] = (&D_8008F19D)[s3 * 2];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}
