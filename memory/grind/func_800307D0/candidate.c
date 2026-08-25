/* func_800307D0 -- APPLIED to src/code6cac_b.c (s10, 2026-08-25).
 * Honest sandbox --disable all = 0 (76/76, 0 rules) and full-build SHA1 == oracle,
 * both measured with this body in place. Verbatim copy of what is in src/ --
 * no grind narration is carried into the source file (see evidence.md == s10 ==).
 */
s32 func_800307D0(u8 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 s3;
    s32 *a2;
    s32 i;

    count = *(s16 *)(a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)(a0 + 0x88) == -1) {
        goto do_sll;
    }
    v0 = *(s16 *)(a0 + 0x332);
    v1 = *(s16 *)(a0 + 0x14);
    v0 = v0 ^ v1;
    s1 = (u32)v0 < 1;
do_sll:
    s3 = *(s16 *)(a0 + s1 * 2 + 0x332);
    a2 = func_80030580((s32 *)a0, s3);
    for (i = s1; i < *(s16 *)(a0 + 0x330) - 1; i++) {
        *(u16 *)(a0 + 0x332 + i * 2) = *(u16 *)(a0 + 0x334 + i * 2);
    }

    *(u16 *)(a0 + 0x330) = *(u16 *)(a0 + 0x330) - 1;
    v1 = *(s16 *)((u8 *)a2 + 2);
    if (v1 == 0xE) {
        s32 a0_arg = D_800A36F2 ^ 0xE;
        func_80032854(a0_arg != 0, 0x2F, (u8 *)a2 + 0x2C, 0);
    } else {
        s32 a0_arg = v1 ^ D_800A36F2;
        func_80032854(a0_arg != 0, 0x2A, (u8 *)a2 + 0x2C, 0);
    }
    return s3;
}
