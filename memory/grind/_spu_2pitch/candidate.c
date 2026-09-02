/* Pitch interpolation helper: scales `atten` (12.12 fixed) by the 48th-root-
   of-two step 0x103B/0x1000 once per 32 cents (rem >> 5), then linearly
   interpolates the remaining 0..31 cents between the two adjacent steps.
   COMPLETED-C 2026-09-02 (grinder s1, pure C, no FAKE constructs). */
u32 _spu_2pitch(u32 atten, u32 rem) {
    u32 ratio = 0x103B;
    u32 lower = atten << 12;
    s32 i;
    s32 steps = rem >> 5;
    u32 frac = rem & 0x1F;
    u32 upper;

    upper = atten * ratio;
    for (i = 0; i < steps; i++) {
        lower = atten * ratio;
        ratio *= 0x103B;
        ratio >>= 12;
        upper = atten * ratio;
    }
    return (lower + (((upper - lower) >> 5) * frac)) >> 12;
}
