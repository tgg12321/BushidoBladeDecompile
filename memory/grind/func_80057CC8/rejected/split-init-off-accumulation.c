/* REJECTED by cheat-reviewer (layer 1), 2026-06-16
 * Decision: FAIL
 * Tests failed: 1 (no semantic purpose for off split), 2 (human-programmer
 * test - one-liner is the natural form), 3 (GCC-internals justification -
 * "off += forces two-address addu"), 5 (family check - 5th spelling of same
 * RA-coalescing coercion as 4 prior rejected forms).
 * retire() confirmed SHA1==oracle for this form (byte match proven).
 * The gap: `addu v0,v0,a2` at pos 35 requires coalescing the offset temp
 * into the destination, but GCC emits `addu v1,v0,a2` (three-address) for
 * the one-liner form. The split `off=...; off+=table` forces two-address but
 * that coercion is precisely what makes it a cheat.
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
    s32 off;

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

    off = (((s32)(prev_idx << 16) >> 16) << 2);
    off += (s32)table;
    p = (s16 *)off;
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
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
