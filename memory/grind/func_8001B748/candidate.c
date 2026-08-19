/* candidate.c - func_8001B748 - BEST IN-SCOPE FORM, honest sandbox score 2 (231/231 insns).
 * Re-measured in place in src/code6cac.c on 2026-08-19 by the s8 SYNTHESIS session: score 2
 * (231/231). That session also independently reproduced the score-0 patch below: sandbox 0
 * and a full-build SHA1 MATCH. Raw log: tmp/grind/func_8001B748/s8/measurements.txt.
 * NOTE: src/code6cac.c at HEAD is NOT this body - HEAD measures 32. Splice this file in before
 * you measure anything.
 *
 * This is the s7 floor-2 body. It carries NO struct/record declaration and NO banned construct.
 *
 * THE SCORE-0 FORM IS SOLVED AND BANKED, but it is out of a grind session's scope:
 * memory/grind/func_8001B748/aggregate_merge_score0.patch reaches score 0 (231/231) with a
 * full-build SHA1 MATCH, using the frozen "per-word splat symbol -> aggregate merge" family
 * with all five prongs measured (see evidence.md FINDINGS 2-5 and the 2026-08-19 INTEGRATION
 * HANDOFF entry in docs/grind/decisions.md). It touches include/code6cac.h,
 * src/code6cac_c2.c and src/text1b.c, which tools/grinder/scope_allow.txt does not yet allow
 * for this function. Do NOT try to respell it TU-locally - that spelling is BANNED
 * (rejected/layer1-fail-struct-cast-at-use-score0-BANNED.c and the 2026-08-19 04:03 ruling).
 */
void func_8001B748(u8 *dst, u8 *a, u8 *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    int new_var;
    s32 pa;
    s32 dx;
    s32 dy;
    u8 *base = (u8 *)&D_80101EC8 + D_800A3748 * 0x44C;
    s32 dz;
    s32 cur;
    s32 use_high;
    s32 v;
    s32 t;
    s32 dd;
    if (dst[0x1F] == 0) {
        dst[0x1F] = 1;
        *((s32 *) (dst + 0)) = ((frac * (*((s16 *) (a + 4)))) + (inv_frac * (*((s16 *) (b + 4))))) >> 12;
        *((s32 *) (dst + 4)) = (((frac * (*((s16 *) (a + 6)))) + (inv_frac * (*((s16 *) (b + 6))))) >> 12) - 0x12C;
        pa = frac * (*((s16 *) (a + 8)));
        D_800A3310 = 0;
        new_var = pa + (inv_frac * (*((s16 *) (b + 8))));
        *((s16 *) (dst + 0x12)) = val;
        *((s16 *) (dst + 0x10)) = 0x80;
        *((s16 *) (dst + 0x14)) = 0;
        *((s32 *) (dst + 0x18)) = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        *((s32 *) (dst + 8)) = new_var >> 12;
        return;
    }
    {
        s32 sum = (*((s32 *) (base + 0x19C))) + (*((s32 *) (base + (0x1A8 & 0xFFFFFFFF))));
        s32 avg = ((s32) (sum + (((u32) sum) >> 31))) >> 1;
        if ((avg - (*((s32 *) (base + 0x184)))) < 0xC8) {
            D_800A3310 += 1;
        }
    }
    use_high = ((s16) D_800A3310) >= 0xB;
    cur = *((s32 *) (dst + 0));
    t = ((frac * (*((s16 *) (a + 4)))) + (inv_frac * (*((s16 *) (b + 4))))) >> 12;
    dx = t - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    *((s32 *) (dst + 0)) = cur + (dx >> 4);
    cur = *((s32 *) (dst + 4));
    t = (((frac * (*((s16 *) (a + 6)))) + (inv_frac * (*((s16 *) (b + 6))))) >> 12) - 0x12C;
    dy = t - cur;
    if (dy < 0) {
        dy += 0xF;
    }
    *((s32 *) (dst + 4)) = cur + (dy >> 4);
    cur = *((s32 *) (dst + 8));
    t = ((frac * (*((s16 *) (a + 8)))) + (inv_frac * (*((s16 *) (b + 8))))) >> 12;
    dz = t - cur;
    if (dz < 0) {
        dz += 0xF;
    }
    *((s32 *) (dst + 8)) = cur + (dz >> 4);
    if (use_high) {
        t = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        t = 0x80 - ((frac_s1 << 8) >> 12);
    }
    *((s16 *) (dst + 0x10)) = (*((u16 *) (dst + 0x10))) + func_8001A4F0(t - (*((s16 *) (dst + 0x10))), 0x10);
    v = func_8001A4F0(val - (*((s16 *) (dst + 0x12))), 0x10);
    *((s16 *) (dst + 0x14)) = 0;
    *((s16 *) (dst + 0x12)) = (*((u16 *) (dst + 0x12))) + v;
    if (use_high) {
        t = ((frac_s1 * 0x7D0) + (inv_s1 * 0x2EE0)) >> 12;
    } else {
        t = ((frac_s1 * 0x1F4) + (inv_s1 * 0x2EE0)) >> 12;
    }
    cur = *((s32 *) (dst + 0x18));
    dd = t - cur;
    if (dd < 0) {
        dd += 0xF;
    }
    *((s32 *) (dst + 0x18)) = cur + (dd >> 4);
    *((s16 *) (dst + 0x30)) = 0x64;
    *((s16 *) (dst + 0x32)) = 0;
    *((s16 *) (dst + 0x34)) = 0x64;
    *((s16 *) (dst + 0x38)) = 0x64;
    *((s16 *) (dst + 0x3A)) = 0;
    *((s16 *) (dst + 0x3C)) = 0x64;
}
