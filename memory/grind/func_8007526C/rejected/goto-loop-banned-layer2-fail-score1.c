/* REJECTED-BY-BAN (not by measurement).  s1/2026-09-07 moved this here from candidate.c.
 * This body measures honest floor 1 (90 of the target's 91 words, all identical; the only
 * missing word is the assembler-layer load-delay nop at asm/funcs/func_8007526C.s:6), but
 * its loop is spelled label + backward goto, which layer-2 FAILED on 2026-09-07 (Match
 * 121e34d7 reverted by 73e786dd) and which is now state.json banned_constructs[0].
 * Do NOT re-propose it.  Kept only as the byte-level reference for what the target's
 * instruction stream is, and as proof that the residual is not in the function's semantics.
 */
void func_8007526C(void) {
    u8 *p;
    s32 i;
    s32 lim;

    i = 0;
    lim = 0xC8;
    p = D_800A36A0;
loop:
    switch (*(u8 *)(p + 0x10)) {
    case 1:
        *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
        if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
            if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
            }
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = lim;
            *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
            *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
        }
        break;
    case 3:
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
        if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
            *(u16 *)(p + 8) = lim;
            *(u16 *)(p + 0xC) = lim;
            *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
            *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
            }
        }
        break;
    case 2:
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
        if ((s16)*(u16 *)(p + 0xC) <= 0) {
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = 0;
            *(u16 *)(p + 0x10) = 0;
        }
        break;
    case 4:
        *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
        if ((s16)*(u16 *)(p + 0xC) <= 0) {
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = 0;
            *(u16 *)(p + 0x10) = 0;
        }
        break;
    }
    i++;
    p += 2;
    if (i < 2) goto loop;
}
