/* REJECTED (session 7, forensics) -- relocation CONTROL for
 * rejected/selection_sound-dup-call-relocated-before-confirm.c.  Single-call
 * two-arm form (the 10/205 fold regime) with the `selection_sound:` block moved
 * to sit immediately before `confirm:`, so the relocation cost can be read off
 * independently of the duplicated-call arm merge.
 *
 * MEASURED (session 7): sandbox --disable all -> score 29, build_insns 214,
 * i.e. the relocation alone costs +9 insns over the in-place 10/205 spelling.
 * Target's own layout puts the selection block FIRST, before case 1
 * (asm/funcs/func_800747D8.s:107-118 precede the case-1 label at :120), so the
 * relocation can never be part of a matching form.
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
        {
        s32 sound;
        if (MENU_800747D8->field64 != 0) {
            sound = 0;
        } else {
            sound = 4;
        }
        func_8005C650(sound, 0x7F, 0x7F);
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
