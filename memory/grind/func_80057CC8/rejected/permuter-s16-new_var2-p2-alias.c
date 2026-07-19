/* REJECTED — permuter s13 downstream-ternary chassis, 199 iters, base_score=15,
 * one score-0 find at iter ~55s (see tmp/grind/func_80057CC8/s13/perm/output-0-1/).
 *
 * Cheat class: pointer-alias holder (same class as the following already-rejected
 * forms — block-scope-alias-p1.c, permuter-long-new_var2-p1-alias.c,
 * permuter-s16-new_var2-p1-alias.c, ternary-direct-bind-no-local.c). This variant
 * aliases the p2 SET (not p1) via a fresh s16* local `new_var2` whose only purpose
 * is to shift pseudo 86's copy-pref origin away from pseudo 129.
 *
 * Rejection: cheat-by-any-spelling per [[no-new-park-categories]]. The construct
 * has zero semantic purpose — a human programmer would write `p = <expr>;`
 * directly, not `T *new_var2 = <expr>; p = new_var2;`. Same intent as the
 * forbidden dead conditional store: change GCC's RA analysis without appearing in
 * the emitted output (post-DCE `new_var2` disappears; the effect is lifetime
 * shaping of pseudo 86).
 *
 * Also explicitly forbidden by s10 Judge binding constraint: "no shared-pointer
 * split into two source-level locals for func_80057CC8, under any spelling
 * (semantic naming, numeric suffix, block-scope, function-scope, or
 * FAKE-annotated) — the split-`p` family is closed."
 *
 * Novelty (evidence datum): prior permuter finds (s4/s5) aliased p1; s13's find
 * aliases p2. Both sides of the shared-pointer are reachable by the permuter as
 * score-0 basins; both are the same cheat class. This adds one more independent
 * confirmation that the ONLY score-0 close accessible by the permuter over the
 * candidate-baseline structural window is a pointer-alias-holder — no legitimate
 * lever exists in the permuter-reachable neighborhood.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s16 *new_var2;  /* CHEAT: alias-holder for p2 SET; no semantic purpose */
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev, ang_next, ang_mid, scale, base, half;
    u16 cx, cy;
    s16 new_var;
    s16 *p;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16)prev_idx < 0) prev_idx = arg0[3] - 1;
    { s32 tmp = arg1 + 1; next_idx = tmp; if ((s16)tmp >= (s32)arg0[3]) next_idx = 0; }

    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;
    new_var2 = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    p = new_var2;
    ang_next = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }
    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
