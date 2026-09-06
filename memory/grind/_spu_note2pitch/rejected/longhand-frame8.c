/* REJECTED s1: longhand copy of the _spu_2pitch loop inside _spu_note2pitch (no inlining).
   Score 21. Frame comes out 8 bytes with the upper-spill at 0(sp); target is 16 bytes with
   the spill at 8(sp) -- the extra 8 bytes come with the integrate.c inlined copy of the
   sibling (A/F forms give 16/8). Also steps was sra (s32 x) here; target srl. */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    s32 x;
    u16 base;
    u32 pitch;
    u32 ratio;
    u32 lower;
    u32 upper;
    s32 i;
    s32 steps;
    u32 frac;

    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = diff;
    if (diff < 0) {
        absdiff = -diff;
    }
    rem = absdiff / 1536;
    oct = rem;
    rem = absdiff - oct * 1536;
    if (diff >= 0) {
        base = 0x1000 << oct;
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        base = 0x1000 >> oct;
    }
    ratio = 0x103B;
    lower = base << 12;
    upper = base * ratio;
    i = 0;
    x = rem;
    if (x < 0) {
        x = -x;
    }
    steps = x >> 5;
    frac = x & 0x1F;
    for (i = 0; i < steps; i++) {
        lower = base * ratio;
        ratio *= 0x103B;
        ratio >>= 12;
        upper = base * ratio;
    }
    pitch = (lower + (((upper - lower) >> 5) * frac)) >> 12;
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
