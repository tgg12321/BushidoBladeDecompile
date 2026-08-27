/* REJECTED s40c -- score 56 at 106 insns (measured on the live chassis, sandbox
 * --disable all).  KEEP THIS FILE: it is the FIRST ban-compliant form in 40 sessions
 * whose ENTIRE callee-save map equals the target's -- $s0 cys, $s1 cxs, $s2 arg0,
 * $s3 next-quantity, $s4/$s5 raw cx/cy, $s6 arg2, $s7 arg3 (greg dispositions,
 * tmp/grind/func_80057CC8/s40c/x1.greg).  It therefore DISPROVES the s35 claim that
 * "arg0 cannot reach $s2 ($18) in any ban-compliant form"; the seat is reachable, it
 * is just not affordable in this block order.
 * Construction: the s32 next-test-first layout (rejected/s32-ifblock-swap-next-first-
 * offset-arms-score41.c, arg0 = 4 refs / 53 insns, address pseudo 88 = 3 refs / 18
 * insns) with `scale = arg0[2] * 40;` moved to immediately after the ang_next call.
 * That shortens arg0's live_length 53 -> 38, raising its global.c allocno priority
 * floor_log2(4)*4/38 = 0.2105 above the address's floor_log2(3)*3/18 = 0.1667, so
 * allocno_compare orders arg0 first and find_reg hands it $s2.
 * Why it is still rejected: the next-test-first block order emits the two `lbu 3($s2)`
 * loads in the wrong order relative to the target and lets cse1 fuse them, and the
 * early `scale` displaces the whole post-call multiply chain -- 106 insns and 56
 * mismatches, i.e. the four points the seat map is worth are swamped by forty points
 * of instruction-order cost. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    {
        s32 tmp = arg1 + 1;
        s32 off = (s16) tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
        next_vert = (s16 *)((s32)table + off);
    }

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;
    scale = arg0[2] * 40;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
