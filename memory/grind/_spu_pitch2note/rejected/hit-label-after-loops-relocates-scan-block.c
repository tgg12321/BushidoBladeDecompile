s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch) {
    u16 search;
    s32 bit;
    s32 shift;
    s32 oct;
    s32 scale;
    u32 curve;
    u32 target;
    u32 lower;
    u32 upper;
    u32 step;
    u32 acc;
    u32 next;
    u32 lo;
    u32 hi;
    s32 outer;
    s32 inner;
    s32 base;
    s32 result;
    s32 quot;
    s32 rem;
    s32 note;
    s32 fine;

    search = ~pitch;
    bit = 0;
    for (shift = 15; shift >= 0; shift--) {
        if (!((search >> shift) & 1)) {
            bit = shift;
            break;
        }
    }
    oct = bit - 12;
    scale = 1 << bit;
    curve = 0x1000;
    target = pitch;
    for (outer = 0; outer < 0x30; outer++) {
        lower = scale * curve;
        curve = (curve * 0x103B) >> 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        base = outer << 5;
        acc = 0;
        next = step;
        for (inner = 0; inner < 0x20; inner++) {
            lo = (lower + acc) >> 12;
            hi = (lower + next) >> 12;
            if (target >= lo && target < hi) {
                goto hit;
            }
            acc += step;
            next += step;
        }
    }
    result = 0x600;
    goto found;
hit:
    result = base + inner;
found:
    quot = result / 128;
    rem = result % 128;
    note = cen_note + quot + oct * 12;
    fine = cen_fine + rem;
    return (note << 8) | fine;
}
