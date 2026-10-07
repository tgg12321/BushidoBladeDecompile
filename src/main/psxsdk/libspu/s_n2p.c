/* PsyQ 4.0 LIBSPU S_N2P: _spu_2pitch, _spu_note2pitch and _spu_pitch2note.
 * .text 0x8008BA94..0x8008BD88, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* Pitch interpolation helper: scales `atten` (12.12 fixed) by the 48th-root-
   of-two step 0x103B/0x1000 once per 32 cents (rem >> 5), then linearly
   interpolates the remaining 0..31 cents between the two adjacent steps. */
inline u32 _spu_2pitch(u32 atten, u32 rem) {
    u32 ratio = 0x103B;
    u32 lower = atten << 12;
    s32 i;
    s32 steps = rem >> 5;
    u32 frac = rem & 0x1F;
    u32 upper;

    upper = atten * ratio;
    for (i = 0; i < steps; i++) {
        lower = atten * ratio;
        ratio *= 0x103B;
        ratio >>= 12;
        upper = atten * ratio;
    }
    return (lower + (((upper - lower) >> 5) * frac)) >> 12;
}

/* Pitch of note/fine relative to the centre note/fine (128 fine steps per
 * semitone, 0x600 per octave): 0x1000 shifted by the whole octaves, then the
 * remaining steps walked through the inlined _spu_2pitch curve; clamped to
 * 0x3FFF. (_spu_2pitch is GNU89 `inline`: integrated here, and still emitted
 * out of line.) */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 atten;
    u32 pitch;
    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = (diff < 0) ? -diff : diff;
    oct = absdiff / 1536;
    rem = absdiff % 1536;
    if (diff >= 0) {
        atten = 0x1000 << oct;
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = 0x1000 >> oct;
    }
    /* FAKE: staging the attenuation through the dead `diff` puts the
       widening `andi` before the inlinee's `li 0x103B`; a fresh local or no
       staging reverses the pair: score 2 (staged-value-reused-variable) */
    diff = atten;
    pitch = _spu_2pitch(diff, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}

s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch) {
    u16 search;
    s32 bit;
    s32 oct;
    s32 scale;
    u32 curve;
    u32 lower;
    u32 upper;
    u32 step;
    u32 acc;
    u32 next;
    u32 lo;
    u32 hi;
    s32 base;
    s32 i;
    s32 inner;
    s32 result;
    s32 quot;
    s32 rem;
    s32 note;
    s32 fine;

    search = ~pitch;
    bit = 0;
    for (i = 15; i >= 0; i--) {
        if (!((search >> i) & 1)) {
            bit = i;
            break;
        }
    }
    oct = bit - 12;
    scale = 1 << bit;
    curve = 0x1000;
    for (i = 0; i < 0x30; i++) {
        lower = scale * curve;
        curve *= 0x103B;
        curve >>= 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        for (inner = 0, base = i * 32, acc = 0, next = step; inner < 0x20;
             inner++) {
            lo = lower + acc;
            hi = lower + next;
            lo >>= 12;
            hi >>= 12;
            if (pitch >= lo && pitch < hi) {
                result = base + inner;
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
