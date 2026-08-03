/* REJECTED (s8, 2026-08-03) — ROUTE B: dead-store-free, promotion fires,
 * but the pseudo is long-lived and the form cannot reach target.
 *
 * The Case3 arm gets its OWN single-set address local `c3`, and `c3` is read
 * from a DIFFERENT basic block (here: the post-loop static-table pointer, as
 * D_8009B7D0 + 0x50 == D_8009B820 -- the static tables are one contiguous
 * block).  That read makes reg_in_basic_block_p() false at loop.c:700, so
 * loop.c cannot hoist the `la D_8009B7D0` out of the Case3 block, while
 * reg_n_sets stays 1 for sched1.  Both preconditions of the LAUNCH_PRIORITY
 * promotion hold and the emitted block IS in target's order
 * (`addu $a1,$zero,$zero` ahead of the la) with NO dead store anywhere.
 *
 * MEASURED (`sandbox func_80060544 --disable all`):
 *   c3 + post-loop read, nothing else ......... 39, build_insns 135
 *       (`c3` is live around the loop back edge -> needs a 10th callee-save;
 *        the function is already saturated at regs=10/0, so `end_off` spills
 *        and the frame grows, vars 48 -> 56.)
 *   THIS FORM (+ `new_var3` computed at its use, which frees the register
 *   and removes the spill) ..................... 11, build_insns 132
 *       Residual: the foreign read replaces target's two-instruction
 *       `lui/addiu` for D_8009B820 with a one-instruction `addiu v0,s6,80`
 *       (hence 132 vs 133), and the `la` lands in the callee-save $s6 where
 *       target has $v0 -- inherent, because a pseudo read outside its block
 *       is live across the loop's calls.
 *   Same lever with the read placed on the second loop's table pointer
 *   (D_8009B7D0 + 0x70 == D_8009B840): identical instruction counts.
 *
 * WHY IT IS DEAD: target's Case3 `la` writes a caller-save register that
 * dies two instructions later.  Any C form that keeps the `la` in the block
 * by giving its destination a foreign read necessarily makes that
 * destination long-lived, so the register identity can never match.  Do not
 * re-probe foreign-read placements; the cost is structural, not positional.
 */
s32 func_80060544(s32 arg0, s32 arg1) {
    s32 geom;
    s32 c3;
    s32 stat;
    s32 last;
    S544 s;
    s32 end_off;
    s32 mid_off;
    s32 i;
    s32 j;
    s32 idx;
    s32 new_var6;
    s32 prev;
    s32 *p0;
    S544 *new_var2;
    s32 *p1;
    int new_var3;
    prev = arg0;
    mid_off = arg0 + 0x4EC;
    end_off = arg0 + 0x5F4;
    s.byte28 = 0;
    s.zero10 = 0;
    s.arg2_field = arg1;
    new_var6 = end_off;
    s.pad20 = 0x200;
    s.pad24 = 0x100;
    s.height = 0;
    s.width = 0;
    i = 0;
    last = 3;
    idx = 0;
    do {
        geom = (s32)(&D_8009B770);
        geom += idx;
        s.p_geom = (s32 *)geom;
        if (i < 3) {
            if (i > 0) {
                goto S800;
            }
            if (i == 0) {
                goto S7D8;
            }
            goto Skip;
        }
        if (i == last) {
            goto Case3;
        }
        goto Skip;
    S7D8:
        stat = (s32)(&D_8009B7D8);
        s.p_static = (s32 *)stat;
        goto Skip;
    S800:
        stat = (s32)(&D_8009B800);
        s.p_static = (s32 *)stat;
        goto Skip;
    Case3:
        c3 = (s32)(&D_8009B7D0);
        s.p_static = (s32 *)c3;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
    Skip:
        if (i != last) {
            s.arg1_field = prev;
            prev = func_8007352C(&s);
        }
        i += 1;
        idx += 0xC;
    } while (i < 4);
    s.p_geom = &D_8009B7A0;
    s.p_static = (s32 *)(c3 + 0x50);
    s.arg1_field = prev;
    new_var2 = &s;
    prev = func_8007352C(new_var2);
    j = 0;
    p1 = &D_8009B840;
    p0 = &D_8009B3B0;
    s.byte29 = 0xFF;
    s.byte2B = 0x10;
    s.byte2A = 0x10;
    s.byte28 = 1;
    do {
        s.p_geom = p0;
        s.p_static = p1;
        s.arg1_field = prev;
        prev = func_8007352C(&s);
        p1 = (s32 *)(((s32)p1) + 8);
        j += 1;
        p0 = (s32 *)(((s32)p0) + 0xC);
    } while (j < 2);
    new_var3 = arg0 + 0x5DC;
    initTexPage(new_var3, 1, 0, saMotionSet((s32)s.p_geom, 0), 0);
    ot_Link(D_800A374C + (arg1 * 4), new_var3);
    return new_var6 - arg0;
}
