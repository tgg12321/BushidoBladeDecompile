typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

s32 fc(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi, v, f;
    s32 need, i, t;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            need = 12 - bits;
            t = 32 - need;
            bits = t;
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20;
            cur <<= 12;
            bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
    return t;
}
