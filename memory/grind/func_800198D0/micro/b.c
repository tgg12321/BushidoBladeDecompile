typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

#define GB(n) \
    (bits < (n) \
        ? (hi = cur >> (32 - bits), cur = *ptr++, need = (n) - bits, \
           bits = 32 - need, v = (hi << need) | (cur >> bits), cur <<= need, v) \
        : (v = cur >> (32 - (n)), cur <<= (n), bits -= (n), v))

void fb(u32 *ptr, u16 *work, s32 bits, u32 cur) {
    u32 hi, v, f;
    s32 need, i;
    for (i = 0; i < 63; i++) {
        f = GB(1);
        if (f) {
            work[i + 3] = GB(12);
        }
    }
    work[0] = bits; work[1] = cur; work[2] = (s32)ptr;
}
