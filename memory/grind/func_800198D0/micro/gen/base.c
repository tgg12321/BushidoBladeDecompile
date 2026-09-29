typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

void f(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi, v; s32 need, t;
    s32 i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = (s32)ptr; res[1] = bits; res[2] = cur;
    
}
