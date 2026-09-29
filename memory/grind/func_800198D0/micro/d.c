typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;

/* d1: nested assignment of need inside bits update */
void d1(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; s32 need, i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            bits = 32 - (need = 12 - bits);
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}

/* d2: bits updated via need-relative add */
void d2(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; s32 need, i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            need = 12 - bits;
            bits += 32 - 12;
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}

/* d3: need computed from bits after bits reassigned (bits = 32 - (12 - bits)) and need separately */
void d3(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; s32 need, i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            need = 12 - bits;
            bits = 32 - (12 - bits);
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}

/* d4: shift counts as u32 */
void d4(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; u32 need; s32 i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            need = 12 - bits;
            bits = 32 - need;
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}

/* d5: bits as u32 compared signed? use u8 need */
void d5(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; s32 i; s32 need;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            need = 12 - bits;
            bits = 32 - need;
            cur = *ptr++;
            work[i + 3] = (hi << need) | (cur >> bits);
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}

/* d6: value via ternary in expression with assignment-in-shift */
void d6(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    u32 hi; s32 i; s32 need;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            hi = cur >> (32 - bits);
            cur = *ptr++;
            need = 12 - bits;
            work[i + 3] = (hi << need) | (cur >> (bits = 32 - need));
            cur <<= need;
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = bits; res[1] = cur; res[2] = (s32)ptr;
}
