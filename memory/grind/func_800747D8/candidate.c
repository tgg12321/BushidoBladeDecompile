/* func_800747D8 candidate - s9 (forensics), honest sandbox score 2/208 (was 6).
 *
 * The ONLY remaining scored hunk is hunk 9, the jump-table base operand
 * (target `lw v0,0(at)` vs ours `lw v0,24(at)`): GCC emits this switch's
 * ADDR_VEC into text1b.c's own .rodata as the local label .L1317, and the
 * isolated sandbox build cannot place it at jtbl_80015A0C (0x80015A0C).
 * ZERO source-level hunks remain - every other instruction is byte-identical
 * to asm/funcs/func_800747D8.s.
 *
 * The s9 change vs the floor-6 body is the selection_sound block only:
 * `sound = 4;` now sits at the END of each of the two inner-switch arms
 * (before `goto selection_sound;`) and the block reduces to
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
 * NOT yet self-vetted.  The duplicated `sound = 4;` is a real statement that
 * re-merges byte-neutrally; the next session must classify it against
 * .claude/rules/duplicated-statement-into-arms.md before any submission.
 */
extern u8 D_8009BD20[][2];
extern s16 D_800A35D0;
extern s8 D_800A35DC;
extern u8 *D_800A36A0;

typedef struct {
    u8 pad00[0x10];
    union {
        s32 word10;
        s16 half10[2];
    } field10;
    u8 pad14[4];
    s16 field18[2];
    u8 pad1C[0x18];
    u16 field34;
    u8 pad36[2];
    s16 field38[2];
    u16 field3C;
    u8 pad3E[0x26];
    u8 field64;
    u8 field65;
    u8 field66;
    u8 field67;
} S_800747D8;

#define MENU_800747D8 ((S_800747D8 *)D_800A36A0)

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
            sound = 4;
            goto selection_sound;
        case 2:
            if (MENU_800747D8->field65 == 0) {
                MENU_800747D8->field65 = MENU_800747D8->field64;
            } else {
                MENU_800747D8->field65 -= 1;
            }
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

#undef MENU_800747D8
