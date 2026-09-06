/* NEAR-MISS s1 (score 6): u32 atten = base; pitch = _spu_2pitch(atten, ...); clamp
   written into atten in both arms. The andi/li ORDER MATCHES (atten multi-set -> not
   boosted); the else-arm  costs a move + seat change in the tail. Shows
   the receiver-multi-set mechanism works; needs a byte-neutral second set. */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 base;
    u32 pitch;
    u32 atten;

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
    atten = base;
    pitch = _spu_2pitch(atten, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        atten = 0x3FFF;
    } else {
        atten = pitch;
    }
    return atten;
}
