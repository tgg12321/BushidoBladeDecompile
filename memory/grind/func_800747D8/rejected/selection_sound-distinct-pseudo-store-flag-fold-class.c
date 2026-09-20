/* REJECTED (session 5, synthesis) - the CLASS result for hunks 17-19.
 * EVERY single-call spelling that selects between the constants 4 and 0 into
 * a pseudo DISTINCT from the loaded test value folds branchless and measures
 * sandbox --disable all -> score 10, build_insns 205 (3 SHORT of target 208).
 * Do not hand-write a thirteenth one.
 *
 * Twelve spellings measured identically at 10/205 across sessions 1, 2 and 5:
 *   s1 H3  two named locals, if/else           (rejected/selection_sound-two-var-split.c)
 *   s1 H4  flag + default 4, override in if    (rejected/selection_sound-default-then-override.c)
 *   s1 H5  no intermediate, field in the if    (rejected/selection_sound-single-shot-no-intermediate.c)
 *   s2     flag + literal, mechanism note      (rejected/selection_sound-flag-then-literal-branchless-fold.c)
 *   s5 v3  s32 sound = 0; if (field64 == 0) sound = 4;
 *   s5 v4  func_8005C650(field64 == 0 ? 4 : 0, ...)   (ternary in the argument)
 *   s5 b1  switch (field64) { case 0: sound = 4; break; default: sound = 0; break; }
 *   s5 b2  s32 sound = 4; if (field64 != 0) sound = 0;  (no intermediate local)
 *   s5 b11 u8 sound (QImode result variable) + if/else
 *   s5 c1  reuse the live local `state` as the result, ==0 arm first
 *   s5 c2  reuse `state`, !=0 arm first
 *   s5 c3  reuse `ret` as the result, ==0 arm first
 *   s5 c4  state = 4; if (field64 != 0) state = 0;
 * (s5 variant sources: tmp/grind/func_800747D8/s5/var/, var2/, var3/)
 *
 * MECHANISM (named, read out of the compiler source, not guessed).
 * GCC 2.7.2's jump.c store-flag transform - tools/gcc-2.7.2/jump.c:1166-1197,
 * the block commented "That didn't work, try a store-flag insn" - rewrites
 * the canonical `x = a; if (...) x = b;` shape into a branchless
 * emit_store_flag sequence.  Its admitting disjunct is jump.c:1190-1191:
 *
 *     && ((reversep = 0, temp2 == const0_rtx)
 *         || (temp3 == const0_rtx
 *             && (reversep = can_reverse_comparison_p (temp4, insn)))
 *
 * i.e. it fires whenever EITHER of the two selected constants is zero - and
 * the pair selected here is {4, 0}, so one of them is always zero no matter
 * which arm holds which value or how the C is spelled.  The emitted fold is
 *     lbu   a0,0x64(v0)
 *     sltiu a0,a0,1
 *     sll   a0,a0,0x2        == (field64 == 0) << 2
 * which is 3 insns shorter than target's real branch.  Reversing the arms
 * only flips which disjunct fires (temp2 vs temp3), which is exactly what
 * the s1/s2/s5 measurements show: same score, same instruction count, every
 * time.  A `switch`, a ternary, a QImode result variable and reuse of an
 * already-live local all reach the same canonical RTL shape and all fold.
 *
 * WHAT ESCAPES IT, and why neither escape is a way out on its own:
 *   (a) The banked floor-6 baseline (candidate.c) reuses ONE variable across
 *       load, test and result.  Its prior value is the load (not a CONST_INT)
 *       and the arm sitting immediately after the conditional jump assigns 4
 *       (not zero), so neither disjunct at jump.c:1190-1191 applies and the
 *       real branch survives - at 208 insns, matching target's count.  The
 *       price is that the test value and the call argument are the SAME
 *       pseudo, so the load lands in $a0 and hunk 17 stays open.
 *   (b) Duplicating the CALL into both arms (rejected/selection_sound-
 *       duplicated-call-into-arms.c) blocks the transform at a different
 *       clause - tools/gcc-2.7.2/jump.c:1066 requires the insn right after
 *       the conditional jump to be followed immediately by the join label
 *       (i.e. the arm must be exactly ONE insn), and a full argument-setup
 *       plus call is five.  That form gets target's register seat right but
 *       costs +3 insns to an unrelated cross-jump-depth limitation.
 *
 * kill_scope: class.  predicate: tools/gcc-2.7.2/jump.c:1190.
 * measured_on: candidate.c floor-6 chassis, src/text1b.c, sandbox
 * --disable all, zero FAKE constructs in any of the twelve spellings.
 *
 * The representative form saved below is s5 b2 (the shortest spelling of the
 * class: default 4, override in the if, field tested directly).
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
        {
        s32 sound = 4;
        if (MENU_800747D8->field64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
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
