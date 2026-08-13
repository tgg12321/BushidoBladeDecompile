/*
 * func_80034F88 — best NON-BANNED form, grind session s12b (permuter modality).
 * Honest sandbox (`sandbox func_80034F88 --disable all`): **score 13**,
 *   50 build insns vs 49 target insns, lbu census 176 (all four reloads —
 *   the matched form's exact access signature).
 * NOT installed in src/ (src/code6cac_b.c is left at its committed state).
 *
 * WHY candidate.c CHANGED. The previous session's candidate scored 0 but was
 * FAILed by the layer-1 cheat-reviewer for carrying four textually repeated
 * `u8 *q = &D_80106A73;` declarations, and the driver has since BANNED that
 * construct under BOTH classifications (the pointer-alias-fake-exception
 * family, and "ordinary program logic"). That body is preserved verbatim at
 * rejected/layer1-fail-0813-1524.c; it is not a candidate any more. This file
 * now holds the best form that carries no banned construct.
 *
 * =====================================================================
 * WHAT s12b ESTABLISHED
 * =====================================================================
 * 1. The FOUR separate materialisations of `&D_80106A73` are load-bearing, and
 *    their price is now measured exactly. Holding the s12 byte-local split
 *    fixed and varying only the handle structure (wave K, 8 forms): one handle
 *    23, two 27, three 23, two-with-a-copy 23, one-with-the-mask-on-the-symbol
 *    28, one-with-blocks-2/3-on-the-symbol 28, no handle at all 29 — against 0
 *    for four. The reload census tracks it monotonically (lbu 173 → 176). So
 *    the byte-local split, s12's breakthrough, pays ONLY on a chassis that
 *    already has four address materialisations; it is worth nothing alone.
 *
 * 2. A `static inline` helper is the first structurally different route to
 *    those four materialisations ever found. One declaration of the handle,
 *    inside a helper called three times, gives three independent pointer
 *    pseudos plus the caller's mask handle — the four-materialisation shape,
 *    reached by ordinary program factoring instead of by repeating a
 *    declaration. Precedent for `static inline` in this tree: src/main.c:2396
 *    (`_memcpy`). Control: the same helper without `inline` emits a real `jal`
 *    and scores 35, so the inlining is what creates the pseudos.
 *
 * 3. It stops at 13 because the inliner substitutes the CONSTANT address at
 *    each use inside the copied body (integrate.c `copy_rtx_and_substitute`
 *    and its const-equivalence map), leaving every mem with a single-use
 *    address that `combine` then folds `%lo` into. The build emits
 *    `lui a0; lbu a0,0(a0)` and `lui at; sb v0,0(at)` where the target has one
 *    shared unfolded `lui`/`addiu` base reused by both the reload and the
 *    store. Passing the address as an argument does not escape it (22).
 *    Closing that folding is s13's F1 — see hypotheses.md.
 *
 * Ties at 13: this form, the same with the mask handle at caller function
 * scope, and the same with the helper taking `p` and computing its own
 * condition. `u8` instead of `s32` for the helper's byte local costs 3 (16);
 * factoring the mask into a second inline helper costs 13 (26).
 */
static inline void bb2_set_flag(s32 c, s32 bit) {
    u8 *q = &D_80106A73;
    s32 v;

    v = *q;
    if (c) {
        c = v | bit;
    } else {
        c = v;
    }
    *q = c;
}

void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    {
        u8 *q = &D_80106A73;

        *q &= 0xF8;
    }

    bb2_set_flag(p[8] & 1, 1);
    bb2_set_flag(p[8] & 2, 2);
    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
