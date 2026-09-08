/* CANDIDATE - _spu_pitch2note (src/main.c)  sandbox --disable all = 36  (s1, 2026-09-08)
 * Pure C, no FAKE constructs. Epilogue and prologue match; residual = the
 * inner-loop back-edge delay slot (label between the increments and inner++,
 * see hypotheses.md F1) plus a register-seat cascade. Prototype
 * s32 (u16, u16, u16); no header prototype exists and no in-EXE caller. */
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
    scale = 1;
    scale <<= bit;
    curve = 0x1000;
    target = pitch;
    for (outer = 0; outer < 0x30; outer++) {
        lower = scale * curve;
        curve *= 0x103B;
        curve >>= 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        acc = 0;
        next = step;
        for (inner = 0; inner < 0x20; inner++) {
            lo = (lower + acc) >> 12;
            hi = (lower + next) >> 12;
            if (target >= lo && target < hi) {
                result = (outer << 5) + inner;
                goto found;
            }
            next += step;
            acc += step;
        }
    }
    result = 0x600;
found:
    quot = result / 128;
    rem = result % 128;
    note = cen_note + quot + oct * 12;
    fine = cen_fine + rem;
    return (note << 8) | fine;
}
