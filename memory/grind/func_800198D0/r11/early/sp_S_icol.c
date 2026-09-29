extern s32 D_800A30EC;

/* Motion bitstream reader: `cur` holds the unread bits left-aligned, `bits`
   how many of them are valid, `ptr` the next stream word. GETBITS reads `nb`
   bits into `dst`; GETBITS_PRE ORs them onto `pre` (the implicit top bit of
   an escape/VLC code). */
#define GETBITS_PRE(dst, nb, pre)                               \
    {                                                           \
        u32 top = (pre);                                        \
        if (bits < (nb)) {                                      \
            s32 need = (nb) - bits;                             \
            u32 hi = cur >> (32 - bits);                        \
            s32 left = 32 - need;                               \
            cur = *ptr++;                                       \
            dst = top | ((hi << need) | (cur >> left));         \
            cur <<= need;                                       \
            bits = left;                                        \
        } else {                                                \
            dst = top | (cur >> (32 - (nb)));                   \
            cur <<= (nb);                                       \
            bits -= (nb);                                       \
        }                                                       \
    }

#define GETBITS(dst, nb)                                        \
    if (bits < (nb)) {                                          \
        s32 need = (nb) - bits;                                 \
        u32 hi = cur >> (32 - bits);                            \
        s32 left = 32 - need;                                   \
        cur = *ptr++;                                           \
        dst = (hi << need) | (cur >> left);                     \
        cur <<= need;                                           \
        bits = left;                                            \
    } else {                                                    \
        dst = cur >> (32 - (nb));                               \
        cur <<= (nb);                                           \
        bits -= (nb);                                           \
    }

#define COPY33(d, s)                                            \
    {                                                           \
        u32 *from = (u32 *)(s);                                 \
        u32 *to = (u32 *)(d);                                   \
        u32 k;                                                  \
        for (k = 0; k < 33; k++) {                              \
            *to++ = *from++;                                    \
        }                                                       \
    }

void func_800198D0(s32 idx, s32 frame, u32 *out, u16 *work) {
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
    s32 i;
    s32 col;
    s32 j;
    s32 n;
    s32 off;
    u16 *p;
    s16 delta;
    u16 code;
    u32 v;
    s16 x;

    sub = frame & 7;
    key = frame >> 3;
    rec = &D_800F1B18[idx * 0x570];
    tbl = (u8 *)*(u32 **)rec + 0x70;
    if (D_800A30EC == 0) {
        COPY33(work + 0x84, rec + 0x88);
    }
    ctr = *(s32 *)(rec + 0x10C);
    *(s32 *)(rec + 0x10C) = ctr + 1;
    slot = rec + (((ctr + 1) & 3) * 0x118 + 0x110);
    if (*(s32 *)slot == frame) {
        COPY33(out, slot + 4);
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    prev = rec + ((ctr & 3) * 0x118 + 0x110);
    if (*(s32 *)prev == frame) {
        COPY33(out, prev + 4);
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    if (*(s32 *)slot != frame - 1) {
        if (*(s32 *)prev == frame - 1) {
            slot = prev;
        } else {
            off = ((tbl[key * 3] << 16) | (tbl[key * 3 + 1] << 8) | tbl[key * 3 + 2]) + 0x380;
            ptr = *(u32 **)rec + (off >> 5);
            off &= 0x1F;
            bits = 32 - off;
            cur = *ptr++ << off;
            COPY33(work, rec + 4);
            GETBITS(v, 1);
            if (v) {
                GETBITS(v, 16);
            }
            work[0] = v;
            GETBITS(v, 1);
            if (v) {
                GETBITS(v, 16);
            }
            work[1] = v;
            GETBITS(v, 1);
            if (v) {
                GETBITS(v, 16);
            }
            work[2] = v;
            for (i = 0; i < 63; i++) {
                GETBITS(v, 1);
                if (v) {
                    GETBITS(work[i + 3], 12);
                }
            }
            j = 0;
            goto decode;
        }
    }
    ptr = *(u32 **)(slot + 0x10C);
    bits = *(s32 *)(slot + 0x110);
    cur = *(u32 *)(slot + 0x114);
    COPY33(work, slot + 4);
    if (sub >= 2) {
        COPY33(work + 0x42, slot + 0x88);
    }
    j = sub - 1;
decode:
    for (; j < sub; j++) {
        GETBITS(v, 1);
        if (v) {
            GETBITS(v, 16);
        }
        work[0] = v;
        GETBITS(v, 1);
        if (v) {
            GETBITS(v, 16);
        }
        work[1] = v;
        GETBITS(v, 1);
        if (v) {
            GETBITS(v, 16);
        }
        work[2] = v;
        for (i = 0; i < 63; i++) {
            code = work[i + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1:
                n = 0;
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
                    n++;
                } while (n < 12);
                if (n == 12) {
                    GETBITS_PRE(delta, 11, 0x800);
                } else if (n >= 2) {
                    n--;
                    GETBITS_PRE(delta, n, 1 << n);
                } else {
                    delta = n;
                }
                delta = (delta & 1) ? -(delta / 2) - 1 : delta / 2;
                break;
            case 2:
                GETBITS(delta, 1);
                if (delta) {
                    delta = 0;
                } else {
                    GETBITS(delta, 12);
                }
                break;
            case 3:
                GETBITS(delta, 1);
                if (delta) {
                    GETBITS(v, 4);
                    n = 0;
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
                        n++;
                    } while (n < 8);
                    if (n == 8) {
                        GETBITS_PRE(delta, 7, 0x80);
                    } else if (n >= 2) {
                        n--;
                        GETBITS_PRE(delta, n, 1 << n);
                    } else {
                        delta = n;
                    }
                    delta = ((delta << 3) | (v & 7)) + 1;
                    if (v & 8) {
                        delta = -delta;
                    }
                }
                break;
            }
            if (j == 0) {
                work[i + 0x45] = delta;
                work[i + 3] += delta;
            } else {
                work[i + 3] += work[i + 0x45] + delta;
                work[i + 0x45] += delta;
            }
        }
    }
    p = &work[0x36];
    for (j = 0; j < 2; j++) {
        for (col = 0; col < 3; col++) {
            x = *p;
            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);
            p++;
        }
        p += 3;
    }
    COPY33(out, work);
    if (sub < 7) {
        *(s32 *)slot = frame;
        COPY33(slot + 4, work);
        COPY33(slot + 0x88, work + 0x42);
        *(u32 **)(slot + 0x10C) = ptr;
        *(s32 *)(slot + 0x110) = bits;
        *(u32 *)(slot + 0x114) = cur;
    }
}
