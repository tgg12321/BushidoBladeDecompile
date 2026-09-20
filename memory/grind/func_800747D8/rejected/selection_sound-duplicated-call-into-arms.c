/* REJECTED (session 5, synthesis) - the DUPLICATED-CALL-INTO-ARMS form.
 * Measured: sandbox --disable all -> score 7, build_insns 211 (target 208).
 * WORSE than the banked floor-6 baseline by 1 point / +3 instructions, but
 * this is the most INFORMATIVE rejection in this ledger so far, because it
 * is the only spelling ever measured on this function that reproduces
 * target's REGISTER SEAT exactly. Read this header before proposing
 * anything else for hunks 17-19.
 *
 *     if (MENU_800747D8->field64 == 0) {
 *         func_8005C650(4, 0x7F, 0x7F);
 *     } else {
 *         func_8005C650(0, 0x7F, 0x7F);
 *     }
 *
 * WHAT IT FIXES.  The banked baseline (one call, one variable reused across
 * load/test/result) emits `lbu a0,0x64(v0)` - the loaded test value and the
 * call argument are the SAME pseudo, so the load lands directly in $a0,
 * which is hunk 17's operand-only mismatch (target: `lbu v0,0x64(v0)`).
 * With the call duplicated into both arms, $a0 is materialized separately in
 * each arm and the test value gets its own pseudo:
 *
 *     ours (this form)              target (asm/funcs/func_800747D8.s:108-118)
 *     lw   v0,0(gp)                 lw    v0,%gp_rel(D_800A36A0)(gp)
 *     lbu  v0,100(v0)   <-- v0 !    lbu   v0,0x64(v0)
 *     bnez v0,L                     beqz  v0,.L80074978
 *      move a0,zero                  addiu a0,zero,4
 *     li   a0,4                     addu  a0,zero,zero
 *     li   a1,127                   addiu a1,zero,0x7F
 *     j    <shared jal>             j     .L80074A20
 *      li  a2,127                    addiu a2,zero,0x7F
 *     L: li a1,127     <-- SURPLUS
 *        j <shared jal> <-- SURPLUS
 *         li a2,127     <-- SURPLUS
 *
 * WHY IT IS +3.  The two arms' identical tails (`li a1,127 / j <jal> /
 * li a2,127`) were never cross-jump-merged with each other.  GCC 2.7.2's
 * jump2 pass tries, for each simplejump, `find_cross_jump (insn,
 * JUMP_LABEL (insn), 1, ...)` FIRST (tools/gcc-2.7.2/jump.c:2005) - a merge
 * against the code physically preceding the jump's own label, with
 * minimum=1 - and only falls back to the sibling-jump search that would
 * pair the two arms with each other (tools/gcc-2.7.2/jump.c:2011-2021) when
 * that first attempt returns newjpos == 0.  Here the first attempt succeeds
 * with a ONE-insn match (the shared `jal func_8005C650` that the field67
 * path falls into), so each arm merges shallowly against the jal and the
 * deeper 3-insn arm-to-arm merge that target shows is never attempted.
 *
 * Four spellings of this same shape were measured, ALL identical at
 * score 7 / build_insns 211 (tmp/grind/func_800747D8/s5/var/):
 *   v1_twocall_swapped.c       (polarity swapped: != 0 plays 0, else plays 4)
 *   v2_twocall_earlygoto_ne.c  (if (!=0) { call(0); goto confirm; } call(4);)
 *   v7_twocall_earlygoto_eq.c  (if (==0) { call(4); goto confirm; } call(0);)
 *   plus the plain if/else above.
 * So the +3 is independent of branch polarity and of whether the arms are
 * written as if/else or as an early-goto - it is the cross-jump-depth
 * property described above, not a layout accident.
 *
 * kill_scope: instance (this chassis, candidate.c floor-6 baseline, zero
 * FAKE constructs in any of the four spellings).
 */
s32 func_800747D8(u32 input) {
    u8 *base;
    s32 sp10;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    S_800747D8 *menu;
    S_800747D8 *work;
    u8 row;

    base = D_800A36A0;
    result = 0;
    if (*(s32 *)(base + 0x10) == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, (s16 *)(base + 0x40), &D_800A35D0);
        switch (ret >> 16) {
    case 1:
        MENU_800747D8->field34 = 0;
        MENU_800747D8->field3C += 1;
        if ((s16)MENU_800747D8->field3C >= 5) {
            MENU_800747D8->field3C = 0;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        MENU_800747D8->field34 = 0;
        MENU_800747D8->field3C -= 1;
        if ((s16)MENU_800747D8->field3C < 0) {
            MENU_800747D8->field3C = 4;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
        }

        state = (s16)MENU_800747D8->field3C;
        switch (state) {
    case 0:
        switch (ret & 0xFF) {
        case 1:
            if (MENU_800747D8->field65 == MENU_800747D8->field64) {
                MENU_800747D8->field65 = 0;
            } else {
                MENU_800747D8->field65 += 1;
            }
            goto selection_sound;
        case 2:
            if (MENU_800747D8->field65 == 0) {
                MENU_800747D8->field65 = MENU_800747D8->field64;
            } else {
                MENU_800747D8->field65 -= 1;
            }
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (MENU_800747D8->field64 == 0) {
            func_8005C650(4, 0x7F, 0x7F);
        } else {
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 1:
        if ((ret & 0xFF) != 0) {
            menu = (S_800747D8 *)D_800A36A0;
            menu->field67 += 1;
            menu = (S_800747D8 *)D_800A36A0;
            menu->field67 &= 1;
            work = (S_800747D8 *)D_800A36A0;
            row = work->field67;
            work->field66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 2:
        if ((ret & 0xFF) != 0) {
            work = (S_800747D8 *)D_800A36A0;
            row = work->field67;
            D_800A35DC += 1;
            work->field66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
confirm:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            MENU_800747D8->field3C = 3;
        }
        break;
    case 3:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            for (i = 0; i < 2; i++) {
                MENU_800747D8->field10.half10[i] = 3;
                MENU_800747D8->field18[i] = 1;
                MENU_800747D8->field38[i] = 0;
            }
        }
        goto tail;
    case 4:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            result = -1;
        }
        goto tail;
        }
tail:
        if (input & 0x100010) {
            func_8005C650(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}
