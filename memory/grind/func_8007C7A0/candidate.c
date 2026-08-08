/* func_8007C7A0 — WIP candidate body (sandbox --disable all == 12, HEAD == 16).
 *
 * Apply to src/display.c (replace the existing func_8007C7A0 body) to resume
 * from the score-12 floor. The candidate does NOT close the function — 12
 * register-rotation diffs remain (the $a2-vs-$a3 X-preserve register
 * tiebreaker), and the 21 existing regfix substs (regfix.txt:3050-3071) are
 * NOT retired by this form.
 *
 * Verify after applying:
 *   & tools/eng.ps1 sandbox func_8007C7A0 --disable all   # expect "score": 12
 *
 * See memory/wip/func_8007C7A0/notes.md for the lever explanation and
 * memory/wip/func_8007C7A0/meta.json for sessions[] / next_hypotheses /
 * rejected forms.
 *
 * DO NOT introduce a `raw_arg0 = arg0` literal-rename chain (drops to 10 but
 * is FORBIDDEN per .claude/rules/param-local-alias-prologue-pair-flip.md +
 * .claude/rules/no-new-park-categories.md "cheats by any spelling").
 */

s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 var_v0_2;
    s16 var_a1;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;                                /* session B preload (unconditional) */
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
        s32 r_e3 = var_v0 | 0xE3000000;
        return var_v1 | r_e3;
    }
}
