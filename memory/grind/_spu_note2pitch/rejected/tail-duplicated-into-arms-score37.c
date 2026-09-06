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
    absdiff = (diff < 0) ? -diff : diff;
    oct = absdiff / 1536;
    rem = absdiff % 1536;
    if (diff >= 0) {
        base = 0x1000 << oct;
        pitch = _spu_2pitch(base, (rem < 0) ? -rem : rem);
        if (pitch >= 0x4000) {
            pitch = 0x3FFF;
        }
        return pitch;
    }
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
    base = 0x1000 >> oct;
    pitch = _spu_2pitch(base, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
