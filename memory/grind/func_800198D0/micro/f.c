typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

#define GB(n) \
    (bits < (n) \
        ? ({ u32 hi = cur >> (32 - bits); s32 need; u32 v; cur = *ptr++; need = (n) - bits; \
             bits = 32 - need; v = (hi << need) | (cur >> bits); cur <<= need; v; }) \
        : ({ u32 v = cur >> (32 - (n)); cur <<= (n); bits -= (n); v; }))

void f1(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    s32 i;
    s16 delta;
    for (i = 0; i < 63; i++) {
        if (GB(1)) {
            delta = GB(11) | 0x800;
            work[i + 3] = delta;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}
