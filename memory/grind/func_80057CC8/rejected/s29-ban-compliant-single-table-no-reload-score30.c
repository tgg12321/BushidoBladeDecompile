/* REJECTED s29 2026-08-20 — the ONLY Judge-ban-compliant form in the bank, and it
 * is STRUCTURALLY short: sandbox --disable all = 30, target_insns 111 vs
 * build_insns 112. One `table` local, loaded once, both ratan2 call sites indexed
 * off it (table[pi*2] / table[ni*2]); no second load, no second pointer local, no
 * reassignment. Because the base stays live across the intervening ratan2 call it
 * is materialized into a callee-save the target does not use — the target instead
 * RELOADS the base after the call (asm/funcs/func_80057CC8.s:17 lw $a2,0x4($s2);
 * :50 lw $a0,0x4($s2)). 112 insns cannot become 111 by allocation alone, so this
 * form is not a floor-3 near-miss: it is unmatchable in kind.
 * This is the measurement that closes the function: every form reaching <= 6
 * requires a second source-level materialization of the base, which is exactly the
 * family the owner refused 2026-07-20 and layer-1 FAILed three times 2026-08-20.
 * Do NOT re-derive.
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
    s32 pi;
    s32 ni;
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

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ni = (s16) next_idx;
    ang_next = ratan2(table[ni * 2] - (s16) cx, table[ni * 2 + 1] - (s16) cy) & 0xFFF;

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
