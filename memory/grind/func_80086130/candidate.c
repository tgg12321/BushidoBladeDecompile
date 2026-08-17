/* func_80086130 — src/main.c — MATCHED (session 1, 2026-08-17)
 *
 * Honest pure-C sandbox distance 0 (--disable all), 0 regfix/asmfix rules,
 * 0 register pins, 0 inline asm. Full build SHA1 == oracle
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (verified twice this session).
 *
 * Semantics: a libsnd-style per-voice stereo volume setter. Rejects voice
 * indices >= 24 with -1. Scales each channel by 129 (the 0..127 -> 0..16383
 * percent-to-raw conversion; GCC synthesises it as `(v << 7) + v`), writes the
 * right channel to the +2 halfword and the left channel to the +0 halfword of
 * the 16-byte-strided voice-attribute table at D_80102A78, then sets the two
 * low dirty bits in the per-voice flag byte D_800F65E0[idx].
 *
 * Provenance: the immediately preceding function in the same file,
 * func_80086014 (src/main.c:967), is an already-matched sibling with exactly
 * this control flow, exactly this table layout and exactly this flag update —
 * it differs only in that it stores the raw values instead of the *129 scaled
 * ones. This body is that sibling's C, verbatim in shape, with the scaling
 * added through two named locals.
 *
 * Why the locals are `s16` and not `s32` (the one non-obvious point, measured):
 * the destination array elements are `s16`, so `s16` is the natural type for a
 * value that exists only to be stored into one. It is also the only spelling
 * that reproduces the target's evaluation order. Measured sandbox distances for
 * the full variant grid this session (tmp/grind/func_80086130/s1/sweep.py,
 * results in sweep_results.txt):
 *     s16 intermediates .....................  1  (-> 0, see below)
 *     inline `y * 129` at the store sites ... 21
 *     s32 intermediates ..................... 25
 *     s32 intermediates, vy declared first .. 25
 *     s32 vx named, y inlined ............... 21
 *     named `s32 i = idx * 8` index ......... 21
 *     s32 params with (s16) casts ........... 21
 *     `s16 *p = &D_80102A78[idx*8]` walk .... 24
 *     `(y << 7) + y` spelling ............... 23
 *     `D_80102A7A[idx*8]` for the +2 store .. 22
 *     flag update hoisted above the stores .. 24
 *     inverted early-return guard ........... 26
 * With `s32` locals GCC's sched1 leaves the x-channel multiply half-formed
 * across the stores and fills the `lbu` load-delay slot with `addu v1,v1,a0`,
 * which pushes the `return 0` materialisation to the top of the block; the
 * `s16` locals complete both multiplies before the address arithmetic, leaving
 * `move v0,zero` as the only instruction available for that delay slot —
 * exactly where the target has it.
 *
 * The last unit of sandbox distance (1 -> 0) was a scoring artifact, not a byte
 * difference: the pre-existing reference object spelled the +2 store as
 * `sh v1,%lo(D_80102A7A)($at)` while this C spells it `sh v1,%lo(D_80102A78)+2`.
 * Both have %hi 0x1010 and both resolve to 0x80102A7A, so the linked words are
 * identical; engine/score.py deliberately does not mask named-symbol LO16
 * addends, so it reported them as differing. The matched sibling func_80086014
 * uses the same `[idx * 8 + 1]` spelling. The full-build SHA1 is the proof.
 *
 * Rewriting to `D_80102A7A[idx * 8]` to silence that artifact is actively
 * WRONG: two distinct array symbols let GCC merge the address computations,
 * dropping the function to 32 instructions and distance 22.
 */

extern s16 D_80102A78[];
extern u8 D_800F65E0[];

s32 func_80086130(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        s16 vx = x * 129;
        s16 vy = y * 129;

        D_80102A78[idx * 8 + 1] = vy;
        D_80102A78[idx * 8] = vx;
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
