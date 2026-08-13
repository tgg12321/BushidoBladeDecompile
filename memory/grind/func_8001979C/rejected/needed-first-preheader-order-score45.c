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
    s32 val;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];
    dst = base;

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        if (bits_left < 0xC) {
            needed = 0xC - bits_left;
            val = 0x20 - bits_left;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            val = 0x20 - needed;
            bits_left = val;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
            dst += 2;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
            dst += 2;
        }
        i++;
    } while (i < 0x3F);

    dst2 = base;
    i = 0;
    do {
        if (bits_left < 2) {
            needed = 2 - bits_left;
            val = 0x20 - bits_left;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            val = 0x20 - needed;
            bits_left = val;
            *(s16 *)(dst2 + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
            dst2 += 2;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
            dst2 += 2;
        }
        i++;
    } while (i < 0x3F);

    val = -2;
    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = val;
        i--;
        out -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
