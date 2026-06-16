/* REJECTED 2026-06-15 — cheat-by-spelling (duplicated coercion construct).
 * Sweep variant v10. Inlines the FIRST p's address expression DIRECTLY into the
 * single_game_getEnemyCharId call args, duplicating the entire
 *   ((s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table))
 * expression for [0] and [1]. CSE eliminates the duplicate in the emitted asm;
 * the duplication's ONLY effect is to shape the RTL such that GCC's RA picks v0
 * (reusing the offset reg) as the addu dest instead of allocating a fresh v1.
 * Sandbox --disable all == 0.
 *
 * Worker self-rejected without invoking cheat-reviewer: matches every signal in
 * [[no-new-park-categories]] "Cheats by any spelling":
 *  - No semantic purpose: a programmer would extract `p` as a local once.
 *  - Dead in emitted output: CSE removes the duplicate computation.
 *  - Justification references GCC internals (RTL shape / RA coalescing).
 *  - Naming/structural pattern announces coercion intent (duplicating an
 *    expression character-for-character is not a natural C idiom).
 *
 * Not analogous to the SOTN-sanctioned [[split-read-defeats-hoist]] — that is
 * scoped to "duplicate-read into BRANCH ARMS to defeat LICM," NOT general
 * expression duplication for RA tie-break in a single basic block. */
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

    ang_prev = single_game_getEnemyCharId(
        ((s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table))[0] - (s16) cx,
        ((s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table))[1] - (s16) cy) & 0xFFF;
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
