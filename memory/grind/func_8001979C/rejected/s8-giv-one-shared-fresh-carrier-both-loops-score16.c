/* REJECTED (session 8): Giv chassis with ONE fresh local used as the OR carrier in BOTH loops: 16. A carrier live in both loops becomes a global allocno and re-opens the $v0/$v1 contest; two DISTINCT fresh locals score 0. */
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
    s32 nd;
    s32 neg2;
    u32 lo;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        dst = base + i * 2;
        if (bits_left < 0xC) {
            nd = 0xC - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo;
            *(s16 *)(dst + 0xA) = (s16)hi;
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
            nd = 2 - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo;
            *(s16 *)(dst2 + 0x8E) = (s16)hi;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
    } while (i < 0x3F);

    neg2 = -2;
    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = neg2;
        i--;
        out -= 0x118;
    } while (i >= 0);
    val = 0;
    *(s32 *)(base + 0x10C) = val;
}
