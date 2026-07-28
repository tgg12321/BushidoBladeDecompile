/* REJECTED s1: return 0xF - (sum / 0x2000) on the pointer-walk base. vars=0
   (no frame help), rename unflipped, AND it perturbs the LOOP schedule: i++
   hoists above the lw leaving a load-delay nop (even with the load split into
   a named temp). Also emits the in-place division tail (no separate temp reg).
   The explicit-if T0 spelling is strictly better on every axis. */
s32 func_80037AA4(void) {
    s32 i, n, sum;
    s8 *p;
    i = 0; sum = 0; n = D_800A38C8;
    if (n > 0) {
        p = (s8 *)&D_80102810;
        do { sum += *(s32 *)(p + 0x18); i += 1; p += 0x28; } while (i < n);
    }
    return 0xF - (sum / 0x2000);
}
