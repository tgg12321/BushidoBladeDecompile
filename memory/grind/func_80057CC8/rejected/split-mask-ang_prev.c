/* s16 forensics-derived probe: split `ang_prev = X & 0xFFF` into two writes
 * (`ang_prev = X;` then `ang_prev &= 0xFFF;`) to defeat sched1's
 * birthing_insn_p check (which requires reg_n_sets == 1 to boost priority
 * to LAUNCH_PRIORITY = 0x7f000001).
 *
 * MEASURED KILL: sandbox --disable all = 3 (byte-neutral, no gradient).
 * Fresh cc1 -da confirms insn 116 (renamed 115 due to +1 stmt) still gets
 * LAUNCH_PRIORITY 0x7f000001 in the T-5 ready list. cse1/combine folded
 * the two SETs back into one BEFORE sched1's life analysis fixed reg_n_sets
 * at 1. The mechanism is unchanged.
 *
 * This axis therefore CANNOT defeat sched1's insn-115 hoist via source-level
 * re-splitting of the mask assignment. Any pure-C form that keeps `ang_prev`
 * as a single-def local (or an anonymous compiler temp) results in
 * reg_n_sets == 1 at sched1 time and birthing fires.
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

    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy);
    ang_prev &= 0xFFF;
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
