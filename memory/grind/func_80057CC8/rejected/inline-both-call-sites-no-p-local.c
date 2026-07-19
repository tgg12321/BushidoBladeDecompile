/* REJECTED 2026-07-19 (s8 rederive) — cheat-by-any-spelling.
 * Sandbox --disable all == 0. target/build_insns=111/111, rules_dropped=7,
 * cheat_asm_stripped=395. Bytes proven this session.
 *
 * FRONTIER AXIS #1 (s7 ledger): inline both call sites' `p` expression
 * entirely (no `p` local declared) so the C-frontend emits a fresh RTL
 * pseudo per call-arg expression, giving p1 and p2 distinct pseudos with
 * independent global-alloc decisions.
 *
 * FORM: the (s16*)(offset+table) address expression is duplicated
 * character-for-character 4 times — once for each of [0]/[1] across both
 * calls. GCC CSE collapses the [0]/[1] pair inside each call-arg list to
 * one temporary, giving 2 distinct pseudos (one per call site). Pseudo
 * lifetime is thereby split: p1's dest allocates independently and can
 * land in v0. p2's dest also allocates independently and lands in v1.
 *
 * WHY REJECTED (identical class to rejected/duplicate-address-expr-pseudo-
 * inline.c, 2026-06-15):
 *  1. No semantic purpose. A programmer writes `p = <expr>; use p[0]; use
 *     p[1];` — that's THE natural form (candidate.c). Writing the 30+
 *     character address expression 4 times is not idiomatic C.
 *  2. Dead in emitted output. GCC CSE removes the duplicates; the ONLY
 *     effect is shaping RTL such that RA picks distinct pseudos.
 *  3. GCC-internals justification. The mechanism is entirely a claim about
 *     RTL pseudo lifetime + global-alloc decisions.
 *  4. Announces coercion intent. Duplicating a compound expression
 *     character-for-character is a coercion signal.
 *
 * Same class as:
 *  - rejected/duplicate-address-expr-pseudo-inline.c (2026-06-15,
 *    partial: inlined FIRST call only; scored 0; rejected)
 *  - rejected/block-scope-alias-p1.c (2026-06-16 layer-1 FAIL,
 *    2026-06-22 layer-2 FAIL)
 *  - rejected/permuter-long-new_var2-p1-alias.c (s4 2026-07-19)
 *  - rejected/permuter-s16-new_var2-p1-alias.c (s5 2026-07-19)
 *
 * This form's ONLY novelty vs the 2026-06-15 rejection is doing it for
 * BOTH calls instead of just the first — a MORE egregious version of the
 * same cheat, not a distinct axis. Frontier hypothesis #1 (inline-
 * expression form) is thereby KILLED with measurement: closes to 0 as
 * expected, and the closing form is cheat-by-any-spelling identical class.
 *
 * NOT COMMITTED. Reverted to candidate.c baseline (floor 3). */
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

    ang_prev = single_game_getEnemyCharId(
        ((s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table))[0] - (s16) cx,
        ((s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table))[1] - (s16) cy) & 0xFFF;
    ang_next = single_game_getEnemyCharId(
        ((s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4))))[0] - (s16) cx,
        ((s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4))))[1] - (s16) cy) & 0xFFF;

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
