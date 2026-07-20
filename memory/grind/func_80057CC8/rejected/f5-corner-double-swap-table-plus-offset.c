/* s20 STRUCTURAL: F5 corner KILLED.
 *
 * The last un-measured cell of the s8 2x2 (p1-swap, p2-swap) operand-order
 * matrix: BOTH p1 and p2 written as (table_expr + offset) instead of the
 * candidate's (offset + table_expr) form.
 *
 * Measurement 2026-07-19 s20 (this file's shape is identical to HEAD src):
 *   sandbox func_80057CC8 --disable all -> score 9  (target_insns=111,
 *   build_insns=111, rules_dropped=7, cheat_asm_stripped=397).
 *
 * s8 recap for the full 2x2 (baseline = candidate.c, offset+table both):
 *   cell (no-swap, no-swap) : 3   (candidate floor)
 *   cell (p1-swap, no-swap) : 3   (byte-neutral; p1 does not steer)
 *   cell (no-swap, p2-swap) : 9   (p2 operand order controls addsi3 reassoc)
 *   cell (p1-swap, p2-swap) : 9   (this file; p2-swap dominates)
 *
 * Verdict: F5 KILLED. Double-swap collapses to the p2-swap outcome; there
 * is no novel addsi3 reassociation behavior exposed by the corner. The p2
 * operand order fully determines the reach of late-reassoc rewrites in this
 * function; p1's operand order is scoring-inert.
 *
 * Unblocks: OWNER-ESCALATION path is one axis (s21 local-alloc block_alloc
 * forensics) away from filing-ready.
 */
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

    /* F5 corner: BOTH p1 and p2 = (table_expr + offset). */
    p = (s16 *)((s32)table + (((s32)(prev_idx << 16) >> 16) << 2));
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((s32)(*(s16 **)(arg0 + 4)) + (((s32)(next_idx << 16) >> 16) << 2));
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
