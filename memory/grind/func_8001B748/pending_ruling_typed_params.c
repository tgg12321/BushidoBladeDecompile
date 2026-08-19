/* PENDING RULING — DO NOT APPLY WITHOUT AN OWNER/JUDGE RULING.
 * func_8001B748 typed-parameter form. MEASURED 2026-08-19 (s8 synthesis), in place in
 * src/code6cac.c: sandbox --disable all => {"score": 0, "target_insns": 231, "build_insns": 231}.
 * Also score 0 via the s4 standalone scorer (tmp/grind/func_8001B748/s4/score.py).
 *
 * Differs from the BANNED layer-1-FAIL form in that the record type is carried by the FUNCTION
 * SIGNATURE (DST *dst, AB *a, AB *b) and there is not one single cast on dst/a/b anywhere in the
 * body — grep confirms zero occurrences of "(DST *)", "(AB *)" and "u16 *". The banned construct
 * was specifically "typedef ... and casting dst/a/b through them at every access site".
 * Whether that ban extends to an ordinary record-typed signature is the question this session filed.
 *
 * Call sites (src/code6cac.c:1046,1051) are NOT edited and do not need to be: they already pass
 * incompatible pointer types today ((s32 *)&D_800F5328 into a u8 * parameter, and (s32 *)arg2 into
 * an s32 parameter), so GCC 2.7.2 warns and compiles exactly as before. Verified by the in-place
 * sandbox run above, which builds the whole of code6cac.c.
 */
typedef struct AB { s16 h0; s16 h2; s16 h4; s16 h6; s16 h8; } AB;
typedef struct DST { s32 w0; s32 w4; s32 w8; s32 wC; u16 h10; u16 h12; s16 h14; s16 h16;
    s32 w18; u8 b1C; u8 b1D; u8 b1E; u8 b1F; s32 w20; s32 w24; s32 w28; s32 w2C;
    s16 h30; s16 h32; s16 h34; s16 h36; s16 h38; s16 h3A; s16 h3C; s16 h3E; } DST;
void func_8001B748(DST *dst, AB *a, AB *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    u8 *base = (u8 *)&D_80101EC8 + D_800A3748 * 0x44C;
    s32 zval;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 cur;
    s32 use_high;
    s32 v;
    s32 t;
    s32 dd;
    if (dst->b1F == 0) {
        dst->b1F = 1;
        dst->w0 = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
        dst->w4 = (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
        D_800A3310 = 0;
        zval = (frac * (a->h8)) + (inv_frac * (b->h8));
        dst->h12 = val;
        dst->h10 = 0x80;
        dst->h14 = 0;
        dst->w18 = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        dst->w8 = zval >> 12;
        return;
    }
    {
        s32 sum = (*((s32 *) (base + 0x19C))) + (*((s32 *) (base + 0x1A8)));
        s32 avg = ((s32) (sum + (((u32) sum) >> 31))) >> 1;
        if ((avg - (*((s32 *) (base + 0x184)))) < 0xC8) {
            D_800A3310 += 1;
        }
    }
    use_high = ((s16) D_800A3310) >= 0xB;
    cur = dst->w0;
    t = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
    dx = t - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    dst->w0 = cur + (dx >> 4);
    cur = dst->w4;
    t = (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
    dy = t - cur;
    if (dy < 0) {
        dy += 0xF;
    }
    dst->w4 = cur + (dy >> 4);
    cur = dst->w8;
    t = ((frac * (a->h8)) + (inv_frac * (b->h8))) >> 12;
    dz = t - cur;
    if (dz < 0) {
        dz += 0xF;
    }
    dst->w8 = cur + (dz >> 4);
    if (use_high) {
        t = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        t = 0x80 - ((frac_s1 << 8) >> 12);
    }
    dst->h10 = dst->h10 + func_8001A4F0(t - (s16)dst->h10, 0x10);
    v = func_8001A4F0(val - (s16)dst->h12, 0x10);
    dst->h14 = 0;
    dst->h12 = dst->h12 + v;
    if (use_high) {
        t = ((frac_s1 * 0x7D0) + (inv_s1 * 0x2EE0)) >> 12;
    } else {
        t = ((frac_s1 * 0x1F4) + (inv_s1 * 0x2EE0)) >> 12;
    }
    cur = dst->w18;
    dd = t - cur;
    if (dd < 0) {
        dd += 0xF;
    }
    dst->w18 = cur + (dd >> 4);
    dst->h30 = 0x64;
    dst->h32 = 0;
    dst->h34 = 0x64;
    dst->h38 = 0x64;
    dst->h3A = 0;
    dst->h3C = 0x64;
}
