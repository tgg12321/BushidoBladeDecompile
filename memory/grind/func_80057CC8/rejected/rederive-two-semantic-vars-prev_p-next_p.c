/* PROVISIONALLY REJECTED 2026-07-19 (s8 rederive) — pending ruling.
 * Sandbox --disable all == 0. target/build_insns=111/111, rules_dropped=7,
 * cheat_asm_stripped=395. Bytes proven this session.
 *
 * REDERIVE-MODALITY EVIDENCE:
 *  - Fresh m2c on asm/funcs/func_80057CC8.s (tools/m2c/m2c.py --valid-syntax)
 *    INDEPENDENTLY produces TWO distinct pointer temps (temp_v0_3, temp_v1_2)
 *    for the prev/next call sites. Structural evidence that the original
 *    source used two variables (or two anonymous expressions), not one
 *    reused `p`.
 *  - Ledger s6/s7 forensics: pseudo 86 (the C-source `s16 *p`) is ONE
 *    global-alloc allocno; its copy-preference {3} unconditionally steers
 *    it to v1. No pure-C form that keeps `p` as ONE C variable can reach
 *    target's p1=v0 / p2=v1 asymmetric allocation. Two distinct C
 *    variables = two RTL pseudos = two independent RA decisions.
 *
 * RELATION TO PRIOR REJECTIONS:
 *  - rejected/separate-p1-p2-function-scope-aliases.c (2026-06-15, sweep
 *    variant v17) used names `p1`, `p2`. Cheat-reviewer rejection cited
 *    (a) semantically equivalent to reused `p` (would-a-programmer test),
 *    (b) "numeric-suffix variant announces codegen-role distinction",
 *    (c) explicitly suggested `prev_p` / `next_p` as the semantic naming.
 *  - This form uses exactly the names the prior rejection suggested:
 *    `prev_p` (pointer to previous arena entry), `next_p` (pointer to
 *    next arena entry). These are problem-domain names — the function
 *    computes the angle from the current point to the previous and next
 *    arena points, then interpolates. A human programmer writing that
 *    would naturally use those names.
 *
 * WHY THIS NEEDS A RULING (not a self-classification):
 *  - Mechanism identical to rejected class: two pointer locals whose
 *    SOLE codegen effect is producing two RTL pseudos vs one allocno.
 *  - Semantic-naming criticism (point (c) of prior rejection) is
 *    addressed but not eliminated: cheats-by-any-spelling policy holds
 *    that mechanism, not names, is the standard.
 *  - Countervailing: fresh m2c independently converges on two temps;
 *    the "same behavior with one vs two variables" test cuts BOTH
 *    ways (a programmer choosing between `p = ...; use p;` twice vs
 *    `prev_p = ...; use prev_p; next_p = ...; use next_p;` faces a
 *    stylistic choice, not a codegen-driven one, when writing from
 *    a specification of the function's behaviour).
 *  - Owner ruling needed on: does the semantic-naming resolution +
 *    m2c-corroborated original-source evidence reclassify this from
 *    cheat to legitimate pure-C form? Or does cheats-by-any-spelling
 *    hold regardless of naming/evidence?
 *
 * NOT COMMITTED. src/text1b.c reverted to candidate.c baseline (floor 3). */
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
    s16 *prev_p;
    s16 *next_p;
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

    prev_p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(prev_p[0] - (s16) cx, prev_p[1] - (s16) cy) & 0xFFF;
    next_p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(next_p[0] - (s16) cx, next_p[1] - (s16) cy) & 0xFFF;

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
