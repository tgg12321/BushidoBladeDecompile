/* REJECTED (session 6, solver) - polarity of the duplicated-call form is
 * codegen-INERT; measures identically to the already-banked
 * selection_sound-duplicated-call-into-arms.c.
 *
 *     if (MENU_800747D8->field64 != 0) {
 *         func_8005C650(0, 0x7F, 0x7F);
 *     } else {
 *         func_8005C650(4, 0x7F, 0x7F);
 *     }
 *
 * MEASURED (session 6): sandbox --disable all score 7, build_insns 211 -
 * the same figures as the `== 0` / 4-first spelling
 * (rejected/selection_sound-duplicated-call-into-arms.c, re-measured this
 * session at 7/211 as variant C).  GCC canonicalises the branch sense, so
 * source-level arm order is not a lever on this chassis.  This EXTENDS the
 * session-2 polarity kill (measured on the same-variable floor-6 chassis) to
 * the duplicated-call chassis.
 *
 * WHAT THIS FORM DOES GET RIGHT (the s6 frontier): hunk 17 disappears -
 * `lbu v0,0x64(v0)` matches target's register seat exactly, because the two
 * constants 4/0 go straight into the hard arg register $a0 in each arm and
 * the loaded test byte is free to live in $v0.  The entire 3-insn surplus is
 * the two arms' `li a1,127 / li a2,127 / jal` tails failing to
 * cross-jump-merge with each other.
 *
 * WHY THAT MERGE NEVER HAPPENS (proved this session with the pre-existing
 * BB2_XJUMP_DEBUG knob; trace at tmp/grind/func_800747D8/s6/dumps/xjdbg.txt
 * lines 1855-1870, RTL uids from tmp/grind/func_800747D8/s6/dumps/
 * text1b.sched2 around line 107818):
 *   XJDBG: enter e1=288 e2=424 min=1 (own-label)
 *   XJDBG:   MATCH i1=286 i2=418 parallel min->0
 *   XJDBG:   PAT-MISMATCH i1=284 set(reg<-127) vs i2=409 set lose=0
 *   XJDBG: result e1=288 min=0 last1=286 => WIN
 *   XJDBG: DO_CROSS_JUMP jump=288 newjpos=286 newlpos=418
 * jump_insn 288 is arm1's `goto confirm`; code_label 424 is `confirm`; the
 * insn physically preceding `confirm` is call_insn 418 - case 2's
 * func_8005C650 call, which falls through into confirm.  So the minimum=1
 * own-label attempt at tools/gcc-2.7.2/jump.c:2005 WINS with a ONE-insn
 * match (call vs call) and retargets arm1's jump to a NEWLY created label
 * (uid 660).  Same for arm2 (jump_insn 304).  After that the sibling-jump
 * pairing loop at tools/gcc-2.7.2/jump.c:2011-2021 can never pair 288 with
 * 304: it is guarded by `INSN_UID (JUMP_LABEL (insn)) < max_uid` (false for
 * uid 660) and `jump_chain` is never updated for a label created by
 * do_cross_jump.  The trace contains ZERO `chain-partner` entries for
 * e1=288 or e1=304 across every `while (changed)` iteration.
 *
 * kill_scope: instance (this spelling, floor-6 chassis, zero FAKE
 * constructs).  The pass behaviour above is a class fact, but the BLOCK
 * LAYOUT that triggers it is a property of this function's source order, so
 * the kill is recorded as instance.
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
        if (MENU_800747D8->field64 != 0) {
            func_8005C650(0, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
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

#undef MENU_800747D8
