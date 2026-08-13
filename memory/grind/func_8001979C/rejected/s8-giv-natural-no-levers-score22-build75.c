/* REJECTED (session 8): Session-8 giv chassis with NONE of the five allocation levers (no nd, no val, no hi-shift-amount reuse, one-statement OR, literal -2). Score 22, build_insns 75: the giv rewrite closes D4 but does not subsume D1/D2/D5. */
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    u32 dst2;
    u32 out;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        dst = base + i * 2;
        if (bits_left < 0xC) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
    } while (i < 0x3F);

    i = 0;
    do {
        dst2 = base + i * 2;
        if (bits_left < 2) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst2 + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
    } while (i < 0x3F);

    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = -2;
        i--;
        out -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
