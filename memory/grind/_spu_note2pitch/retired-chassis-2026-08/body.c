s32 _spu_note2pitch(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    volatile s32 sp0;
    volatile s32 sp4;
    volatile s32 sp8;
    s32 diff;
    s32 abs_diff;
    s32 quot;
    s32 rem;
    s32 base;

    arg0 = ((arg0 & 0xFFFF) << 7) + (arg1 & 0xFFFF);
    arg2 = ((arg2 & 0xFFFF) << 7) + (arg3 & 0xFFFF);
    diff = arg2 - arg0;
    abs_diff = diff;
    if (diff < 0) {
        abs_diff = -diff;
    }

    quot = abs_diff / 1536;
    rem = abs_diff - (quot * 1536);

    if (diff >= 0) {
        base = 0x1000 << quot;
    } else {
        if (rem != 0) {
            quot += 1;
            rem = 0x600 - rem;
        }
        base = 0x1000 >> quot;
    }

    {
        s32 atten = base & 0xFFFF;
        s32 mult_const = 0x103B;
        s32 a3_val = (u32)atten << 12;
        s32 counter = 0;
        s32 abs_rem;
        u32 upper;
        s32 frac;

        abs_rem = rem;
        if (rem < 0) {
            abs_rem = -abs_rem;
        }
        upper = (u32)abs_rem >> 5;
        frac = abs_rem & 0x1F;
        sp8 = atten * mult_const;

        if (upper != 0) {
            do {
                a3_val = atten * mult_const;
                mult_const = ((u32)mult_const * 0x103B) >> 12;
                sp8 = atten * mult_const;
                counter += 1;
            } while (counter < (s32)upper);
        }

        {
            u32 result = (u32)(a3_val + (((u32)(sp8 - a3_val) >> 5) * frac)) >> 12;
            if (result >= 0x4000U) {
                result = 0x3FFF;
            }
            return result & 0xFFFF;
        }
    }
}
