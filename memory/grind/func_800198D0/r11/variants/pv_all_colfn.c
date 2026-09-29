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

/* Decode motion frame `frame` of object `obj` into `out` (33 words). Frames are
   keyframes every 8 plus up to 7 delta sub-frames; the record at
   D_800F1B18[obj * 0x570] caches the last four decoded frames with the reader
   state after each, so a frame is either copied from the cache, continued from
   the cached previous frame, or decoded from its keyframe. `work` holds the
   current pose (+0x00), the per-channel rates (+0x84) and the channel codes
   (+0x108). */
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
    /* Ruling 11 (ordinary-c-judge-decidable.md): two values, both loop indices --
     * the keyframe channel loop's and the post-pass column loop's. (D) proof:
     * memory/grind/func_800198D0/r11/proof.md */
    s32 col;
    s32 ch;
    /* Ruling 11: two values, both loop indices -- the sub-frame loop's and the
     * post-pass row loop's. (D) proof: memory/grind/func_800198D0/r11/proof.md */
    s32 step;
    s32 row;
    s32 off;
    s32 shift;
    u16 *p;
    u16 code;
    /* Ruling 11: eight values, each a bit field read by GETBITS -- the three
     * keyframe header words, the per-channel keyframe flag, the three
     * sub-frame header words and case 3's 4-bit low code. (D) proof: memory/grind/func_800198D0/r11/proof.md */
    s16 x;

    sub = frame & 7;
    key = frame >> 3;
    rec = &D_800F1B18[obj * 0x570];
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
            s32 kidx;
            u32 h2;
            u32 h1;
            u32 h0;
            off = ((tbl[key * 3] << 16) | (tbl[key * 3 + 1] << 8) | tbl[key * 3 + 2]) + 0x380;
            ptr = *(u32 **)rec + (off >> 5);
            shift = off & 0x1F;
            bits = 32 - shift;
            cur = *ptr++ << shift;
            COPY33(work, rec + 4);
            GETBITS(h0, 1);
            if (h0) {
                GETBITS(h0, 16);
            }
            work[0] = h0;
            GETBITS(h1, 1);
            if (h1) {
                GETBITS(h1, 16);
            }
            work[1] = h1;
            GETBITS(h2, 1);
            if (h2) {
                GETBITS(h2, 16);
            }
            work[2] = h2;
            for (kidx = 0; kidx < 63; kidx++) {
                u32 kflag;

                GETBITS(kflag, 1);
                if (kflag) {
                    GETBITS(work[kidx + 3], 12);
                }
            }
            step = 0;
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
    step = sub - 1;
decode:
    for (; step < sub; step++) {
        u32 h5;

        u32 h4;

        u32 h3;

        GETBITS(h3, 1);
        if (h3) {
            GETBITS(h3, 16);
        }
        work[0] = h3;
        GETBITS(h4, 1);
        if (h4) {
            GETBITS(h4, 16);
        }
        work[1] = h4;
        GETBITS(h5, 1);
        if (h5) {
            GETBITS(h5, 16);
        }
        work[2] = h5;
        for (ch = 0; ch < 63; ch++) {
            /* Ruling 11: four values -- the channel's decoded delta, case 1's
             * magnitude, case 2's zero flag and case 3's magnitude. (D) proof:
             * memory/grind/func_800198D0/r11/proof.md */
            s16 delta;

            code = work[ch + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1: {
                /* Ruling 11: two values, both bit counts -- the zero-run length and
                 * the suffix length (one less). (D) proof: memory/grind/func_800198D0/r11/proof.md */
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
                    GETBITS_PRE(mag1, 11, 0x800);
                } else if (zeros >= 2) {
                    s32 len;

                    len = zeros - 1;
                    GETBITS_PRE(mag1, len, 1 << len);
                } else {
                    mag1 = zeros;
                }
                delta = (mag1 & 1) ? -(mag1 / 2) - 1 : mag1 / 2;
                break;
            }
            case 2: {
                s16 flag;

                GETBITS(flag, 1);
                if (flag) {
                    delta = 0;
                } else {
                    GETBITS(delta, 12);
                }
                break;
            }
            case 3: {
                GETBITS(delta, 1);
                if (delta) {
                    /* Ruling 11: two values, both bit counts -- the zero-run length
                     * and the suffix length (one less). (D) proof: memory/grind/func_800198D0/r11/proof.md */
                    s32 zeros2;
                    s16 mag3;
                    u32 lo;

                    GETBITS(lo, 4);
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
                        GETBITS_PRE(mag3, 7, 0x80);
                    } else if (zeros2 >= 2) {
                        s32 len2;

                        len2 = zeros2 - 1;
                        GETBITS_PRE(mag3, len2, 1 << len2);
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
