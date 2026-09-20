/* REJECTED (session 7, forensics) -- "dup-call form relocated to sit immediately
 * before `confirm:`".  Semantically identical to the banked in-place dup-call
 * form (rejected/selection_sound-duplicated-call-into-arms.c); the ONLY change
 * is that the `selection_sound:` block is moved down so it is the last thing
 * before `confirm:` and therefore needs no trailing `goto confirm;`.
 *
 * WHY IT WAS WRITTEN: to test whether the s6 finding ("the minimum=1 own-label
 * find_cross_jump at tools/gcc-2.7.2/jump.c:2005 wins a 1-insn call-vs-call
 * match against case 2's call and thereby locks the two arms out of the
 * sibling-jump pairing loop") is caused by the arm jumps having been THREADED
 * past the if/else join label to `confirm` -- which happens precisely because
 * the join label is immediately followed by the `goto confirm;`.
 *
 * MEASURED (session 7): sandbox --disable all -> score 32, build_insns 217.
 * The arm merge DID happen and is proved by the BB2_XJUMP_DEBUG trace
 * tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907:
 *     XJDBG: enter e1=406 e2=421 min=1 (own-label)
 *     XJDBG:   MATCH i1=404 i2=418 parallel min->0
 *     XJDBG:   MATCH i1=402 i2=416 set(reg<-127) min->-1
 *     XJDBG:   MATCH i1=400 i2=414 set(reg<-127) min->-2
 *     XJDBG:   PAT-MISMATCH i1=398 set(reg<-4) vs i2=412 set(reg<-0) lose=0
 *     XJDBG: result e1=406 min=-2 last1=400 => WIN
 *     XJDBG: DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414
 * i.e. jal / a2=127 / a1=127 merged, mismatching only at a0=4 vs a0=0 -- the
 * exact 3-insn merge the in-place spelling never gets.  It is REJECTED anyway
 * because relocating the block costs +9 insns of layout (the sibling control,
 * rejected/selection_sound-single-call-relocated-before-confirm.c, moves
 * 10/205 -> 29/214), which swamps the 3 insns the merge recovers.
 *
 * kill_scope: instance (this relocation, this chassis, zero FAKE constructs).
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
selection_sound:
        if (MENU_800747D8->field64 == 0) {
            func_8005C650(4, 0x7F, 0x7F);
        } else {
            func_8005C650(0, 0x7F, 0x7F);
        }
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
