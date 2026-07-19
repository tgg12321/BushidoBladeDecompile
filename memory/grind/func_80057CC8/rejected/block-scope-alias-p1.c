/* REJECTED 2026-06-15 — cheat-reviewer FAIL (cheat-by-spelling: live-range-shaping
 * alias). Block-scoped `s16 *p1` for the FIRST p computation, with function-scope
 * `s16 *p` retained for the SECOND p. Sandbox --disable all == 0, retire == ok
 * (SHA1 == oracle, all 7 regfix rules dropped). REJECTED at layer-1 review.
 *
 * Reviewer evidence (4 of 6 tests failed):
 *  - Test 1 (no semantic purpose): replacing `p1` with function-scope `p` gives
 *    byte-identical observable behavior; `p1` is never read outside its block;
 *    the narrower lexical scope only changes the RA's view of live range.
 *  - Test 2 (human-programmer): a programmer writing from spec would either
 *    use a single `p` (HEAD) or two semantically-named locals (prev_p/next_p),
 *    not `p1` block-scoped + `p` function-scoped.
 *  - Test 3 (GCC-internals justification): worker's stated rationale ("narrows
 *    p1's lifetime to the call") references RA live range explicitly; the
 *    mechanism is allocator behavior, not program semantics.
 *  - Test 6 (naming-announces-intent): `p1` as a suffix-variant of `p` flags
 *    codegen-control naming (p, first variant) rather than a problem-domain name.
 *
 * Sibling rejected forms (same SHA1==oracle close, all cheat-by-spelling):
 *  - rejected/duplicate-address-expr-pseudo-inline.c    (v10 from sweep)
 *  - rejected/split-lh-px0-px1-locals.c                 (v15 from sweep)
 *  - rejected/separate-p1-p2-function-scope-aliases.c   (v17 from sweep)
 *
 * Per [[no-new-park-categories]] / [[review-discipline-before-commit]]: this
 * form is forbidden by any spelling. Do NOT resurrect it; route the search to
 * (a) ALLOCDBG/-da dumps to understand why the first p's offset temp is not
 * coalesced while the second's is; (b) deeper CFG restructure of prev_idx's
 * def chain; or (c) directed permuter on the reassociated baseline. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    {
        s16 *p1 = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
        ang_prev = single_game_getEnemyCharId(p1[0] - (s16) cx, p1[1] - (s16) cy) & 0xFFF;
    }
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

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
