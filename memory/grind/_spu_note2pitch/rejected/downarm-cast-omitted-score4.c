/* REJECTED (s2, 2026-09-06): Only the up-shift arm narrows to u16 (the down-shift arm cannot overflow u16 so the cast looks redundant). Score 4: with one arm untruncated there is no identical tail for jump2 to cross-jump, so the andi does not land after the join label. The target's single post-join andi applies to BOTH arms' values -> the original narrowed in both branches. */
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
    rem = absdiff / 1536;
    oct = rem;
    rem = absdiff - oct * 1536;
    if (diff >= 0) {
        atten = (u16)(0x1000 << oct);
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = 0x1000 >> oct;
    }
    pitch = _spu_2pitch(atten, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
