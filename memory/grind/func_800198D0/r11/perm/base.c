



typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
extern u8 D_800F1B18[];
extern s32 D_800A30EC;
void func_800198D0(s32 obj, s32 frame, u32 *out, u16 *work) {
    u8 *rec;
    u8 *tbl;
    u8 *slot;
    u8 *prev;
    u32 *ptr;
    u32 cur;
    s32 bits;
    s32 sub;
    s32 key;
    s32 ctr;
    s32 col;
    s32 ch;
    s32 step;
    s32 row;
    s32 off;
    s32 shift;
    u16 *p;
    u16 code;
    s16 x;

    sub = frame & 7;
    key = frame >> 3;
    rec = &D_800F1B18[obj * 0x570];
    tbl = (u8 *)*(u32 **)rec + 0x70;
    if (D_800A30EC == 0) {
        { u32 *from = (u32 *)(rec + 0x88); u32 *to = (u32 *)(work + 0x84); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
    }
    ctr = *(s32 *)(rec + 0x10C);
    *(s32 *)(rec + 0x10C) = ctr + 1;
    slot = rec + (((ctr + 1) & 3) * 0x118 + 0x110);
    if (*(s32 *)slot == frame) {
        { u32 *from = (u32 *)(slot + 4); u32 *to = (u32 *)(out); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    prev = rec + ((ctr & 3) * 0x118 + 0x110);
    if (*(s32 *)prev == frame) {
        { u32 *from = (u32 *)(prev + 4); u32 *to = (u32 *)(out); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    if (*(s32 *)slot != frame - 1) {
        if (*(s32 *)prev == frame - 1) {
            slot = prev;
        } else {
            s32 kidx;
            u32 h2;
            u32 h1;
            u32 h0;
            off = ((tbl[key * 3] << 16) | (tbl[key * 3 + 1] << 8) | tbl[key * 3 + 2]) + 0x380;
            ptr = *(u32 **)rec + (off >> 5);
            shift = off & 0x1F;
            bits = 32 - shift;
            cur = *ptr++ << shift;
            { u32 *from = (u32 *)(rec + 4); u32 *to = (u32 *)(work); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
            if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h0 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h0 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
            if (h0) {
                if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h0 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h0 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
            }
            work[0] = h0;
            if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h1 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h1 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
            if (h1) {
                if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h1 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h1 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
            }
            work[1] = h1;
            if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h2 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h2 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
            if (h2) {
                if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h2 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h2 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
            }
            work[2] = h2;
            for (kidx = 0; kidx < 63; kidx++) {
                u32 kflag;

                if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; kflag = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { kflag = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
                if (kflag) {
                    if (bits < (12)) { s32 need = (12) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; work[kidx + 3] = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { work[kidx + 3] = cur >> (32 - (12)); cur <<= (12); bits -= (12); };
                }
            }
            step = 0;
            goto decode;
        }
    }
    ptr = *(u32 **)(slot + 0x10C);
    bits = *(s32 *)(slot + 0x110);
    cur = *(u32 *)(slot + 0x114);
    { u32 *from = (u32 *)(slot + 4); u32 *to = (u32 *)(work); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
    if (sub >= 2) {
        { u32 *from = (u32 *)(slot + 0x88); u32 *to = (u32 *)(work + 0x42); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
    }
    step = sub - 1;
decode:
    for (; step < sub; step++) {
        u32 h5;

        u32 h4;

        u32 h3;

        if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h3 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h3 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
        if (h3) {
            if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h3 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h3 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
        }
        work[0] = h3;
        if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h4 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h4 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
        if (h4) {
            if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h4 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h4 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
        }
        work[1] = h4;
        if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h5 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h5 = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
        if (h5) {
            if (bits < (16)) { s32 need = (16) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; h5 = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { h5 = cur >> (32 - (16)); cur <<= (16); bits -= (16); };
        }
        work[2] = h5;
        for (ch = 0; ch < 63; ch++) {
            s16 delta;

            code = work[ch + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1: {
                s32 zeros;
                s16 mag1;

                zeros = 0;
                do {
                    u32 bit;

                    if (bits == 0) {
                        cur = *ptr++;
                        bits = 32;
                    }
                    bit = cur >> 31;
                    cur <<= 1;
                    bits--;
                    if (bit) {
                        break;
                    }
                    zeros++;
                } while (zeros < 12);
                if (zeros == 12) {
                    { u32 top = (0x800); if (bits < (11)) { s32 need = (11) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; mag1 = top | ((hi << need) | (cur >> left)); cur <<= need; bits = left; } else { mag1 = top | (cur >> (32 - (11))); cur <<= (11); bits -= (11); } };
                } else if (zeros >= 2) {
                    s32 len;

                    len = zeros - 1;
                    { u32 top = (1 << len); if (bits < (len)) { s32 need = (len) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; mag1 = top | ((hi << need) | (cur >> left)); cur <<= need; bits = left; } else { mag1 = top | (cur >> (32 - (len))); cur <<= (len); bits -= (len); } };
                } else {
                    mag1 = zeros;
                }
                delta = (mag1 & 1) ? -(mag1 / 2) - 1 : mag1 / 2;
                break;
            }
            case 2: {
                s16 flag;

                if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; flag = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { flag = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
                if (flag) {
                    delta = 0;
                } else {
                    if (bits < (12)) { s32 need = (12) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; delta = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { delta = cur >> (32 - (12)); cur <<= (12); bits -= (12); };
                }
                break;
            }
            case 3: {
                if (bits < (1)) { s32 need = (1) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; delta = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { delta = cur >> (32 - (1)); cur <<= (1); bits -= (1); };
                if (delta) {
                    s32 zeros2;
                    s16 mag3;
                    u32 lo;

                    if (bits < (4)) { s32 need = (4) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; lo = (hi << need) | (cur >> left); cur <<= need; bits = left; } else { lo = cur >> (32 - (4)); cur <<= (4); bits -= (4); };
                    zeros2 = 0;
                    do {
                        u32 bit;

                        if (bits == 0) {
                            cur = *ptr++;
                            bits = 32;
                        }
                        bit = cur >> 31;
                        cur <<= 1;
                        bits--;
                        if (bit) {
                            break;
                        }
                        zeros2++;
                    } while (zeros2 < 8);
                    if (zeros2 == 8) {
                        { u32 top = (0x80); if (bits < (7)) { s32 need = (7) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; mag3 = top | ((hi << need) | (cur >> left)); cur <<= need; bits = left; } else { mag3 = top | (cur >> (32 - (7))); cur <<= (7); bits -= (7); } };
                    } else if (zeros2 >= 2) {
                        s32 len2;

                        len2 = zeros2 - 1;
                        { u32 top = (1 << len2); if (bits < (len2)) { s32 need = (len2) - bits; u32 hi = cur >> (32 - bits); s32 left = 32 - need; cur = *ptr++; mag3 = top | ((hi << need) | (cur >> left)); cur <<= need; bits = left; } else { mag3 = top | (cur >> (32 - (len2))); cur <<= (len2); bits -= (len2); } };
                    } else {
                        mag3 = zeros2;
                    }
                    delta = ((mag3 << 3) | (lo & 7)) + 1;
                    if (lo & 8) {
                        delta = -delta;
                    }
                }
                break;
            }
            }
            if (step == 0) {
                work[ch + 0x45] = delta;
                work[ch + 3] += delta;
            } else {
                work[ch + 3] += work[ch + 0x45] + delta;
                work[ch + 0x45] += delta;
            }
        }
    }
    p = &work[0x36];
    for (row = 0; row < 2; row++) {
        for (col = 0; col < 3; col++) {
            x = *p;
            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);
            p++;
        }
        p += 3;
    }
    { u32 *from = (u32 *)(work); u32 *to = (u32 *)(out); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
    if (sub < 7) {
        *(s32 *)slot = frame;
        { u32 *from = (u32 *)(work); u32 *to = (u32 *)(slot + 4); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
        { u32 *from = (u32 *)(work + 0x42); u32 *to = (u32 *)(slot + 0x88); u32 k; for (k = 0; k < 33; k++) { *to++ = *from++; } };
        *(u32 **)(slot + 0x10C) = ptr;
        *(s32 *)(slot + 0x110) = bits;
        *(u32 *)(slot + 0x114) = cur;
    }
}
