typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;

void f(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    s32 i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            u32 hi = cur >> (32 - bits); s32 need = 12 - bits; s32 left = 32 - need; cur = *ptr++; work[i+3] = (hi << need) | (cur >> left); bits = left; cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = (s32)ptr; res[1] = bits; res[2] = cur;
}
