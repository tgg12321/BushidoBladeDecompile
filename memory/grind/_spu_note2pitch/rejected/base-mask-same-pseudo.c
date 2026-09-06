/* REJECTED s1: u32 base with base &= 0xFFFF before the call. Score 3. Fixes the
   andi/li order (dest multi-set, not sched-boosted) but the mask then reads and writes the
   SAME pseudo: arms sllv/srav a2 + andi a2,a2 ; target has arms in v0 and andi a2,v0.
   Same failure for the u32-arms + u16 round-trip form (base = atten; atten = base;). */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u32 base;
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
    base &= 0xFFFF;
    pitch = _spu_2pitch(base, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
