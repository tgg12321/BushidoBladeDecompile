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
    s32 ch;
    /* Ruling 11: two values, both loop indices -- the sub-frame loop's and the
     * post-pass row loop's. (D) proof: memory/grind/func_800198D0/r11/proof.md */
    s32 idx2;
    s32 off;
    s32 shift;
    u16 *p;
    u16 code;
    /* Ruling 11: eight values, each a bit field read by GETBITS -- the three
     * keyframe header words, the per-channel keyframe flag, the three
     * sub-frame header words and case 3's 4-bit low code. (D) proof: memory/grind/func_800198D0/r11/proof.md */
    u32 field;
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
            off = ((tbl[key * 3] << 16) | (tbl[key * 3 + 1] << 8) | tbl[key * 3 + 2]) + 0x380;
            ptr = *(u32 **)rec + (off >> 5);
            shift = off & 0x1F;
            bits = 32 - shift;
            cur = *ptr++ << shift;
            COPY33(work, rec + 4);
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[0] = field;
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[1] = field;
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[2] = field;
            kidx = 0;
            while (kidx < 63) {
                GETBITS(field, 1);
                if (field) {
                    GETBITS(work[kidx + 3], 12);
                }
                kidx++;
            }
            idx2 = 0;
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
    idx2 = sub - 1;
decode:
    for (; idx2 < sub; idx2++) {
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[0] = field;
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[1] = field;
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[2] = field;
        for (ch = 0; ch < 63; ch++) {
            /* Ruling 11: four values -- the channel's decoded delta, case 1's
             * magnitude, case 2's zero flag and case 3's magnitude. (D) proof:
             * memory/grind/func_800198D0/r11/proof.md */
            s16 temp;

            code = work[ch + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1: {
                /* Ruling 11: two values, both bit counts -- the zero-run length and
                 * the suffix length (one less). (D) proof: memory/grind/func_800198D0/r11/proof.md */
                s32 nbits;

                nbits = 0;
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
                    nbits++;
                } while (nbits < 12);
                if (nbits == 12) {
                    GETBITS_PRE(temp, 11, 0x800);
                } else if (nbits >= 2) {
                    nbits = nbits - 1;
                    GETBITS_PRE(temp, nbits, 1 << nbits);
                } else {
                    temp = nbits;
                }
                temp = (temp & 1) ? -(temp / 2) - 1 : temp / 2;
                break;
            }
            case 2: {
                GETBITS(temp, 1);
                if (temp) {
                    temp = 0;
                } else {
                    GETBITS(temp, 12);
                }
                break;
            }
            case 3: {
                GETBITS(temp, 1);
                if (temp) {
                    /* Ruling 11: two values, both bit counts -- the zero-run length
                     * and the suffix length (one less). (D) proof: memory/grind/func_800198D0/r11/proof.md */
                    s32 nbits2;

                    GETBITS(field, 4);
                    nbits2 = 0;
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
                        nbits2++;
                    } while (nbits2 < 8);
                    if (nbits2 == 8) {
                        GETBITS_PRE(temp, 7, 0x80);
                    } else if (nbits2 >= 2) {
                        nbits2 = nbits2 - 1;
                        GETBITS_PRE(temp, nbits2, 1 << nbits2);
                    } else {
                        temp = nbits2;
                    }
                    temp = ((temp << 3) | (field & 7)) + 1;
                    if (field & 8) {
                        temp = -temp;
                    }
                }
                break;
            }
            }
            if (idx2 == 0) {
                work[ch + 0x45] = temp;
                work[ch + 3] += temp;
            } else {
                work[ch + 3] += work[ch + 0x45] + temp;
                work[ch + 0x45] += temp;
            }
        }
    }
    p = &work[0x36];
    for (idx2 = 0; idx2 < 2; idx2++) {
        s32 col;

        col = 0;
        while (col < 3) {
            x = *p;
            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);
            p++;
            col++;
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
