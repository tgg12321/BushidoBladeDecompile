/* s44 REJECTED (2026-08-27, structural).  REGIME-B ORDER ATTACK -- KILLED.
 * Derived from tmp/grind/func_80057CC8/s41/d1.c (the Regime-B chassis: next-wrap-if FIRST,
 * prev-wrap-if second, single next-address def in the wrap-merge block, the prev-if branch
 * supplying the def/use separation that makes the address a cross-block allocno).
 *
 * The s43 frontier's item #1 asked for a wrap-first spelling in which the prev-arm's
 * arg0[3] read is NOT dominated by the wrap test, so the target's SECOND `lbu 3($s2)`
 * (asm/funcs/func_80057CC8.s:24 and :31) survives cse1.  This form achieves exactly that
 * by reading the count through a DISTINCT memory reference -- the high byte of the
 * halfword at arg0+2, `(u8)(*(u16 *)(arg0 + 2) >> 8)`, which is the same value on
 * little-endian but a different rtx (mem:HI at +2 vs mem:QI at +3), so cse1 cannot fuse it
 * with the dominating wrap-test read.
 *
 * MEASURED: sandbox func_80057CC8 --disable all -> score 44, build_insns 109
 * (against d1's 45 @ 106 and the candidate's 16 @ 108).  The second count read DOES
 * survive -- and it is worth ONE point.  Regime B does not move.
 *
 * DECISIVE, INDEPENDENT KILL (this session): Regime B's block ORDER is inverted relative
 * to the target.  tmp/grind/func_80057CC8/s44/d1.s emits the NEXT-wrap branch first
 * (`slt $2,$2,$6 / bne $2,$0,.L277`) and the PREV-wrap branch second
 * (`sll $2,$2,16 / bgez $2,.L274`); the target emits the PREV-wrap branch first
 * (asm/funcs/func_80057CC8.s:18-26) and the NEXT-wrap branch second (:30-36).  s42 proved
 * in closed form that Regime B's defining property -- a cross-block next-address allocno
 * with a single def -- REQUIRES the prev-if to sit after the wrap-if (only the prev-if can
 * separate a def that must follow the next-wrap merge from its uses).  So Regime B's
 * register map and the target's block order are mutually exclusive, and no order-level or
 * content-level spelling inside Regime B can reach distance 0.  Regime B is foreclosed on
 * block order alone, independently of the lbu residual this probe recovered.
 */
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
    s32 cnt;
    s32 off;
    s16 *table;
    s16 *next_vert;

    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);
    cnt = arg0[3];

    {
        s32 tmp = arg1 + 1;
        off = tmp * 4;
        if ((s16) tmp >= cnt) {
            off = 0;
        }
    }
    next_vert = (s16 *)((s32)table + off);

    prev_idx = arg1 - 1;
    if ((s16) prev_idx < 0) {
        prev_idx = (u8)(*(u16 *)(arg0 + 2) >> 8) - 1;
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    scale = arg0[2] * 40;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

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
