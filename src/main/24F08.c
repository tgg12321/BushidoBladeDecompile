/* func_80034708 alone. .text 0x80034708 (ROM 0x24F08). Start boundary: G8 (cc1 -G8 by proof).
 *
 * Compiled -G8 (Makefile GP_FILES). The
 * original compiler knew the 4-byte cursor array D_800A3174 was small data:
 * the target reads cursor[0] / cursor[1] straight off $gp while the loop walks
 * &cursor[i]. Under -G0 an array element's address stays in a register. The
 * same setting explains this function's strings: the <=8-byte ones sit in
 * .sdata at 0x800A3178.., the 9-byte ones in .rodata at 0x80010834. */
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"

extern s32 rand(void);
extern void func_8005C650(s32, s32, s32);
extern void func_800344B4(void);

/* This menu's two cursors ([0] = P1 row 0..11, [1] = P2 row 0..3) and its
 * colour / format strings. */
extern s16 D_800A3174[2];
extern u8 D_800A3178[];
extern u8 D_800A3180[];
extern u8 D_800A3188[];
extern u8 D_800A3190[];
extern u8 D_800A3198[];
extern u8 D_800A31A0[];
extern u8 D_800A31A8[];
extern u8 D_800A31B0[];
extern u8 D_800A31B8[];
extern u8 D_800A31C0[];
extern u8 D_800A31C8[];
extern u8 D_800A31D0[];
extern u8 D_80010834[];
extern u8 D_80010840[];

void func_80034708(void) {
    s32 i;
    u8 *off;
    u8 *on;

    D_800A37B8++;
    rand();
    off = D_800A3178;
    on = D_800A3180;
    func_8003D52C(D_800A3188, (s32)(D_800A3174[0] == 0 ? on : off), (s8)D_80102778.unk_4[0]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 0 ? on : off), (s8)D_80102778.unk_4[1]);
    func_8003D52C(D_800A3188, (s32)(D_800A3174[0] == 1 ? on : off), (s8)D_80102778.unk_4[2]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 1 ? on : off), (s8)D_80102778.unk_4[3]);
    func_8003D52C(D_800A3198, (s32)(D_800A3174[0] == 2 ? on : off), (s8)D_80102778.unk_4[4]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 2 ? on : off), (s8)D_80102778.unk_4[5]);
    func_8003D52C(D_800A31A0, (s32)(D_800A3174[0] == 3 ? on : off), D_80102778.unk_0[0] >> 8);
    func_8003D52C(D_800A31A8, (s32)(D_800A3174[1] == 3 ? on : off), D_80102778.unk_0[1] >> 8);
    func_8003D52C(D_800A31B0, (s32)(D_800A3174[0] == 4 ? on : off), (s8)D_80102778.unk_C);
    func_8003D52C(D_800A31B0, (s32)(D_800A3174[0] == 5 ? on : off), (s8)D_80102778.unk_D);
    func_8003D52C(D_800A31B8, (s32)(D_800A3174[0] == 6 ? on : off), (s8)D_80102778.unk_E);
    func_8003D52C(D_80010834, (s32)(D_800A3174[0] == 7 ? on : off), (s8)D_80102778.unk_F);
    func_8003D52C(D_80010840, (s32)(D_800A3174[0] == 8 ? on : off), D_80106A50.flags & 1);
    func_8003D52C(D_800A31C0, (s32)(D_800A3174[0] == 9 ? on : off), (D_80106A50.flags >> 1) & 1);
    func_8003D52C(D_800A31C8, (s32)(D_800A3174[0] == 10 ? on : off), D_800A36F9);
    func_8003D52C(D_800A31D0, (s32)(D_800A3174[0] == 11 ? on : off), D_800A3690);

    for (i = 0; i < 2; i++) {
        if (g_pad_state.pressed & (0x1000 << (i * 16))) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3174[i] > 0) {
                D_800A3174[i]--;
            } else {
                D_800A3174[i] = (i != 0) ? 3 : 11;
            }
        } else if (g_pad_state.pressed & (0x4000 << (i * 16))) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3174[i] < ((i != 0) ? 3 : 11)) {
                D_800A3174[i]++;
            } else {
                D_800A3174[i] = 0;
            }
        } else if (g_pad_state.pressed & (0x8000 << (i * 16))) {
            func_8005C650(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                D_80102778.unk_4[i]--;
                break;
            case 1:
                D_80102778.unk_4[2 + i]--;
                break;
            case 2:
                D_80102778.unk_4[4 + i]--;
                break;
            case 3:
                D_80102778.unk_0[i] -= 0x80;
                break;
            case 4:
                D_80102778.unk_C--;
                break;
            case 5:
                D_80102778.unk_D--;
                break;
            case 6:
                D_80102778.unk_E--;
                break;
            case 7:
                D_80102778.unk_F--;
                break;
            case 8:
                D_80106A50.flags ^= 1;
                break;
            case 9:
                D_80106A50.flags ^= 2;
                break;
            case 10:
                D_800A36F9--;
                break;
            case 11:
                D_800A3690--;
                break;
            }
        } else if (g_pad_state.pressed & (0x2000 << (i * 16))) {
            func_8005C650(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                D_80102778.unk_4[i]++;
                break;
            case 1:
                D_80102778.unk_4[2 + i]++;
                break;
            case 2:
                D_80102778.unk_4[4 + i]++;
                break;
            case 3:
                D_80102778.unk_0[i] += 0x80;
                break;
            case 4:
                D_80102778.unk_C++;
                break;
            case 5:
                D_80102778.unk_D++;
                break;
            case 6:
                D_80102778.unk_E++;
                break;
            case 7:
                D_80102778.unk_F++;
                break;
            case 8:
                D_80106A50.flags ^= 1;
                break;
            case 9:
                D_80106A50.flags ^= 2;
                break;
            case 10:
                D_800A36F9++;
                break;
            case 11:
                D_800A3690++;
                break;
            }
        }
        D_80102778.unk_4[i] = ((s8)D_80102778.unk_4[i] + 33) % 33;
        D_80102778.unk_4[2 + i] = ((s8)D_80102778.unk_4[2 + i] + 8) % 8;
        D_80102778.unk_4[4 + i] = ((s8)D_80102778.unk_4[4 + i] + 2) % 2;
    }
    D_80102778.unk_C = ((s8)D_80102778.unk_C + 38) % 38;
    D_80102778.unk_D = ((s8)D_80102778.unk_D + 7) % 7;
    D_80102778.unk_E &= 1;
    D_80102778.unk_F &= 1;
    D_800A36F9 = (D_800A36F9 + 4) % 4;
    D_800A3690 &= 1;
    if (g_pad_state.pressed & 0x08000800) {
        func_8005C650(1, 0x7F, 0x7F);
        func_800344B4();
    }
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s16 D_800A3174[2] = { 0, 0 };
