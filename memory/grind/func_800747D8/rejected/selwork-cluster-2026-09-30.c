/* func_800747D8 - s10 (forensics), integrated 2026-09-21.  BYTES PROVEN ON
 * MAIN: a full driver build carrying this body produces build/bb2.exe SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (== the oracle).
 *
 * Honest sandbox score is 0 (build_insns 208 == target_insns 208), re-measured
 * on the integrated tree 2026-09-21; `--diff` reports 0 source-level /
 * 0 operand-only / 27 not-scored hunks, the not-scored set being masked
 * branch-target artifacts of the isolated build.  (While the function was held
 * as an integration handoff this comment recorded a 2/208 residual on the
 * ADDR_VEC's %lo addend.  That residual was an artifact of measuring before the
 * rodata geometry below was in place; it is gone, and the sandbox and the full
 * build now agree.)
 *
 * The body alone was NOT sufficient - two edits on surfaces a grind session may
 * not touch were also required, and BOTH landed in commit f98985ae3 (the
 * func_8006ECF4 completion) before this body was integrated.  Full recipe +
 * proof: memory/grind/func_800747D8/integration/README.md
 * In short: (1) Makefile:136 RODATA_ALIGN2_FILES += text1b, because
 * tools/gcc-2.7.2/final.c:1515-1518 emits an unconditional `.align 3` before
 * every .rdata ADDR_VEC and this function's table sits at a 4-mod-8 address;
 * (2) the rodata-ownership move that keeps text1b.o(.rodata) contiguous across
 * 0x80015988..0x80015A20 - the hand-written const D_800159A0 plus the
 * compiler-generated ADDR_VECs of func_8006E534 and func_8006ECF4, which the
 * compiler emits into this TU once those functions are C.
 *
 * The s9 change vs the floor-6 body is the selection_sound block only:
 * `sound = 4;` sits at the END of each of the two inner-switch arms (before
 * `goto selection_sound;`) and the block reduces to
 * `if (field64 != 0) sound = 0;`.  Mechanism (measured, not guessed):
 * `reg_set_last` (tools/gcc-2.7.2/rtlanal.c:886-888) stops at a CODE_LABEL,
 * so with `sound = 4;` on the far side of the `selection_sound:` label the
 * store-flag gate at tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather
 * than CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
 * fold; the two-arm branch survives with the test byte on its own pseudo,
 * which is target's `lbu v0,0x64(v0)` / `beqz v0` seat.  Putting the
 * assignment in the ARMS rather than at the top of `case 0:` (s6 variant F,
 * score 8) is what keeps $a0 dead across the field65 update block and lets
 * the two copies cross-jump-merge back into the single `li a0,4` that
 * target carries in the branch's delay slot.
 *
 * Self-vet: memory/grind/func_800747D8/self_vet.md.  The duplicated
 * `sound = 4;` is claimed under .claude/rules/duplicated-statement-into-arms.md
 * and carries the mandated FAKE annotation inline (below).
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
    s32 sound;

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
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        case 2:
            if (MENU_800747D8->field65 == 0) {
                MENU_800747D8->field65 = MENU_800747D8->field64;
            } else {
                MENU_800747D8->field65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (MENU_800747D8->field64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
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
