u32 _spu_2pitch(s32 atten, s32 rem) {
    s32 mult_const = 0x103B;
    s32 lower = atten << 12;
    s32 counter = 0;
    s32 steps = (u32)rem >> 5;
    s32 frac = rem & 0x1F;
    s32 upper;

    upper = atten * mult_const;
    while (counter < steps) {
        lower = atten * mult_const;
        mult_const = ((u32)mult_const * 0x103B) >> 12;
        upper = atten * mult_const;
        counter++;
    }
    return (u32)(lower + (((u32)(upper - lower) >> 5) * frac)) >> 12;
}
