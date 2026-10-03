#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */





/* Extern function declarations */




extern void func_800194F4(void);

extern void VSync(s32);








extern u8 *D_800A3894;



extern void func_8005B5AC(void);




















extern s32 func_80020D38(void);
extern void func_800602AC(s32, s32);


extern s32 D_800A38B4;











extern void eff_Init(void);
extern void func_80040510(s32, s32, s32);











extern s32 func_8004939C(void);






extern u8 D_800A37A8;






/* GP-relative extern data (for decompiled functions) */




































/* Extern function declarations for decompiled functions */









extern void ResetRCnt(s32);









extern void func_80022568(u8 *);




/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern u16 bits_ExtractMask3F83F8(s32);
extern s32 bits_DepositMask3F83F8(s16);
extern u16 func_80019488(void);
extern void func_800194C0(s16);
extern u16 rand(void);
extern void func_80019568(s32);
extern u8 D_800A38AC;
extern s32 D_800A37D8;
extern u8 D_800A3916;
extern s32 D_800A38D0;
extern s32 D_800A3908;
extern s32 D_800A38FC;
extern u16 D_800A37C4;

s32 func_8003AB44(void) {
    D_800A37B8++;
    switch (D_800A38AC) {
        case 0:
            D_800A38AC = 1;
            break;
        case 1:
            comb_ReadCtsSetRts();
            D_800A37D8 = 0;
            if (D_800A38A0 == 0) {
                SetDispMask(1);
                D_800A38AC = 2;
                break;
            }
            D_800A38AC = 3;
            break;
        case 2:
            if (D_80102788.pressed & 0x10) {
                goto fail;
            }
            if (_comb_control(3, 1, 0) == 0) {
                break;
            }
            goto done;
        case 3:
            if (_comb_control(3, 1, 0) != 0) {
                goto retry;
            }
            /* fall through */
        done:
            _comb_control(3, 0, 0);
            D_800A38AC = 4;
            break;
        retry:
            D_800A37D8++;
            if (D_800A37D8 < 4) {
                break;
            }
            /* fall through */
        fail:
            comb_ResetClose();
            return -1;
        case 4:
            SetDispMask(0);
        case 5:
        case 6:
            D_800A38AC++;
            break;
        case 7:
            D_800A3916 = 1;
            comb_EnableEvents();
            return 1;
    }
    return 0;
}
s32 func_8003ACB8(void) {
    s32 temp_s0;

    func_80077AE0();
    SetDispMask(0);
    D_800A37B8 = 0;
    D_800A38AC = 0;
    D_800A38D0 = 0;
    D_800A3908 = 0;
    D_800A38FC = 0;
    comb_Init();
    do {
        func_80019568(1);
        func_8005C6D0();
        temp_s0 = func_8003AB44();
        VSync(2);
    } while (temp_s0 == 0);
    func_80077B00();
    func_800194F4();
    D_800A37C4 = bits_ExtractMask3F83F8(D_80106A50.unk_00);
    func_8003AA48();
    VSync(1);
    func_8003AA48();
    D_800A38E4 = bits_DepositMask3F83F8(D_800A36C6);
    D_800A37C4 = func_80019488();
    VSync(1);
    func_8003AA48();
    VSync(1);
    func_8003AA48();
    VSync(1);
    func_8003AA48();
    func_800194C0(D_800A36C6);
    D_800A37C4 = rand();
    VSync(1);
    func_8003AA48();
    VSync(1);
    func_8003AA48();
    VSync(1);
    func_8003AA48();
    {
        s32 var_v0;
        if (D_800A38A0 == 0) {
            var_v0 = (u16)D_800A37C4;
        } else {
            var_v0 = D_800A36C6;
        }
        D_800A3904 = var_v0;
    }
    gpu_InitDisplay();
    gpu_SetDispMaskOn();
    ResetRCnt(0xF2000001);
    return temp_s0;
}
void func_8003AE5C(u8 *arg0) {
    s32 addr = (s32)0x80190800;
    s32 result = -1;
    s32 done = 0;

    do {
        s32 cmd = *arg0++;
        switch (cmd) {
            case 1:
                result = *arg0;
            case 0:
            case 20:
                done = 1;
                break;
            case 16:
                arg0 += 5;
                break;
            case 17:
                arg0 += 3;
                break;
            case 3:
                arg0 += 2;
                break;
            case 19:
                arg0 += 4;
                break;
            case 2:
                arg0 += 0x1D;
                break;
        }
    } while (!done);

    if (result >= 0) {
        (&D_800A37A8)[D_800A37A0] = *(u16 *)&D_800A36A4;
        gpu_ResetGraphMode1();
        func_80020D38();
        func_800602AC(result, addr);
    }
}

void func_8003AF40(s32 arg0) {
    if (D_80102778.unk_4[2 + arg0] == 0xFF) {
        D_80102778.unk_4[2 + arg0] = (&D_80102778.unk_4[2])[(u32)arg0 < 1u];
    }
    func_80022580(arg0, (s8)D_80102778.unk_4[4 + arg0], (s8)D_80102778.unk_4[arg0], (s8)D_80102778.unk_4[2 + arg0], 0);
    gpu_ResetGraphMode1();
    func_80020D38();
    func_80040510(arg0, D_8008D578[(s8)D_80102778.unk_4[arg0]], (s32)0x80190800);
}
void func_8003AFFC(void) {
    s32 addr = (s32)0x80190800;
    s32 s0;
    s16 *edcp;
    s32 s2;
    u8 *tbl;
    s32 v1;

    gpu_ResetGraphMode1();
    func_80020D38();
    func_8004939C();

    tbl = D_8008E5CC[0];
    s2 = 0;
    edcp = &g_practice_menu_table[0].unk_14;
    s0 = 0;
loop:
    /* interim: byte-offset puns on g_practice_menu_table (.unk_12 / .unk_0A / .unk_0E), inherited from pre-struct code (9cb130a8); naturalize when func_8003AFFC is matched */
    func_800493E4(*(s16 *)((u8 *)g_practice_menu_table + s0 + 0x12));
    func_800494D4(s2, *(tbl + *(s16 *)((u8 *)&g_practice_menu_table[0].unk_0A + s0) * 8 + *(s16 *)((u8 *)&g_practice_menu_table[0].unk_0E + s0)));

    v1 = *edcp;
    if (v1 != -1) {
        func_800493E4(D_8008EB80[v1]);
        v1 = *edcp;
        if (v1 == 14) {
            func_800493E4(D_8008EB80[14] + 3);
        }
    }
    edcp = (s16 *)((u8 *)edcp + 0x44C);
    s0 += 0x44C;
    s2++;
    if (s2 < 2) goto loop;

    func_80049584(addr);
}
void func_8003B10C(s32 arg0) {
    s32 addr = (s32)0x80190800;

    gpu_ResetGraphMode1();
    func_80020D38();
    func_8004939C();

    func_800493E4(g_practice_menu_table[arg0].unk_12);

    if (D_800A38DC == 5) {
        func_800494D4(arg0, D_8008E6A4[g_practice_menu_table[arg0].unk_0A][g_practice_menu_table[arg0].unk_0E]);
    } else {
        func_800494D4(arg0, D_8008E5CC[g_practice_menu_table[arg0].unk_0A][g_practice_menu_table[arg0].unk_0E]);
    }
    func_80049584(addr);
}
void func_8003B20C(s32 arg0) {
    D_80102778.unk_4[4] = 0;
    D_80102778.unk_4[5] = 1;
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = 1;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = ((s32 *)&D_800900EC)[arg0];
    eff_Init();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = D_8008D538[(s8)D_80102778.unk_4[0]];
}
extern u8 *D_800A3894;
extern void player_SetCharId(s32, s32);
void func_8003B2C8(void) {
    u8 *p = &D_80102778.unk_4[0];
    D_800A3836 = *p;
    {
        u8 *base = D_800A3894;
        u8 v1 = D_800A376A;
        u8 v0 = *base;
        D_800A36C8 = v1;
        D_800A376A = 0;
        *p = v0;
    }
    player_SetCharId(0, 0);
}
void func_8003B328(void) {
    u8 *p = &D_80102778.unk_4[0];
    u8 v_277C = *p;
    u8 v_376A = D_800A376A;
    u8 v_3836 = D_800A3836;
    u8 v_36C8 = D_800A36C8;
    D_800A3836 = 0xFF;
    D_800A3915 = v_277C;
    D_800A36F4 = v_376A;
    *p = v_3836;
    D_800A376A = v_36C8;
    player_SetCharId(0, v_36C8);
    func_80022568((u8 *)g_practice_menu_table);
}
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = D_8008D538[(s8)D_80102778.unk_4[0]];
    a1 = D_8008D9EC[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        /* FAKE: store-only pointer alias — direct symbolic stores expand via the
           assembler sb macro ($at), so the address never enters RA; the pointer
           local makes it an RA-visible pseudo materialized into $v1 pre-branch,
           matching target. Direct/ternary/diamond/offset forms measured 6/6/8/6.
           Sanctioned per owner ruling 2026-07-14 (decisions.md), per-instance. */
        u8 *p = &D_80102778.unk_4[1];
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_80102778.unk_4[3] = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102778.unk_A[1] = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
s32 func_8003B484(u8 *arg0) {
    D_800A3712 = 1;
    D_80102778.unk_4[1] = arg0[0];
    D_80102778.unk_4[3] = arg0[1];
    D_80102778.unk_A[1] = arg0[2];
    func_8003AF40(1);
    func_8003AFFC();
    return 3;
}
void func_8003B4DC(void) {
    D_800A3712 = 1;
    D_80102778.unk_4[2] = 0;
    D_80102778.unk_4[1] = 0x1F;
    D_80102778.unk_4[3] = 0;
    func_8003AF40(0);
    func_8003AF40(1);
    func_8003AFFC();
}
void func_8003B534(s32 a0) {
    D_800A37B0 = a0;
    D_800A3834 = 6;
    D_800A3878 = D_800A3894 + (a0 * 4 + 1);
}
void func_8003B56C(s32 arg0) {
    D_800A390C = arg0;
    D_800A3834 = 6;
    D_800A3878 = D_800A385C + (arg0 * 4 - 4);
}
void func_8003B5A4(void) {
    s32 done;
    u8 *chardata;

    func_8005B5AC();
    done = 0;
    chardata = &D_80102778.unk_4[1];

    do {
        s32 ptr = D_800A3844;
        s32 cmd;
        D_800A3844 = ptr + 1;
        cmd = *(u8 *)ptr;

        switch (cmd) {
            case 16: {
                s32 ret = func_8003B3A4((u8 *)D_800A3844);
                D_800A3844 += ret;
                break;
            }

            case 17: {
                s32 ret = func_8003B484((u8 *)D_800A3844);
                D_800A3844 += ret;
                break;
            }

            case 3: {
                u8 *p = (u8 *)D_800A3844;
                u8 byte0;
                D_800A3844 = (s32)(p + 1);
                byte0 = p[0];
                D_800A3844 = (s32)(p + 2);
                chardata[0] = byte0;
                chardata[2] = p[1];
                if ((s8)byte0 == D_800A3915) {
                    player_SetCharId(1, D_800A36F4);
                }
                func_8003AF40(1);
                func_8003AFFC();
                break;
            }

            case 1: {
                u8 byte;
                u8 *p = (u8 *)D_800A3844;
                u8 counter = D_800A37A0;
                D_800A3844 = (s32)(p + 1);
                byte = *p;
                D_800A37A0 = counter + 1;
                D_800A36A4 = byte;
                (&D_800A37A8)[counter] = byte;
                D_800A3834 = 22;
                done = 1;
                func_80022568((u8 *)g_practice_menu_table);
                D_800A3907 = 0;
                break;
            }

            case 0:
                if (D_800A380C == 0) {
                    D_800A38A4 = (D_8008D9EC[g_practice_menu_table[0].unk_0A] != 0);
                    D_800A3834 = 18;
                } else {
                    D_800A3834 = 10;
                }
                done = 1;
                break;

            case 19:
                D_800A3834 = 6;
                D_800A3878 = D_800A3844;
                D_800A3844 += 4;
                done = 1;
                break;

            case 2:
                D_800A3894 = (u8 *)D_800A3844;
                D_800A3844 += 0x1D;
                func_8003B2C8();
                func_8003AF40(0);
                func_8003B3A4(D_800A3894 + 1);
            case 18:
                D_800A3834 = 0;
                D_800A3907++;
                done = 1;
                break;

            case 20:
                D_800A385C = (u8 *)D_800A3844;
                D_800A3844 += 12;
                done = 1;
                func_8003B4DC();
                func_8003B56C(1);
                D_800A38BA = 0;
                break;
        }
    } while (!done);
}
void func_8003B870(void) {
    player_SetCharId(0, D_800A376A);
    player_SetCharId(1, 0);
    func_8005B5AC();
    gpu_InitDisplay();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    gpu_SetDispMaskOn();
}
void func_8003B8E4(void) {
    s32 tmp;
    s32 ret;

    tmp = D_800A37B8 + 1;
    D_800A37B8 = tmp;
    if (tmp < 3) {
        ret = func_80060544(D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    }
    if (D_800A37B8 == 3) {
        DrawSync(0);
        func_8003AE5C(D_800A3844);
        D_800A37C0 = 500;
        D_800A38F8 = 0;
        g_disp_enable = DISP_ACTIVE;
        func_8001D790();
        func_8003B5A4();
        gpu_SetDrawEnvBg(1, 0, 0, 0);
        D_800A390D = 1;
    }
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
u16 D_800A37C4;
s32 D_800A37D8;
u8 D_800A38AC;
s32 D_800A38D0;
s32 D_800A38FC;
s32 D_800A3908;
u8 D_800A3916;
