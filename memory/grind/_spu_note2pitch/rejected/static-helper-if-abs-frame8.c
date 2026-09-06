/* REJECTED s1: static inline helper with the abs INSIDE (if (rem<0) rem=-rem) and the
   loop body duplicated. Score 23. Frame 8/0 (no phantom 8), and the if-form abs is a real
   branch that splits the block, whereas the target's bgez/move/negu is the single abssi2
   insn (mips.md abssi2 template) produced by fold-const.c's (A<0?-A:A) -> ABS_EXPR. */
/* Signed-offset pitch-curve helper used by _spu_note2pitch. The step/lerp
   loop is the same 48th-root-of-two walk as _spu_2pitch (above); this
   variant takes a signed cents offset, normalises its sign, and works on a
   16-bit attenuation base. Only ever inlined (GCC 2.7.2 integrate.c). */
static inline u32 spu_2pitch_abs(u16 atten, s32 rem) {
    u32 ratio = 0x103B;
    u32 lower = atten << 12;
    u32 upper = atten * ratio;
    s32 i = 0;
    s32 steps;
    u32 frac;

    if (rem < 0) {
        rem = -rem;
    }
    steps = (u32)rem >> 5;
    frac = rem & 0x1F;
    for (i = 0; i < steps; i++) {
        lower = atten * ratio;
        ratio *= 0x103B;
        ratio >>= 12;
        upper = atten * ratio;
    }
    return (lower + (((upper - lower) >> 5) * frac)) >> 12;
}

u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 base;
    u32 pitch;

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
    pitch = spu_2pitch_abs(base, rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
