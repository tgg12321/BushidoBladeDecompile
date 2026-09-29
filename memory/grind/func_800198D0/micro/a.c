typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

/* A: expression macro, assignment embedded in shift amount */
#define GB_A(n) \
    (bits < (n) \
        ? (hi = cur >> (32 - bits), cur = *ptr++, need = (n) - bits, \
           v = (hi << need) | (cur >> (bits = 32 - need)), cur <<= need, v) \
        : (v = cur >> (32 - (n)), cur <<= (n), bits -= (n), v))

void fa(u32 *ptr, u16 *work, s32 bits, u32 cur) {
    u32 hi, v;
    s32 need, i;
    for (i = 0; i < 63; i++) {
        if (GB_A(1)) {
            work[i + 3] = GB_A(12);
        }
    }
    work[0] = bits; work[1] = cur; work[2] = (s32)ptr;
}
