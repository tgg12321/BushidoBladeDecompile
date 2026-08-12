/* REJECTED — func_800645B0.  Banked by the session-6 (forensics) grind.
 *
 * WHY IT IS DEAD (measured, sweep19 — tmp/grind/func_800645B0/s5/sweep19.py):
 * The three word-stride destinations (D_800F0D78 / D_800F0D7C / videoDec) and
 * the halfword destination (D_800F0BCC) are separate splat symbols, so spelling
 * the stores as array subscripts on a cast base — `((s32 *)(&D_800F0D78))[wid]`
 * — makes GCC emit the scaling and the symbol address separately instead of
 * folding the constant symbol into the computed byte offset.  Every subscript
 * spelling regresses hard, and dropping the named halfword index (letting it
 * become an anonymous CSE temp) costs two more instructions because the
 * `idx << 1` value stops being shared between the *3 sum and the s16 store.
 *
 *   CA control (named idx2, hand-built byte offsets)      3 / 78
 *   SB control                                            1 / 78
 *   TA  no named idx2, sum `idx * 3`, s16 subscripted    14 / 80
 *   TB  TA + word stores subscripted                     44 / 85
 *   TC  CA + word stores subscripted                     36 / 82
 *   TD  TC + s16 subscripted                             44 / 85
 *   UA  SB chassis, no named idx2, s16 subscripted       25 / 83
 *
 * Conclusion for future sessions: the hand-built `*(s32 *)((s32)&SYM + off)`
 * spelling with a NAMED halfword index is not a stylistic choice on this
 * function, it is load-bearing, and the named `idx2` local must stay.
 *
 * The body below is variant TA, the cheapest of the family (14 / 80).
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                wid = idx * 3;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                ((s16 *)(&D_800F0BCC))[idx] = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
