/* REJECTED (s5, rederive, 2026-08-20) — 2-D array declaration shape.
 *
 * SHAPE: include/code6cac.h  `extern u8 D_8008F19C[][2];`  (instead of `[]`)
 *        src                 `out[0x42] = D_8008F19C[s3][0];`
 *                            `out[0x43] = D_8008F19C[s3][1];`
 * This is the shape a human would write once the data model is known — the
 * table at 0x8008F19C is five 2-byte Shift-JIS characters, so `[idx][byte]`
 * states the model directly. It does NOT match.
 *
 * MEASURED THIS SESSION (full build, the real oracle):
 *   build SHA1 e3ae7aae6692ef268d22344bf0e7bd8c8001f34b != oracle
 *   12 differing bytes, all in the prologue/epilogue frame words:
 *     0x80038170 addiu $sp,$sp,-0x30   (target -0x38)
 *     every sw/lw $sN save-offset 8 lower; 0x80038398 addiu $sp,$sp,0x30
 *   sandbox (against a CLEAN build/ reference) = 12.
 *
 * MECHANISM (consistent with the s4 forensics): `x[i][j]` on a `u8 [][2]`
 * makes GCC form the row address `&D_8008F19C[s3]` and emit the two loads as
 * offsets 0/1 off that register. There is therefore NO `symbol_ref + 1` const
 * addend anywhere, so combine has no address-fold to perform, no dead
 * `(use (reg 120))` residue survives, global.c forms no SET-less pseudo, and
 * reload1.c alter_reg never reserves the extra 8-byte slot. Result: cc1
 * reports vars= 8 / frame 48, target needs vars= 16 / frame 56.
 *
 * COROLLARY: the target's 56-byte frame is only reachable when the second
 * read is spelled as a +1 addend on the SAME symbol (`D_8008F19C[s3*2+1]`).
 * That narrows the original source's expression shape further than s4 had it.
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
        out[0x42] = D_8008F19C[s3][0];
        out[0x43] = D_8008F19C[s3][1];
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
