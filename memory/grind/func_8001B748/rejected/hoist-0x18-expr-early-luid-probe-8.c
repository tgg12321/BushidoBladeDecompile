void func_8001B748(u8 *dst, u8 *a, u8 *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    int new_var;
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
        t = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        dst[0x1F] = 1;
        *((s32 *) (dst + 0)) = ((frac * (*((s16 *) (a + 4)))) + (inv_frac * (*((s16 *) (b + 4))))) >> 12;
        *((s32 *) (dst + 4)) = (((frac * (*((s16 *) (a + 6)))) + (inv_frac * (*((s16 *) (b + 6))))) >> 12) - 0x12C;
        new_var = (frac * (*((s16 *) (a + 8)))) + (inv_frac * (*((s16 *) (b + 8))));
        D_800A3310 = 0;
        *((s16 *) (dst + 0x12)) = val;
        *((s16 *) (dst + 0x10)) = 0x80;
        *((s16 *) (dst + 0x14)) = 0;
        *((s32 *) (dst + 0x18)) = t;
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



/* REJECTED (s4): sched2-LUID-derived probe. Hoisting ((frac_s1*0x9C4)+(inv_s1*0x2710))>>12
   into a named local earlier in the early-exit block was meant to raise
   INSN_LUID(116, the D_800A3310 store) above INSN_LUID(140, the last synth-chain
   insn) in sched2's input stream, flipping the sched.c:2461 LUID tie-break.
   Measured: t-at-top=8, t-before-gp-store=8, t-after-gp-store=8, dd=18, cur=12, dx=8
   (all @ 231 insns). sched1 re-sinks the chain below the stores regardless of where
   the source computes it, so .greg order (and therefore sched2's LUIDs) is unchanged;
   the gp store still diverges identically AND the hoist damages the dst+0x10/0x12
   region (li v0,128 / mult a3,a0 / sh v0,16(s0) reorder). KILLED. */
