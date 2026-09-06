/* REJECTED (s2, 2026-09-06): rem = absdiff % 1536; oct = absdiff / 1536; (remainder statement first). Score 37 - the divmod pair is not coalesced in the target's order; the quotient must be requested first. */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u32 atten;
    u32 pitch;
    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = (diff < 0) ? -diff : diff;
    rem = absdiff % 1536;
    oct = absdiff / 1536;
    if (diff >= 0) {
        atten = (u16)(0x1000 << oct);
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = (u16)(0x1000 >> oct);
    }
    pitch = _spu_2pitch(atten, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
