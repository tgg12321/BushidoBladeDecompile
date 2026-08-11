/* func_8007C86C — WIP candidate body (sandbox --disable all == 12, HEAD == 20).
 *
 * Identical structural shape to memory/wip/func_8007C7A0/candidate.c; only
 * the GPU command constant differs (0xE4000000 vs 0xE3000000). Both
 * functions are siblings with identical 21-rule patterns.
 *
 * Apply to src/display.c (replace the existing func_8007C86C body) to resume
 * from the score-12 floor. The candidate does NOT close the function — same
 * $a2-vs-$a3 X-preserve cascade as C7A0, and the 21 existing rules
 * (regfix.txt:3073-3094) are NOT retired by this form.
 *
 * Verify after applying:
 *   & tools/eng.ps1 sandbox func_8007C86C --disable all   # expect "score": 12
 *
 * See memory/wip/func_8007C86C/notes.md for the lever explanation and
 * memory/wip/func_8007C86C/meta.json for sessions[] / next_hypotheses.
 *
 * DO NOT introduce a `raw_arg0 = arg0` literal-rename chain (same form
 * forbidden on the sibling — see memory/wip/func_8007C7A0/meta.json
 * rejected_forms).
 */

s32 func_8007C86C(s16 arg0, s16 arg1)
{
    s16 var_v0_2;
    s16 var_a1;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;                                /* session 2 preload (clean v8 base) */
    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
    }
    if (var_a1 >= 0) {
        if ((D_8009BE7A - 1) < var_a1) {
            var_a1 = D_8009BE7A - 1;
        }
    } else {
        var_a1 = 0;
    }
    var_v1 = var_a1 & 0xFFF;                      /* SOTN duplicate-read: precompute Y wide-mask */
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        var_v1 = var_a1 & 0x3FF;                  /* overwrites in narrow-mode branch */
        var_v1 = var_v1 << 0xA;
        var_v0 = var_v0_2 & 0x3FF;
    } else {
        var_v1 = var_v1 << 0xC;                   /* uses the precomputed wide-mask */
        var_v0 = var_v0_2 & 0xFFF;
    }
    {
        s32 r_e4 = var_v0 | 0xE4000000;
        return var_v1 | r_e4;
    }
}
