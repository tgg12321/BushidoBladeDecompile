/* REJECTED (s4 forensics, 2026-08-19): TWO-BASE spelling of the s3 table pair —
 *     out[0x42] = (&D_8008F19C)[s3 * 2];
 *     out[0x43] = (&D_8008F19D)[s3 * 2];
 * (this is the shape the pre-migration cheat form used, and the shape the
 * splat-named scalar header invites).
 *
 * MEASURED DEAD on the current chassis:
 *   sandbox func_80038170 --disable all = 13   (vs 0 for the one-base form)
 *   cc1 .frame $sp,48 # vars= 8   (target needs vars= 16, frame 56)
 *
 * MECHANISM (instrumented cc1 + -da dumps, artifacts under
 * tmp/grind/func_80038170/s4/ and .../dumps/): a bare SYMBOL_REF is a legal MIPS
 * `lbu sym($r)` address, so expand never creates the address pseudo that the
 * one-base `+1` addend form creates (reg 120). No pseudo -> no combine
 * address-fold residue `(use (reg:SI 120))` -> global.c never leaves an
 * unallocated referenced pseudo -> reload1.c:2403 alter_reg never calls
 * assign_stack_local for it -> get_frame_size() stays 8 instead of 16. The
 * 8-byte gap then shifts every register-save offset, which is the entire
 * 13-instruction distance.
 *
 * This is the decisive KILL for the two-symbol program model: the target's own
 * frame size PROVES the original C indexed a single stride-2 array. Do not
 * re-propose any two-base spelling for this pair.
 */
void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    mask = D_80106A50;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = D_8008F204[i];
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

    strcpy(out + 4, D_8008F1C0);

    out[0x22] = D_8008F1A8[s1 * 2 + 0];
    out[0x23] = D_8008F1A8[s1 * 2 + 1];
    out[0x3C] = D_8008F1A8[s2 * 2 + 0];
    out[0x3D] = D_8008F1A8[s2 * 2 + 1];

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
