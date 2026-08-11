/* ADAPTED BODY (REGRESSION) - src/main.c - _spu_pitch2note (LIBSPU/S_N2P)
 * Reference: sotn-decomp src/main/psxsdk/libspu/s_n2p.c:56-96 - transplanted verbatim
 * (the reference has no library-specific symbols at all, only arithmetic).
 *
 * MEASURED: 81   (baseline 64).  build 85 / target 74.  WORSE than BB2's current body,
 * and 11 instructions LONGER than the target.
 *
 * VERDICT: DIFFERENT-BUILD. SOTN's revision computes the least-significant-bit scan
 * with a forward `for (i = 0; i < 16; i++) if ((arg2 >> i) & 1) lsb = i;` sweep that
 * keeps the LAST hit. BB2's target does a REVERSE scan over the COMPLEMENT
 * (`search = ~arg2 & 0xFFFF`, shift from 15 downward, stop on the first clear bit),
 * which is a different instruction sequence, not a different spelling of the same one.
 * The divide/modulo chain after the scan does correspond, but it cannot be reached
 * from the reference's scan shape.
 *
 * BB2's existing body is a closer reconstruction of the target - but it is built out
 * of six `register asm("$N")` pins, which are cheat-asm and are why the function is
 * still queued. The honest finding is that neither form is near: the reference is a
 * different library revision, and the pinned reconstruction is not a legitimate C form.
 */

s32 _spu_pitch2note(u16 arg0, u16 arg1, u16 arg2) {
    s32 temp_a0;
    s32 note_high;
    s32 lsb_mask;
    s32 temp_v1;
    s32 lsb;
    s32 note_low;
    s32 i;

    lsb = 0;
    for (i = 0; i < 16; i++) {
        if ((arg2 >> i) & 1) {
            lsb = i;
        }
    }

    lsb_mask = (1 << (lsb + 1)) - (1 << lsb);
    temp_a0 = arg2 - (1 << lsb);

    temp_v1 =
        arg1 +
        ((temp_a0 - (lsb_mask * ((temp_a0 * 12) / lsb_mask)) / 12) * 0x600) /
            lsb_mask;
    note_high = arg0 + ((lsb - 12) * 12) + (temp_a0 * 12) / lsb_mask;

    note_low = temp_v1 % 0x80;
    note_high += (temp_v1 < 0x80) ? 0 : 1;

    if (note_high < 0) {
        return -1;
    }
    if (note_high >= 0x80) {
        return -1;
    }

    return (note_high << 8) | note_low;
}
