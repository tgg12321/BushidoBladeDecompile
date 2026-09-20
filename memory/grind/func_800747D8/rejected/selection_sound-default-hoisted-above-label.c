/* REJECTED (session 6, solver) - REGRESSED the floor to 8, but this is the
 * most informative form in this bank and the s6 frontier: it is the first
 * spelling ever measured that reproduces BOTH of target's selection-block
 * bytes the floor-6 baseline gets wrong.
 *
 *     s32 sound;                        (function-scope declaration)
 *     ...
 *     case 0:
 *         sound = 4;                    <-- ABOVE the `selection_sound:` label
 *         switch (ret & 0xFF) { ... goto selection_sound; ... }
 *         goto confirm;
 * selection_sound:
 *         if (MENU_800747D8->field64 != 0) {
 *             sound = 0;
 *         }
 *         func_8005C650(sound, 0x7F, 0x7F);
 *
 * MEASURED (session 6): sandbox --disable all score 8, build_insns 208
 * (== target_insns).  Scored diff in tmp/grind/func_800747D8/s6/diff_F.txt.
 *
 * WHAT IT GETS RIGHT (both were open on the floor-6 baseline):
 *   - hunk 17 register seat: `lbu v0,0x64(v0)` - MATCHES target.  The load
 *     no longer lands in $a0, because $a0 is carrying the default 4.
 *   - the branch itself: `beqz v0,<join>` - MATCHES target (the floor-6
 *     baseline emits `bnez a0,<join>`).
 *
 * WHY IT ESCAPES THE STORE-FLAG FOLD (the point of the experiment): placing
 * `sound = 4;` above the `selection_sound:` CODE_LABEL makes reg_set_last
 * (tools/gcc-2.7.2/rtlanal.c:886-888: "Scan backwards until reg_set_last_1
 * changed one of the above flags.  Stop when we reach a label") terminate at
 * that label and return 0, so temp3 is the REG itself, not CONST_INT, and
 * the gate at tools/gcc-2.7.2/jump.c:1178-1181 fails for BRANCH_COST 1.  The
 * branchy shape survives - 208 insns, not 205.
 *
 * WHY IT STILL COSTS 8: `sound = 4;` is now in a DIFFERENT basic block from
 * the branch, so reorg's backward fill_simple_delay_slots cannot reach it.
 * Three consequences, all visible in diff_F.txt:
 *   - hunk 17's delay slot holds `li a1,127` (stolen from the branch's
 *     target thread) where target holds `li a0,4`;
 *   - hunk 10: the orphaned `li a0,4` is emitted into the switch-dispatch
 *     delay slot at c734, an insn target does not have;
 *   - hunks 13/14: with $a0 pinned to the live 4 across the field65 block,
 *     that block's `lbu`/`bne`/`addiu` move to $a1 (3 operand-only diffs the
 *     floor-6 baseline does not have).
 *
 * kill_scope: instance (this placement, floor-6 chassis, zero FAKE
 * constructs).
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
    s32 sound;
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
        sound = 4;
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
