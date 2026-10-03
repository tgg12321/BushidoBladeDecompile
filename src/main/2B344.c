/* 92 game functions, among them bitstream_ReadBits, gpu_SetDrawMoveArray and math_AlignUp4. .text
 * 0x8003AB44 (ROM 0x2B344). Start boundary: LEGACY (a tooling split); one object by the per-file
 * gp model (Q65 merge group) and the rodata-align section 11 merge. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

/* Extern function declarations */
extern void pad_ResetState(void);

extern void VSync(s32);








extern void func_800602AC(s32, s32);


extern s32 D_800A38B4;











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
            if (g_pad_state.pressed & 0x10) {
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
    pad_ResetState();
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
    edcp = &D_80101EC8[0].unk_14;
    s0 = 0;
loop:
    /* interim: byte-offset puns on D_80101EC8 (.unk_12 / .unk_0A / .unk_0E), inherited from pre-struct code (9cb130a8); naturalize when func_8003AFFC is matched */
    func_800493E4(*(s16 *)((u8 *)D_80101EC8 + s0 + 0x12));
    func_800494D4(s2, *(tbl + *(s16 *)((u8 *)&D_80101EC8[0].unk_0A + s0) * 8 + *(s16 *)((u8 *)&D_80101EC8[0].unk_0E + s0)));

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

    func_800493E4(D_80101EC8[arg0].unk_12);

    if (D_800A38DC == 5) {
        func_800494D4(arg0, D_8008E6A4[D_80101EC8[arg0].unk_0A][D_80101EC8[arg0].unk_0E]);
    } else {
        func_800494D4(arg0, D_8008E5CC[D_80101EC8[arg0].unk_0A][D_80101EC8[arg0].unk_0E]);
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
    func_80022568((u8 *)D_80101EC8);
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
                func_80022568((u8 *)D_80101EC8);
                D_800A3907 = 0;
                break;
            }

            case 0:
                if (D_800A380C == 0) {
                    D_800A38A4 = (D_8008D9EC[D_80101EC8[0].unk_0A] != 0);
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

/* ---- merged from code6cac_c2.c (owner ruling Q65: one original file) ---- */


/* Extern function declarations */
extern void LoadImage(s32, s32);



extern u32 D_800A3D30;
















extern Unk800A4750Rec D_800A4750[];
extern Unk800A6690Rec D_800A6690[];
extern s16 D_800A7FE0[32][32];
extern u16 D_800A87E0[];
extern u8 g_stage_collision[];
extern s32 *func_8004153C();
extern void func_800432A0(s32, s32, s32, s32, s32);



extern void func_8003AF40(s32);






extern s32 D_800A38B4;








extern s32 func_80052C28(s32, s32);
extern s32 func_800788B0(void);
extern void func_800372C0(void);
extern void func_800548DC(void);
extern s32 func_8005FC9C(s32, s32);
extern s32 func_80054F68(void);
extern s32 func_8005E54C(s32, s32, s32);
extern void func_8005C650(s32, s32, s32);
extern void func_80060758(void);
extern void func_8001CD68(u8 *);

extern s32 func_800600C8(s32, s32, s32);
extern void func_8001DA2C(void);

extern void func_8005FBC8(s32, s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_8001DBE4(void);


extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern void func_800342A0(void);
extern s32 func_80022408(s32 *);
extern void StoreImage(s32 *, u16 *);
extern void gte_ReadFarColor(u8 *);










extern s8 D_8008EA70;



extern s32 g_gpu_ot_ptr;









extern void func_8003AFFC(void);
extern void func_8004659C(s32);
extern void snd_SerialMixOn(void);
extern void cdrom_StartAudio(s32, s32);
extern void func_80037260(void);
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    u8 flags;

    magic = 0x80190800;
    func_8001DA2C();
    game_Cleanup();
    if (g_disp_enable != DISP_ACTIVE) gpu_InitDisplay();
    if (g_disp_enable != DISP_DISABLED) gpu_SetDispMaskOn();
    func_800174F4();
    gpu_ResetGraphMode1();
    func_80020D38();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            saved_first = D_80101EC8[0].unk_12;
            saved_44c = D_80101EC8[1].unk_12;
            if (qf & 0x10) D_80101EC8[0].unk_12 = 0x32;
            if (q[3] & 0x20) D_80101EC8[1].unk_12 = 0x32;
            func_8003AFFC();
            D_80101EC8[0].unk_12 = saved_first;
            D_80101EC8[1].unk_12 = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EC8[0].unk_12; else a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80101EC8[1].unk_12; else a0_arg = -1;
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, p[0], 0, a3_arg, a0_arg, -1, -1, magic);
    func_80041688(0, 0);
    func_80041688(1, 0);
    if (((u8 *)D_800A3878)[3] & 0x40) func_8004659C(-1);
    if ((s8)D_80102778.unk_4[1] == 0xE || (s8)D_80102778.unk_4[1] == 0x1D) {
        func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
    }
    func_8001DBE4();
    g_disp_enable = DISP_DISABLED;
    g_disp_fade = 0;
    snd_SerialMixOn();
    v0 = func_80036EA8(5, ((u8 *)D_800A3878)[1]);
    cdrom_StartAudio(v0, ((u8 *)D_800A3878)[2]);
    func_80037260();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_SetDispMaskOn();
}

void func_8003BCB4(void) {
    D_800A37B8++;

    if (func_80054F68() != 0) {
        if ((g_pad_state.pressed & 0x400040) == 0) {
            return;
        }
    }

    func_800372C0();
    func_800548DC();

    if (D_800A38DC != 0) {
        return;
    }

    if (D_800A3894 != 0) {
        D_800A3834 = 0;
        switch (D_800A37B0) {
        case 1:
        case 2:
        case 4:
        case 5:
            D_800A3907++;
            return;
        case 3:
            func_8003AF40(0);
            func_8003AFFC();
            /* fall through */
        case 6:
            D_800A3894 = 0;
            goto call_bar;
        default:
            return;
        }
    }

    if (D_800A385C != 0) {
        s32 val = D_800A390C;
        if (val == 1) {
            D_800A3834 = 0;
            return;
        }
        if (val == 0) {
            return;
        }
        if (val >= 4) {
            return;
        }
        D_800A385C = 0;
    }

call_bar:
    func_8003B5A4();
}

extern void func_80078824(s32);
void func_8003BE10(void) {
    gpu_ResetGraphMode1();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    func_8005B72C();
    func_80078824((s32)0x80118800);
    snd_SerialMixOn();
    {
        s32 v0 = func_80036EA8(5, 0x20);
        cdrom_StartAudio(v0, 4);
    }
    func_80037260();
    D_800A3834 = 0xB;
    gpu_SetDispMaskOn();
}
void func_8003BEA8(void) {
    s32 s0 = 0;
    s32 v0;
    s32 v1;

    v0 = func_800788B0();
    v1 = g_pad_state.pressed & 0x40;
    v0 = (u32)v0 > 0u;
    if (v1 != 0) {
        v0 = 1;
    }
    if (v0 != 0) {
        func_800372C0();
        {
            s32 *ptr = &D_80106A50.unk_00;
            s32 old_val = *ptr;
            s32 new_val = old_val | D_800A37A4;
            if (new_val != old_val) {
                *ptr = new_val;
                s0 = 1;
            }
        }
        {
            s32 stage_u = (s32)(u16)D_80101EC8[0].unk_0A;
            s16 stage;
            if ((u32)stage_u < 2u) {
                goto check_bit;
            }
            stage = (s16)stage_u;
            if (stage == 2) {
                goto check_bit;
            }
            if ((u32)(stage_u - 0xC) < 2u) {
                goto check_bit;
            }
            if (stage == 0xE) {
                goto check_bit;
            }
            goto skip_bit;
        }
    check_bit:
        {
            s32 bit = (s16)D_80101EC8[0].unk_0A;
            u8 val;
            if (bit >= 0xC) {
                bit = bit - 9;
            }
            {
                u8 *vptr = &D_80106A50.unk_04;
                val = *vptr;
                if (!((val >> bit) & 1)) {
                    *vptr = val | (1 << bit);
                    s0 = 1;
                }
            }
        }
    skip_bit:
        if (s0 != 0) {
            D_800A3834 = 0x1A;
        } else {
            D_800A3834 = 8;
        }
    }
}
void func_8003BFC4(void) {
    s32 v;
    gpu_ResetGraphMode1();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    v = func_80045814();
    func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    game_Init();
    D_800A3834 = 8;
}

extern void func_8003B10C(s32);

void func_8003C040(void) {
    s32 a0;
    s8 *p;
    gpu_InitDisplay();
    gpu_ResetGraphMode1();
    func_80020CDC();
    if (((u32)(D_800A38A4 - 4)) < 2u) {
        file_ResetDmaFlag();
    }
    {
        if (D_800A38A4 == 6) {
            D_80102778.unk_4[0] = 8;
            D_80102778.unk_4[2] = 6;
            a0 = 0;
            goto after_dispatch;
        }
        if (D_800A38A4 == 7) {
            D_80102778.unk_4[0] = 0x16;
            D_80102778.unk_4[2] = 7;
            a0 = 0;
            goto after_dispatch;
        }
        if (D_800A38A4 == 8) {
            a0 = 0;
            D_80102778.unk_4[0] = 0x1E;
            goto write_e_zero;
        }
        if (D_800A38A4 == 9) {
            a0 = 0;
            D_80102778.unk_4[0] = 0x20;
        write_e_zero:
            D_80102778.unk_4[2] = 0;

        after_dispatch:
            func_8003AF40(a0);

            func_8003B10C(0);
        }
    }
    if (D_800A38A4 != 9) {
        a0 = D_800A38A4;
    } else {
        a0 = 8;
    }
    func_8005FBC8(a0, (s32)0x80118800);
    {
        if (D_800A38A4 == 4) {
            if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
                goto do_copy;
            }
        }
        if (D_800A38A4 == 5) {
            if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
                goto skip_copy;
            }
        do_copy:
            D_80102778.unk_4[0] = D_80102778.unk_4[1];

            D_80102778.unk_4[2] = D_80102778.unk_4[3];
            func_8003AF40(0);
            func_8003B10C(0);
        }
    skip_copy:;
    }
    func_80054884(0x16, (&D_8009016C)[D_800A38A4], 0, D_80101EC8[0].unk_12, -1, -1, -1, (s32)0x80118800);
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    p = (s8 *)(((s8 *)(&D_8008EA70)) + (D_800A38A4 << 1));
    if (p[0] >= 0) {
        snd_SerialMixOn();
        cdrom_StartAudio(func_80036EA8(5, p[0]), (u8)p[1]);
        func_80037260();
    }
    D_800A37B8 = 0;
    D_800A3834 = 0x13;
    gpu_SetDispMaskOn();
}
void func_8003C2C0(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_8005FC9C(D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    if (func_80054F68() == 0 || (g_pad_state.pressed & 0x400040) != 0) {
        if ((u32)(D_800A38A4 - 6) < 2u && D_800A3781 != 0) {
            s32 stage = (s16)D_80101EC8[0].unk_0A;
            s32 newval = 8;
            if (D_8008D9EC[stage] != 0) {
                newval = 9;
            }
            D_800A38A4 = newval;
            D_800A3834 = 0x12;
        } else {
            u8 a4val = D_800A38A4;
            if ((u32)(a4val - 4) < 2u) {
                D_800A3834 = 0x18;
            } else if ((u32)(a4val - 6) < 2u) {
                D_800A3834 = 0x1A;
            } else if ((u32)(a4val - 8) < 2u) {
                D_800A3834 = 0x1A;
            } else if (a4val < 2u) {
                D_800A3834 = 0xA;
            } else {
                D_800A3834 = 8;
            }
        }
    }
    {
        s16 state = D_800A3834;
        if (state != 0x13) {
            func_800372C0();
            state = D_800A3834;
            if (state != 0x12) {
                func_800548DC();
            }
        }
    }
}
void func_8003C42C(void) {
    /* FAKE: frame layout -- only counts[0..2] are used (D_800A377C holds 0/1/2); counts[3]..[6]
     * score 4, [7] or [8] gives the target's -0x38 frame. */
    s32 counts[8];
    s32 i;

    s32 rounds;
    rounds = D_800A389B;
    i = 0;
    counts[2] = 0;
    counts[1] = 0;
    counts[0] = 0;
    if (rounds > 0) {
        /* FAKE: base / n are second handles of counts / rounds for the loop; using them
         * directly scores 4. */
        s32 *base = counts;
        s32 n = rounds;
        do {
            s32 idx = D_800A377C[i];
            base[idx]++;
            i++;
        } while (i < n);
    }
    /* FAKE: the counts[0] < counts[1] store is written in both decisive arms (here and in
     * the tie-break below); one shared store after the if/else scores 11. */
    if (counts[0] != counts[1]) {
        D_800A382D = counts[0] < counts[1];
    } else {
        s32 rounds2 = D_800A389B;
        i = 0;
        counts[1] = 0;
        counts[0] = 0;
        if (rounds2 > 0) {
            /* FAKE: base2 / n2, the same handles for the second tally; direct use scores 6. */
            s32 *base2 = counts;
            s32 n2 = rounds2;
            u8 (*tbl)[2] = D_800F65F8;
            do {
                s32 j = 0;
                s32 *p = base2;
                u8 *q = *tbl;
                do {
                    *p += *q++;
                    j++;
                    p++;
                } while (j < 2);
                i++;
                tbl++;
            } while (i < n2);
        }
        if (counts[0] != counts[1]) {
            D_800A382D = counts[0] < counts[1];
        } else {
            D_800A382D = 2;
        }
    }
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x15;
}
void func_8003C560(void) {
    s32 counter;
    s32 ret;
    s32 id;
    u8 a4val;

    counter = D_800A37B8 + 1;
    D_800A37B8 = counter;
    if (D_800A382D == 2) {
        if (counter == 0x1E) {
            func_8005C650(0xA4, 0x7F, 0x7F);
        }
    } else {
        if (counter == 0x1E) {
            id = 0xA7;
            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {
                id = 0xA8;
            }
            func_8005C650(id, 0x7F, 0x7F);
        }
        if (D_800A37B8 == 0x43) {
            func_8005C650(0xA9, 0x7F, 0x7F);
        }
    }
    ret = func_8005E54C(D_800A3784, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((g_pad_state.pressed & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        if (D_800A382D == 2) {
            D_800A3834 = 0x18;
        } else {
            func_800372C0();
            a4val = 4;
            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {
                a4val = 5;
            }
            D_800A38A4 = a4val;
            D_800A3834 = 0x12;
        }
    }
}
/*
 * Copies the three clock records (D_80106A50.times, frame counts at 30 fps)
 * into the record at func_80077D00(): minutes (/1800), seconds ((/30) % 60),
 * hundredths ((% 30) * 100 / 30) and the record's unk_0 byte at +0x21 + i*4;
 * then three bytes from func_8001CD68 at +0x2D..+0x2F and the stage at +0x30.
 *
 * func_8001CD68 is called on the loop's exit path rather than after the loop:
 * a call inside the loop sets loop.c's loop_has_call, halving the
 * move_movables threshold (loop.c:532, loop.c:1631), so the 0x91A2B3C5 (/1800)
 * magic stays materialised in the loop while 0x88888889 (/30) is hoisted to
 * the preheader, as in the target.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    FileTimeRec *base;
    FileTimeRec *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = D_80106A50.times;
    do {
        src = &base[i];
        dst = (u8 *)s0 + i * 4;
        a = src->unk_4;
        a = a / 1800;
        dst[0x21] = a;
        b = src->unk_4;
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = src->unk_4;
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = src->unk_0;
        dst[0x24] = v;
        i += 1;
        /* FAKE: the call is spelled inside the loop on its exit path; the natural
         * spelling is a counted loop followed by the call (the target places the
         * jal right after the back-branch, asm/funcs/func_8003C714.s:77-79). Inside
         * the loop it sets loop_has_call, which keeps the /1800 magic in the loop. */
        if (i >= 3) {
            func_8001CD68(buf);
            break;
        }
    } while (1);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = D_80101EC8[0].unk_0A;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_SetDispMaskOn();
}
void func_8003C8B4(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_80060768(D_800A38B4, 1, D_800A38E9);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((g_pad_state.pressed & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        func_80033FE4();
    }
}
void func_8003C958(void) {
    gpu_InitDisplay();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    gpu_SetDispMaskOn();
}
extern void func_80046BF4(s16 *, s16 *, s32);
void func_8003C9A4(void) {
    s32 ret;
    s32 *a0 = &D_800F6608.w0;
    s16 *a1 = &D_800F6608.h10;

    func_8003F1E4(0);
    a0[0] = 0;
    D_800F6608.w4 = -0xBB8;
    D_800F6608.w8 = 0;
    *a1 = 0x20;
    D_800F6608.h14 = 0;
    D_800F6608.w18 = 0x2710;
    D_800F6608.h12 = (s16)(D_800A36AC << 2);
    func_80046BF4((s16 *)a0, a1, 0x2710);
    func_80046DA8(1);

    if (D_800A3929 == 0) {
        D_800A38B4 = D_800A38B4 + (func_8005C8A8(1, D_800A3817, D_800A38B4, 0) / 4) * 4;

        if ((g_pad_state.pressed & 0x10001000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 != 0) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((g_pad_state.pressed & 0x40004000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = 0;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }

        if ((g_pad_state.pressed & 0x400040) != 0) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
        }
        return;
    }

    if (D_800A3817 == 0) {
        ret = func_8005FA98(0, D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    }
    D_800A3929 = D_800A3929 + 1;
    if ((u8)D_800A3929 < 0x3C) return;

    func_800372C0();
    if (D_800A3817 == 0) {
        if (D_800A38DC == 5) {
            gpu_ResetGraphMode1();
            func_80020CDC();
            D_800A3874 = 0;
            func_800342A0();
            return;
        }
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF = (u8)func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        func_8001DA2C();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if (D_800A3817 == 2) {
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
void func_8003CCCC(void) {
    gpu_InitDisplay();
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    gpu_SetDispMaskOn();
}
void func_8003CD10(void) {
    s32 *a0 = (s32 *)&D_800F6608;
    s16 *a1 = (s16 *)((u8 *)a0 + 0x10);
    s32 ret;

    func_8003F1E4(0);
    a0[0] = 0;
    D_800F6608.w4 = -0xBB8;
    D_800F6608.w8 = 0;
    *a1 = 0x20;
    D_800F6608.h14 = 0;
    D_800F6608.w18 = 0x2710;
    D_800F6608.h12 = (s16)(D_800A36AC << 2);
    func_80046BF4((s16 *)a0, a1, 0x2710);
    func_80046DA8(1);

    ret = func_800600C8(D_800A391F, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 >= 0x97 || (g_pad_state.pressed & 0x400040) != 0) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
extern s32 D_800A3818;
void func_8003CE18(void) {
    s32 s0;
    s32 v0;
    s8 player;

    func_8001DA2C();
    func_800372C0();
    gpu_InitDisplay();
    gpu_ResetGraphMode1();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    func_8003E22C();
    func_8003F218(0);
    v0 = math_FovToScreenDist(0x2D);
    SetGeomScreen(v0);
    player = D_800A3748;
    {
        u16 val = (u16)D_80101EC8[player].unk_0E;
        if ((u16)(val - 6) < 2) {
            s0 = 8;
            if (player != 0) {
                s0 = 9;
            }
        } else {
            s0 = 6;
            if (player != 0) {
                s0 = 7;
            }
        }
    }
    {
        s32 *addr = &D_80101EC8[0].unk_F4.x;
        s32 result;
        if (D_800A3748 == 0) {
            addr = &D_80101EC8[1].unk_F4.x;
        }
        result = func_80022408(addr);
        D_800A3818 = result;
        func_80054884(0x16, s0, result, (s32)D_80101EC8[0].unk_12, (s32)D_80101EC8[1].unk_12, -1, -1, 0);
    }
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x1D;
    gpu_SetDispMaskOn();
}
extern void func_80021D10(s32, s32 *, s32);
extern void func_800618B4(s32 *, s32 *);
extern s32 *func_8005507C(void);
extern s32 *func_8005508C(void);
extern void func_80061064(s32 *, s32 *);
extern void func_8003B328(void);
extern void func_8003B534(s32);
extern s32 D_800A312C;
void func_8003CF84(void) {
    /* FAKE: unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out, owner rulings 2026-08-17 + 2026-08-18): reconstructs the original frame's 16-byte allocated-but-untouched leading region (compiled-out >=7-word call). SOTN-master precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. */
    volatile u32 pre_pad[4];
    s32 vec[3];
    /* FAKE: unwritten TRAILING pad (owner ruling 2026-08-18, this function only): the target frame has a second 8-byte allocated-but-untouched object above vec, which no phantom-slot producer reproduces. */
    volatile u32 pad2[2];
    s32 *a;
    s32 *b;
    s32 flag = 0;
    s8 p;
    s16 stage;

    func_800335D8();
    p = D_800A3748;
    stage = D_80101EC8[p].unk_0A;
    if (D_800A37B8 == D_8008EAC0[stage]) {
        func_8005C650(40 * p + 0x2D, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB04) {
        func_8005C650(40 * D_800A3748 + 0x31, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB06) {
        func_8005C650(40 * D_800A3748 + 0x36, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB08) {
        if (D_800A3748 == 0) {
            func_8005C650(0x53, 0x7F, 0x7F);
        } else {
            func_8005C650(0x2B, 0x7F, 0x7F);
        }
    }
    if (D_800A37B8 == D_8008EB0A) {
        func_8005C650(0x71, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB0C) {
        func_80021D10(0, vec, D_800A3818);
        vec[0] += D_8008EB10;
        vec[1] += D_8008EB14;
        vec[2] += D_8008EB18;
        func_800618B4(vec, &D_800A312C);
    }
    a = func_8005507C();
    b = func_8005508C();
    func_80061064(a, b);
    if (func_80054F68() == 0) {
        flag = 1;
    }
    if (flag != 0 || (g_pad_state.pressed & 0x400040) != 0) {
        func_800548DC();
        if (D_800A38DC == 4 || D_800A38DC == 6) {
            /* FAKE: indexes past D_800A37D2 into D_800A37D3 by player number (owner Q63, this
             * byte pair only): the target also reaches each byte through its own symbol, which
             * no single array or struct gives. */
            (&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;
        }
        func_8001979C(0, (u32 *)D_80102770);
        func_8001979C(1, (u32 *)D_801027B0[0][4]);
        func_8001979C(2, (u32 *)D_801027B0[1][4]);
        if (D_800A38DC == 0 && (u8)D_800A3836 != 0xFF) {
            func_8001DA2C();
            func_8003B328();
            func_8003AF40(0);
            func_8003AFFC();
            func_8003B534(4);
        } else {
            D_800A3834 = 0x18;
        }
    }
    D_800A37B8 = D_800A37B8 + 1;
}
void func_8003D2C4(void) {
    LoadImage((s32)&D_800A3220, (s32)&D_80090178);
}
extern s32 D_800A3218;
extern s32 D_800A321C;
/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s32 D_800A3358;
static s32 D_800A335C;
static s32 D_800A3360;
static s32 D_800A3364;
static s32 D_800A3368;
static s32 D_800A336C;
static s32 g_game_flag_a;
static s32 g_game_flag_b;

void func_8003D2F4(void) {
    s32 v0;
    s32 v1;
    D_800A3364 = 0xF0F0F0;
    v0 = D_800A3218;
    v1 = D_800A321C;
    D_800A3358 = 0;
    D_800A3360 = 0;
    D_800A335C = 0;
    D_800A3218 = (u32)v0 < 1u;
    if (v1 == 0) {
        D_800A3358 = 0x20;
    }
}
void func_8003D330(void) {
    OTag *p = (OTag *)((u8 *)&D_800A3D30 + (D_800A3218 << 3));
    OTag *ot;
    ((u8 *)p)[3] = 1;
    *((u32 *)p + 1) = 0xE100001F;
    ot = (OTag *)g_gpu_ot_ptr;
    p->addr = ot->addr;
    ot->addr = (u32)p;
}
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} Sprt8Prim;
extern Sprt8Prim D_800A3930[2][32];
void func_8003D39C(s32 x, s32 y, s32 ch, s32 color) {
    s32 n = D_800A3358;
    Sprt8Prim *p;
    OTag *ot;

    if (n == 0x20) return;
    D_800A3358 = n + 1;
    p = &D_800A3930[D_800A3218][n];
    ((u8 *)p)[3] = 3;
    p->code = 0x74;
    p->x0 = x;
    p->y0 = y;
    p->u0 = (ch & 7) * 8 - 0x40;
    p->v0 = (ch >> 5) * 8 - 0x20;
    p->clut = ((ch >> 3) & 3) << 6 | 0x773F;
    *(u32 *)&p->r0 = (color >> 1) | 0x74000000;
    ot = (OTag *)g_gpu_ot_ptr;
    ((OTag *)p)->addr = ot->addr;
    ot->addr = (u32)p;
}
void func_8003D478(s32 x, s32 y, u8 *str, s32 color) {
    s32 ch;
    s32 start_x = x;

    ch = *str++;
    if (ch == 0) return;

    do {
        if (ch == 0x20) {
            /* space - advance */
        } else if (ch == 0x0A) {
            x = start_x;
            y += 8;
            goto next_char;
        } else {
            func_8003D39C(x, y, ch, color);
        }
        x += 8;
    next_char:
        ch = *str++;
    } while (ch != 0);
}
typedef char *va_list;
#define va_start(ap, last) ((ap) = (va_list)(&(last) + 1))
#define va_arg(ap, type) ((type *)(void *)(ap += 4))[-1]

void func_8003D52C(u8 *fmt, s32 first_arg, ...) {
    u8 buf[0x400];
    u8 seg[0x100];
    va_list ap;
    s32 cur_arg;
    s32 seen_pct;
    u8 *p;
    s32 ch;

    cur_arg = first_arg;
    seen_pct = 0;
    p = seg;
    va_start(ap, first_arg);
    buf[0] = 0;

    while ((ch = *fmt++) != 0) {
        if (ch == '%') {
            if (seen_pct == 0) {
                seen_pct = 1;
            } else {
                *p = 0;
                sprintf(buf + strlen(buf), seg, cur_arg);
                p = seg;
                cur_arg = va_arg(ap, s32);
            }
        }
        *p++ = ch;
    }

    *p = 0;
    sprintf(buf + strlen(buf), seg, cur_arg);

    p = buf;
    while ((ch = *p++) != 0) {
        s32 row = D_800A3360;
        if (row >= 0x1A) break;

        if (ch == ' ') {
            D_800A335C++;
        } else if (ch == '\n') {
            D_800A335C = 0;
            D_800A3360 = row + 1;
        } else if (ch == '~') {
            ch = *p++;
            if (ch == 0) break;
            if (ch == 'c' || ch == 'C') {
                s32 d1, d2, d3;
                d1 = *p++;
                if (d1 == 0) break;
                d2 = *p++;
                if (d2 == 0) break;
                d3 = *p++;
                if (d3 == 0) break;
                D_800A3364 = ((d1 - '0') << 5) | ((d2 - '0') << 13) | ((d3 - '0') << 21);
            }
        } else {
            func_8003D39C(D_800A335C * 8 + 0x10, row * 8 + 0x10, ch, D_800A3364);
            D_800A335C++;
        }
        if (D_800A335C >= 0x4C) {
            D_800A335C = 0;
            D_800A3360++;
        }
    }
}

void func_8003D774(s32 arg0, s32 arg1) {
    s32 *ptr = (s32 *)((u8 *)&D_800A3D40 + arg1 * 24);
    ptr[0] = arg0;
    ptr[1] = 0;
    ptr[2] = 0;
    *(s16 *)((u8 *)ptr + 0x16) = 0;
    *(s16 *)((u8 *)ptr + 0x14) = 0;
    *(s16 *)((u8 *)ptr + 0x12) = 0;
    *(s16 *)((u8 *)ptr + 0x10) = 0;
    *(s16 *)((u8 *)ptr + 0xE) = 0;
    *(s16 *)((u8 *)ptr + 0xC) = 0;
}
extern s32 bitstream_ReadBits(u32 *, s32);
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    u8 *p;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        s32 sval;
        nbits = bitstream_ReadBits((s32 *)base, 4);
        if (nbits == 0) {
            nbits = 16;
        }
        val = (s16)bitstream_ReadBits((s32 *)base, nbits);
        sval = val;
        sign_bit = nbits - 1;
        if ((sval >> sign_bit) & 1) {
            val = val | (0xFFFF << sign_bit);
        }
        p = base + i * 2;
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(p + 0xC) + val);
        i++;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}

/* Bitstream reader.  State through `u32 *s`: s[0]=word pointer, s[1]=current word,
   s[2]=bits still available in s[1].  Returns the next `n` bits.
   `m1 = 1 << avail; m1 -= 1;` is the user-sanctioned same-variable split-init
   accumulation family (owner ruling 2026-06-13) -- both statements are live and the pair
   folds back into one emitted `addiu v0,v0,-1`. */
s32 bitstream_ReadBits(u32 *s, s32 n)
{
    s32 avail = s[2];
    u32 r;

    if (avail < n) {
        u32 m1, m2;
        s32 shift;
        u32 p;
        /* FAKE: `hi` names the masked high-bit slice of the freshly loaded word,
           mechanism: GCC 2.7.2 RTL expansion (expr.c expand_binop) fixes the iorsi3
           source-operand order from the C expression tree and combine preserves it --
           naming the slice moves it to operand 1 (`or v1,v1,v0`, target) without
           touching statement order, so sched1's order and the greg allocation are
           byte-identical to the un-named form, whereas swapping the operands in the
           source expression itself also moves the LUID and regresses the
           allocation. */
        u32 hi;

        n -= avail;
        m1 = 1 << avail;
        m1 -= 1;
        m2 = (1 << n) - 1;
        r = s[1] & m1;
        p = s[0];
        s[0] = p + 4;
        shift = 32 - n;
        p = *(u32 *)p;
        s[2] = shift;
        s[1] = p;
        hi = ((u32)p >> shift) & m2;
        r = (r << n) | hi;
    } else {
        s[2] = avail - n;
        r = (s[1] >> (avail - n)) & ((1 << n) - 1);
    }
    return r;
}
void gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE (*)[2]);
void func_8003D91C(void) {
    RECT buf;
    buf.x = 0;
    buf.y = 0x1E0;
    buf.w = 0x140;
    buf.h = 1;
    gpu_SetDrawMoveArray(&buf, 0x1F, light_effect_col);
    buf.w = 0x40;
    buf.x = 0x140;
    buf.y = 0x1E0;
    buf.h = 8;
    gpu_SetDrawMoveArray(&buf, 0x13, D_800A4340);
}
void gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE (*a2)[2]) {
    s32 s4, s3;
    s32 s2;

    s4 = a0->x;
    s3 = a0->y;

    s2 = a1 - 1;
    if (s2 != -1) {
        do {
            s16 v0;
            v0 = a0->y + a0->h;
            a0->y = v0;
            if (v0 >= 0x200) {
                a0->y = s3;
                a0->x = a0->x + a0->w;
            }
            SetDrawMove(&(*a2)[0], a0, s4, s3);
            (*a2)[1] = (*a2)[0];
            a2++;
        } while (--s2 != -1);
    }
}
extern void func_8003DBE4(s32, s32, DR_MOVE (*)[2], s32, s32);
void func_8003DA8C(s32 arg0, s32 arg1) {
    s16 *rec;
    s32 *ptr;
    s32 dist;
    s32 offset;

    D_800905F8 = 0xFFFF;
    /* FAKE: the pair read through a pointer local; a direct
       `D_800906A4[arg0][0]` read is MEM_IN_STRUCT and is scheduled above the
       D_800905F8 store (the same sched.c:834-839 exemption as below), score 9. */
    rec = D_800906A4[arg0];
    if (rec[0] != 0) {
        {
            s32 base = func_8003F268();
            if (base == 0) {
                base = 0x6590;
            } else {
                base = 0x55F0;
            }
            dist = base - arg1;
        }
        if (dist < 0x1770) {
            offset = 0x1770 - dist;
            dist = 0x1770;
        } else {
            offset = 0;
        }
        {
            s16 v1 = D_800F6656;
            s16 mask = D_80090608;
            if ((v1 & ~mask) & 1) {
                D_80090600 = dist;
                D_80090604 = offset;
            }
            if (v1 & 1) {
                arg1 -= D_80090604;
            } else {
                arg1 -= offset;
            }
        }
        D_80090608 = (u16)D_800F6656;
        ptr = &D_8009060C[arg0];
        {
            /* FAKE: D_800906A4[arg0][1] read as a byte offset off the array
               base: an element read is MEM_IN_STRUCT, and sched.c
               true_dependence (tools/gcc-2.7.2/sched.c:834-839) lets a
               varying in-struct HImode read pass the fixed scalar store to
               D_80090608 just above, so both reads move ahead of the `sh`
               (score 8).  The target keeps them after it. */
            s32 idx = arg0 * 4;
            func_8003DBE4(arg1, 0x1F, light_effect_col, *ptr, *(s16 *)((u8 *)D_800906A4 + 2 + idx));
            func_8003DBE4(arg1, 0x13, D_800A4340, *ptr, *(s16 *)((u8 *)D_800906A4 + 2 + idx));
        }
    }
}
void func_8003DBE4(s32 arg0, s32 arg1, DR_MOVE (*arg2)[2], s32 arg3, s32 arg4) {
    s32 limit;
    s32 step;
    DR_MOVE *pkt;
    s32 i;

    pkt = *arg2;

    if (arg4 != 0) {
        limit = arg1;
    } else {
        limit = arg1 - 1;
    }

    if (D_800F6656 & 1) {
        step = D_80090600;
    } else {
        if (func_8003F268() == 0) {
            step = 0x6590 - arg0;
        } else {
            step = 0x55F0 - arg0;
        }
    }

    if (step < 0) {
        return;
    }

    if ((u32)arg3 < (u32)step) {
        step = (s32)((u32)arg3 / (u32)arg1);
    } else {
        step = step / arg1;
    }

    i = 0;
    pkt += D_800A36AC & 1;

    if (i < limit) {
        do {
            s32 idx = func_80052C28((u32)arg0 >> 2, 2);
            if (idx < 0x1000) {
                /* FAKE: SDK addPrim (setaddr/getaddr P_TAG views) on OT entry idx; the explicit
                 * mask-and-or spelling of the two stores scores 14. */
                /* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a (PS1 use: src/main/psxsdk/libgpu/sys.c:288) */
                ((OTag *)pkt)->addr = ((OTag *)&D_800A378C[idx])->addr;
                ((OTag *)&D_800A378C[idx])->addr = (u32)pkt;
                pkt += 2;
                if (i == limit - 1) {
                    D_800905F8 = idx;
                }
            }
            arg0 += step;
            i++;
        } while (i < limit);
    }

    if (arg4 != 0) {
        func_8003DDF8((u32)pkt);
    } else {
        ((OTag *)pkt)->addr = ((OTag *)&D_800A378C[0xFFB])->addr;
        ((OTag *)&D_800A378C[0xFFB])->addr = (u32)pkt;
    }
}
void func_8003DDF8(u32 arg0) {
    u32 *ptr = D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}
/* func_8003DE14 - builds count-1 progressively fogged copies of the VRAM
 * rectangle `rect`. The original is first copied one rect height up; each
 * pass then blends every non-zero pixel toward the GTE far colour by
 * (i + 1) / count (the last pass writes the far colour itself) and uploads
 * the result one rect height below the previous one, wrapping to the next
 * column at y 0x200.
 *
 * FAKE constructs (each labelled at its site): the `gm` named green mask, the
 * inner bound's `+ rect[2] - rect[2]` detour and the `((s32)dst_buf + j) - j`
 * LoadImage argument. The two detours are combine-foldable chain extenders
 * ([[dead-store-fake-exception]]) with zero emitted bytes.
 */
void func_8003DE14(s16 *rect, s32 count) {
    u16 src_buf[0x200];
    u16 dst_buf[0x200];
    u8 color_info[0x20];
    s32 i;
    s32 saved_y;
    s32 r;
    s32 g;
    s32 b;
    s32 target_color;

    DrawSync(0);
    count--;
    StoreImage((s32 *)rect, src_buf);
    DrawSync(0);
    ((u16 *)rect)[1] -= ((u16 *)rect)[3];
    LoadImage((s32)rect, (s32)src_buf);
    saved_y = rect[1];
    rect[1] = ((u16 *)rect)[3] + saved_y;
    gte_ReadFarColor(color_info);

    r = color_info[0];
    g = color_info[1];
    b = color_info[2];
    target_color = (((u32)r >> 3) | (s32)-0x8000) | ((g & 0xF8) << 2) | ((b & 0xF8) << 7);

    i = 0;
    if (count > 0) {
        s32 blend_base = 0x1000;
        do {
            s32 total = rect[2] * rect[3];
            u16 *src = src_buf;
            u16 *dst = dst_buf;
            s32 factor = ((i + 1) << 12) / count;
            s32 j = 0;
            if (total > 0) {
                s32 complement = blend_base - factor;
                do {
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst = pixel;
                            src++;
                            dst++;
                            goto loop_check;
                        }
                        *dst++ = target_color;
                        src++;
                        goto loop_check;
                    }
                    {
                        u16 pixel = *src;
                        s32 px = pixel & 0xFFFF;
                        if (px == 0) {
                            *dst = pixel;
                            src++;
                            dst++;
                            goto loop_check;
                        }
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 sum;
                            s32 rp;
                            s32 gp;
                            /* FAKE: `gm` names the green channel's masked result so that
                             * g_src dies at the mask instead of at the store; mechanism:
                             * global.c allocno priority (prio = nrefs*40000/live_length)
                             * - naming gm takes green from 18 refs/livelen 18 to 18/16 and
                             * red from 24/21 to 24/22, so pri(px)=44444 > pri(red)=43636
                             * and px is allocated $a0 with red $a1 and green $v1, the
                             * target's seat map; without the name red is 24/21=45714,
                             * outranks px and steals $a0. */
                            s32 gm;
                            src++;
                            rp = r_src * complement;
                            r_src = r * factor;
                            sum = rp + r_src;
                            r_src = sum >> 15;
                            r_src = r_src & 0x1F;
                            gp = g_src * complement;
                            g_src = g * factor;
                            sum = gp + g_src;
                            g_src = sum >> 10;
                            px = ((u32)px >> 7) & 0xF8;
                            px = px * complement;
                            sum = px + b * factor;
                            px = sum >> 5;
                            gm = g_src & 0x3E0;
                            *dst = (pixel & 0x8000) | r_src | gm | (px & 0x7C00);
                        }
                    }
                    dst++;
                loop_check:
                    j++;
                /* FAKE: the inner loop's bound is routed through the algebraically
                 * equivalent detour `+ rect[2] - rect[2]`, which combine folds back to the
                 * direct `rect[2] * rect[3]` with ZERO emitted bytes.  Its only surviving
                 * effect is the extra reg_n_refs that flow.c records BEFORE the fold.
                 * Mechanism: local-alloc.c:1669-1684 `qty_compare_1` ranks the two
                 * block-local halfword loads of the bound by
                 * floor_log2(n_refs)*n_refs*size/(death-birth).  Both loads die at the
                 * shared `mult`, so the earlier-born rect[2] load has the strictly larger
                 * denominator: at the natural 2 refs each (weighted x3 for loop depth =
                 * 6) it scores floor_log2(6)*6/4 = 3 against the rect[3] load's
                 * floor_log2(6)*6/2 = 6, is sorted second, and is handed $v1 instead of the
                 * target's $v0.  The detour's two extra reads CSE onto the same pseudo, so
                 * flow counts 4 refs (weighted 12) and it scores floor_log2(12)*12/4 = 9 >
                 * 6, sorts first and takes $v0 - the target's map `lh $v0,4($s0)` /
                 * `lh $v1,6($s0)` / `mult $v0,$v1`.  Same family and same mechanism as
                 * the `((s32)dst_buf + j) - j` extender below
                 * ([[dead-store-fake-exception]] combine-foldable chain-extender clause,
                 * owner ruling 2026-07-01). */
                } while (j < rect[2] * rect[3] + rect[2] - rect[2]);
            }

            {
                s32 new_y = ((u16 *)rect)[1] + ((u16 *)rect)[3];
                ((u16 *)rect)[1] = new_y;
                if ((s16)new_y >= 0x200) {
                    rect[1] = saved_y;
                    ((u16 *)rect)[0] += ((u16 *)rect)[2];
                }
            }
            /* FAKE: j chain extender on the dst_buf argument; mechanism:
             * combine.c folds the +j/-j pair away but flow.c's reg_n_refs for j is
             * counted before it, lifting j's allocno priority (global.c
             * allocno_compare) above `complement`'s so the $t4/$t5 seat pair
             * matches; without it j and complement swap registers. */
            LoadImage((s32)rect, ((s32)dst_buf + j) - j);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
void func_8003E0E0(void) {
    s16 buf[4];
    buf[1] = 0x1E1;
    buf[2] = 0x140;
    buf[0] = 0;
    buf[3] = 1;
    func_8003DE14(buf, 0x1F);
}
void func_8003E120(void) {
    s16 buf[4];
    buf[0] = 0x140;
    buf[1] = 0x1E8;
    buf[2] = 0x40;
    buf[3] = 8;
    func_8003DE14(buf, 0x13);
}
extern s32 D_800A3228;
extern void MoveImage(s16 *, s32, s32);
void func_8003E164(s32 arg0) {
    s16 buf[4];
    s32 *s0;

    if (D_800A3228 == arg0) {
        goto end;
    }
    func_8003E22C();
    s0 = func_8004153C(arg0);
    if (s0 == 0) {
        goto end;
    }
    if (arg0 != 0) {
        buf[0] = 0x300;
    } else {
        buf[0] = 0x280;
    }
    buf[1] = 0xF8;
    buf[2] = 0x40;
    buf[3] = 6;
    MoveImage(buf, 0x140, 0x1E8);
    if (arg0 == 0) {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x140, 0xE8);
    } else {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x1C0, 0xE8);
    }
    DrawSync(0);
    func_8003E120();
end:
    D_800A3228 = arg0;
}
void func_8003E22C(void) {
    s32 *v1;

    if (D_800A3228 != -1) {
        v1 = func_8004153C(D_800A3228);
        if (v1 != 0) {
            if (D_800A3228 == 0) {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x140, -0xE8);
            } else {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x1C0, -0xE8);
            }
        }
        D_800A3228 = -1;
    }
}
s32 func_8003E2A0(void) {
    return D_800A3228;
}
void func_8003E2AC(void) {
    u16 *p = (u16 *)&D_800F6656;
    *p = *p & 0xFFFD;
}
u32 func_8003E2C8(void) {
    return D_800905F8;
}
extern void func_8003F388(s16 *arg0);
void func_8003E2D8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_fp;
    s32 temp_lo;
    s32 temp_s2;
    s32 temp_s7;
    s32 temp_t0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 var_s0;
    s32 var_s1;
    s32 var_s3;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;

    temp_t0 = arg0 + 0x7D00;
    temp_s2 = arg1 + 0x7D00;
    temp_a2 = arg2 + 0x7D00;
    temp_a3 = arg3 + 0x7D00;
    arg0 = temp_a2 - temp_t0;
    arg1 = temp_a3 - temp_s2;
    var_s1 = temp_t0 / 2000;
    var_s0 = temp_s2 / 2000;
    temp_fp = temp_a2 / 2000;
    temp_s7 = temp_a3 / 2000;
    if ((arg0 == 0) && (arg1 == 0)) {
        if ((var_s1 >= 0) && (var_s0 >= 0) && (var_s1 < 0x20) && (var_s0 < 0x20)) {
            sp10[0] = var_s1 - 0x10;
            sp10[2] = var_s0 - 0x10;
            func_8003F388(sp10);
        }
    } else {
        temp_v1 = (var_s1 * 0x7D0) + 0x3E8;
        temp_t0 -= temp_v1;
        temp_v0 = (var_s0 * 0x7D0) + 0x3E8;
        temp_s2 -= temp_v0;
        temp_a2 -= temp_v1;
        temp_a3 -= temp_v0;
        if (arg0 < 0) {
            var_s6 = -1;
            arg0 = -arg0;
            temp_t0 = -temp_t0;
            temp_a2 = -temp_a2;
        } else {
            var_s6 = 1;
        }
        var_s5 = 1;
        if (arg1 < 0) {
            var_s5 = -1;
            arg1 = -arg1;
            temp_s2 = -temp_s2;
            temp_a3 = -temp_a3;
        }
        if (arg0 < arg1) {
            temp_v0_2 = arg0;
            arg0 = arg1;
            arg1 = temp_v0_2;
            temp_v0_3 = temp_t0;
            temp_t0 = temp_s2;
            temp_s2 = temp_v0_3;
            temp_a2 = temp_a3;
            var_s4 = 1;
        } else {
            var_s4 = 0;
        }
        temp_lo = (s32)(arg1 << 0xC) / arg0;
        temp_a2 += 0x3E8;
        temp_t0 += 0x3E8;
        var_s3 = (temp_a2 / 2000) - (temp_t0 / 2000);
        temp_s2 += 0x3E8;
        temp_s2 -= (s32)(temp_t0 * temp_lo) >> 0xC;
        sp20 = (s32)(temp_lo * 0x7D0) >> 0xC;
        if (var_s3 != -1) {
loop_16:
            if (var_s1 >= 0) {
                if ((var_s0 >= 0) && (var_s1 < 0x20) && (var_s0 < 0x20)) {
                    sp10[0] = var_s1 - 0x10;
                    sp10[2] = var_s0 - 0x10;
                    func_8003F388(sp10);
                }
            }
            temp_s2 %= 2000;
            temp_s2 += sp20;
            if (var_s3 != 0) {
                if (temp_s2 >= 0x7D1) {
                    if (var_s4 != 0) {
                        if (var_s6 < 0) {
                            var_s1 -= 1;
                        } else {
                            var_s1 += 1;
                        }
                    } else if (var_s5 < 0) {
                        var_s0 -= 1;
                    } else {
                        var_s0 += 1;
                    }
                    if ((var_s1 >= 0) && (var_s0 >= 0) && (var_s1 < 0x20) && (var_s0 < 0x20)) {
                        sp18[0] = var_s1 - 0x10;
                        sp18[2] = var_s0 - 0x10;
                        func_8003F388(sp18);
                    }
                }
                if (var_s4 != 0) {
                    if (var_s5 < 0) {
                        var_s0 -= 1;
                    } else {
                        var_s0 += 1;
                    }
                } else if (var_s6 < 0) {
                    var_s1 -= 1;
                } else {
                    var_s1 += 1;
                }
                var_s3 -= 1;
                if (var_s3 != -1) {
                    goto loop_16;
                }
            }
        }
        if ((temp_fp >= 0) && (temp_s7 >= 0) && (temp_fp < 0x20) && (temp_s7 < 0x20)) {
            sp10[0] = temp_fp - 0x10;
            sp10[2] = temp_s7 - 0x10;
            func_8003F388(sp10);
        }
    }
}
void func_8003E6A0(s32 arg0, s32 arg1) {
    func_8003E2D8(D_80101DF0.work.t[0], D_80101DF0.work.t[2], arg0, arg1);
}
extern Unk80101DF0Record *D_800A3708;
extern s32 D_800A322C;
extern s32 D_800927C0[64][32];
extern s32 D_80090740[64][32];
extern s32 D_80094840[];
extern s32 D_800A7EF0[];
extern s32 *func_8003EB84(s32, s32, s32 *);
extern s16 *camera_CalcAngles(void);
extern void math_RotMatrixYXZ(Unk80101DF0Rot *, s32 *);
extern s32 ratan2(s32, s32);
extern void func_800620B8(s16 *, s32 *);
/* func_8003E6D8 - grid pass. D_800A3708's xf.rot is turned into a matrix
 * (func_80042A88) and applied to {0,0,0x1000}; ratan2 of the result, stored
 * to D_800A336C, picks ((a >> 6) & 0x3F) one of 64 tables of 32-bit row
 * masks (ANDed with D_80094840 into the scratchpad when neither D_800A322C
 * nor P1 bit 0 is set). The 31x31 grid window centred on the cell of
 * D_800A3708's work.t ((t + 0x7D00) / 2000) is walked, and each cell whose
 * mask bit is set emits its records, as in func_8003EB84.
 *
 * COMPLETED-INLINE-ASM-CANONICAL: pure-C body plus four PsyQ SDK GTE macro
 * islands (the sequence gte_ApplyMatrix expands to -- gtemac.h: SetRotMatrix,
 * ldv0, rtv0, stlvnl -- with its gte_rtv0 step cited below as the identical
 * word gte_mvmva(1,0,0,3,0)), the same spelling and clobber
 * provenance as func_80019310 (src/code6cac.c) and func_800203B4:
 *   gte_SetRotMatrix(r0) -- inline_c.h:297-310 (clobbers as published + $15)
 *   gte_ldv0(r0)         -- inline_c.h:16-20; publishes no clobbers, "$12" and
 *                           "memory" ADDED (it reads vec[] through $12),
 *                           precedent func_80019310 / func_8002D320
 *   gte_mvmva(1,0,0,3,0) -- inline_c.h:816-817 (body gte_mvmva_core,
 *                           :809-814); its two leading nops ride at the
 *                           tail of the gte_ldv0 island, as in func_80019310
 *   gte_stlvnl(r0)       -- inline_c.h:1111-1117; "memory" is its own clobber
 */
void func_8003E6D8(s32 arg0) {
    s32 mat[8];
    s16 vec[4];
    s32 res[3];
    s16 pos[4];
    s32 cx;
    s32 cz;
    s16 x;
    s16 z;
    s32 *mask;
    s32 *out;
    s32 i;
    s32 j;
    s32 bits;
    s16 row;
    s16 col;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    Unk800A4750Rec *e;
    Unk800A6690Rec *e2;
    s32 *list;
    s32 ang;
    s32 idx;

    cx = (D_800A3708->work.t[0] + 0x7D00) / 2000;
    cz = (D_800A3708->work.t[2] + 0x7D00) / 2000;
    camera_CalcAngles();
    math_RotMatrixYXZ(&D_800A3708->xf.rot, mat);
    vec[2] = 0x1000;
    vec[0] = 0;
    vec[1] = 0;
    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_ldv0(r0) --- inline_c.h:16-20 (the lwc2
     * pair). The two nops that follow are NOT gte_ldv0 text: they are the
     * leading `nop; nop` of gte_mvmva_core (inline_c.h:809-814), carried at
     * the tail of this island exactly as func_80019310 does. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $0, 0($12)\n"
        "lwc2   $1, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "memory");
    /* Sony libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817, whose body
     * is gte_mvmva_core(r0) at inline_c.h:809-814 (`nop; nop; .word <literal>`).
     * This instance is gte_mvmva(1,0,0,3,0): sf=1, mx=rotation, v=V0, cv=none,
     * lm=0. It is spelled as the bare `.word 0x4A486012`, the final cop2 word:
     * the macro's own literal is a DMPSX placeholder that Sony's dmpsx post-pass
     * rewrites, and this build has no such pass. The core's two nops are
     * carried at the tail of the gte_ldv0 island above (func_80019310's form). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r0) --- inline_c.h:1111-1117. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(res) : "$12", "memory");
    ang = ratan2(res[0], res[2]);
    D_800A336C = ang;
    x = cx - 0xF;
    z = cz - 0xF;
    idx = (ang >> 6) & 0x3F;
    if (D_800A322C != 0) {
        mask = D_800927C0[idx];
    } else if (D_800F6656 & 1) {
        mask = D_80090740[idx];
    } else {
        s32 *src2;
        s32 *dst;
        src2 = D_80094840;
        mask = D_80090740[idx];
        dst = (s32 *)0x1F800004;
        for (i = 0; i < 0x1F; i++) {
            *dst++ = *src2++ & *mask++;
        }
        mask = (s32 *)0x1F800004;
    }

    out = D_800A7EF0;
    for (i = 0; i < 0x1F; i++) {
        row = z + i;
        if (row < 0) {
            mask++;
            continue;
        }
        if (row >= 0x20) {
            break;
        }
        bits = *mask++;
        if (bits == 0) {
            continue;
        }
        bits <<= 1;
        for (j = 0; j < 0x1F; j++) {
            col = x + j;
            if (col < 0) {
                /* FAKE: the column shift is written in both arms instead of
                 * once in the for-increment. jump2 cross-jumps the two copies
                 * back into the one shift in the loop branch's delay slot
                 * (bytes unchanged); the second copy lifts bits' reg_n_refs
                 * 17->23 so global-alloc ranks it above a3 (pri 11500 vs
                 * 8823), giving the target's bits->$t1 / a3->$t2. */
                bits <<= 1;
                continue;
            }
            if (col >= 0x20) {
                break;
            }
            if (bits < 0) {
                vidx = D_800A7FE0[row][col];
                if (vidx >= 0) {
                    a3 = g_stage_collision[row * 0x20 + col];
                    do {
                        t0 = D_800A87E0[vidx++];
                        v1 = t0 & 0x7FFF;
                        if (v1 < D_800A3368) {
                            e = &D_800A4750[v1];
                            e->unk6 = a3 & 3;
                            if ((a3 & 8) || ((a3 & 4) && (e->unk7 & 8))) {
                                e->unk7 |= 1;
                            } else {
                                e->unk7 &= 0xFE;
                            }
                            list = (s32 *)D_800A3820;
                            D_800A3820 = (s32)(list + 1);
                            *list = (s32)e;
                        } else {
                            e2 = &D_800A6690[v1 - D_800A3368];
                            if (e2->unk58 == 0) {
                                *out++ = (s32)e2;
                                e2->unk58 = 1;
                            }
                        }
                    } while (!(t0 & 0x8000));
                }
            }
            bits <<= 1;
        }
    }

    if ((D_800F6656 & 1) && D_800A322C == 0) {
        out = func_8003EB84(x, z, out);
    }
    while (out != D_800A7EF0) {
        out--;
        *(s32 *)D_800A3820 = *out;
        (*(Unk800A6690Rec **)D_800A3820)->unk58 = 0;
        D_800A3820 += 4;
    }
    if (g_stage_init_tbl[stage_GetId()].unk4 != 0) {
        g_stage_init_tbl[stage_GetId()].unk4();
    }
    {
        Unk80101DF0Xform *cam = &D_80101DF0.xf;
        pos[0] = -cam->rot.vx;
        pos[1] = -cam->rot.vy;
        pos[2] = -cam->rot.vz;
        func_800620B8(pos, cam->mat.t);
    }
}
s32 *func_8003EB84(s32 a0, s32 a1, s32 *out) {
    s32 sp[0x20];
    s32 mask;
    s32 i;
    s32 t4;
    s32 t1;
    s32 t2;
    s16 temp_v0;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    Unk800A4750Rec *e;
    Unk800A6690Rec *e2;
    s32 *list;

    if (a0 >= 0) {
        mask = -1;
        if (a0 >= 0x20) {
            goto skip;
        }
        goto calc;
    } else {
        mask = -1;
        if (-a0 < 0x20) {
        calc:
            if (a0 < 0) {
                mask = -1U >> (a0 + 0x1F);
            } else if (a0 == 0) {
                mask = 0;
            } else {
                mask = -1 << (0x20 - a0);
            }
        }
    }
skip:
    for (i = 0; i < 0x20; i++) {
        if (i >= a1 && i < a1 + 0x1F) {
            sp[i] = mask;
        } else {
            sp[i] = -1;
        }
    }

    for (t4 = 0; t4 < 0x20; t4++) {
        t2 = sp[t4];
        if (t2 != 0) {
            for (t1 = 0; t1 < 0x20; t1++) {
                if (t2 < 0) {
                    temp_v0 = D_800A7FE0[t4][t1];
                    vidx = temp_v0;
                    if (temp_v0 >= 0) {
                        a3 = g_stage_collision[t4 * 0x20 + t1];
                        do {
                            t0 = D_800A87E0[vidx++];
                            v1 = t0 & 0x7FFF;
                            if (v1 < D_800A3368) {
                                e = &D_800A4750[v1];
                                e->unk6 = a3 & 3;
                                if ((a3 & 8) || ((a3 & 4) && (e->unk7 & 8))) {
                                    e->unk7 |= 1;
                                } else {
                                    e->unk7 &= 0xFE;
                                }
                                list = (s32 *)D_800A3820;
                                D_800A3820 = (s32)(list + 1);
                                *list = (s32)e;
                            } else {
                                e2 = &D_800A6690[v1 - D_800A3368];
                                if (e2->unk58 == 0) {
                                    *out = (s32)e2;
                                    out += 1;
                                    e2->unk58 = 1;
                                }
                            }
                        } while (!(t0 & 0x8000));
                    }
                }
                t2 = t2 << 1;
            }
        }
    }

    return out;
}
extern s16 D_800A3678;
extern s32 D_800A3230;
extern void func_80052C10();
/* func_8003EDC0 - unpacks a stream of u16 words in sections ended by -1:
 * (cell, id) pairs into the 16-byte record table (count kept in D_800A3368),
 * the transform-node records (header word, xf.mat.t, xf.rot; each node is
 * then passed to its g_anim_func_table handler), then the 32x32 cell grid,
 * each listed cell indexing a run of record ids (bit 15 ends a run) in the
 * run table. Cells are numbered row * 32 + column. */
void func_8003EDC0(u16 *p, s32 arg1) {
    Unk800A4750Rec *r;
    Unk800A6690Rec *c;
    s32 i;
    s32 j;
    s16 n;
    s16 w; /* the stream word just read */
    s16 x;
    s16 z;
    u16 tok;

    n = 0;
    while ((w = *p++) != -1) {
        r = &D_800A4750[n++];
        tok = *p++;
        r->unk4 = tok;
        if (tok & 0x8000) {
            r->unk4 = tok & 0x7FFF;
            r->unk7 = 8;
        } else {
            r->unk7 = 0;
        }
        r->unk0 = 0xC;
        r->unk6 = 0;
        r->unk2 = arg1;
        r->unk8 = (w % 32) * 2000 - 32000;
        r->unkC = (w / 32) * 2000 - 32000;
    }
    D_800A3368 = n;
    n = 0;
    while ((w = *p++) != -1) {
        c = &D_800A6690[n++];
        c->node.unk1 = 0;
        c->node.unkC = 0;
        c->node.unk8 = 0;
        c->node.unkA = 4;
        c->node.unk2 = w;
        c->node.unk4 = arg1;
        c->node.xf.mat.t[0] = *p++;
        c->node.xf.mat.t[0] |= *p++ << 16;
        c->node.xf.mat.t[1] = *p++;
        c->node.xf.mat.t[1] |= *p++ << 16;
        c->node.xf.mat.t[2] = *p++;
        c->node.xf.mat.t[2] |= *p++ << 16;
        c->node.xf.rot.vx = *p++;
        c->node.xf.rot.vy = *p++;
        c->node.xf.rot.vz = *p++;
        if (c->node.xf.rot.vz != (c->node.xf.rot.vx == c->node.xf.rot.vy)) {
            c->node.unk0 = 1;
        } else {
            c->node.unk0 = 0;
        }
        g_anim_func_table[c->node.unk8](&c->node.xf.rot, &c->node.xf.mat);
        c->unk58 = 0;
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            D_800A7FE0[i][j] = -1;
        }
    }
    /* FAKE: the grid row counter i is reused as the run-table fill index; a
     * fresh index local moves the row counter from $a3 to $a1. */
    i = 0;
    while ((w = *p++) != -1) {
        x = w % 32;
        z = w / 32;
        D_800A7FE0[z][x] = i;
        do {
            w = *p++;
            D_800A87E0[i++] = w;
        } while (!(w & 0x8000));
    }
    D_800A3230 = (D_800A3230 >= i) ? D_800A3230 : i;
    if (D_800A3230 >= 1000) {
        /* FAKE: the argument is unproven -- the halt stub ignores its
         * arguments (func_8003FA24 passes it a string). The target holds the
         * count in $a0 at this jal; without the argument it is given $v1. */
        func_80052C10(D_800A3230);
    }
    D_800A3678 = 0;
    D_800A367A = 0;
    D_800A367C = 0;
}

/* ---- merged from config.c (owner ruling Q65: one original file) ---- */
/* Rodata owned by config.c per func_8003FA24's reference at asm/funcs/func_8003FA24.s:240-241.
 * Re-attributed from asm/data/101C.rodata_c2_post.s. Named per named_syms.txt alias g_str_multipul_model_80010D8C.
 * Fixed [16] to match the asm/data block's exact byte content (14 chars + null + 1 pad). */
const char D_80010D8C[16] = "Multipul Model";

/* Forward declarations */
extern void sys_StubEmpty3(s32, s32, s32);
extern void obj_ClearAll(void);
extern void sys_StubEmpty2(void);
extern void obj_Clear(s32);

/* Externs for globals */
extern void func_8001924C(s32 *, s32);
extern void func_80045A28(s32, s32);
extern void gte_SetMatrixRotTransIR(s32 *, s32 *, s16 *);

/* Externs for globals */
extern s32 D_80094A6C[];

/* --- Functions 0x8003F168 - 0x8004019C --- */
void stage_ExecInitFunc(void) {
    if (g_stage_init_tbl[stage_GetId()].init != 0) {
        g_stage_init_tbl[stage_GetId()].init();
    }
}
s32 func_8003F1C8(void) {
    return D_800A336C;
}

void *game_GetCharData(void) {
    return D_800A6690;
}

void func_8003F1E4(s32 a0) {
    if (a0) {
        D_800F6656 = 3;
        D_800F6658 = 2;
    } else {
        D_800F6656 = 0;
        D_800F6658 = 1;
    }
}

void func_8003F218(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == D_800A322C) {
        return;
    }
    D_800A322C = a0;
    if (!a0) {
        func_8003F1E4(0);
    }
    g_game_mirror_mode = (s16)D_800A322C;
}
s32 func_8003F268(void) {
    return D_800A322C;
}
void stage_InitCollision(void) {
    s32 i, j;
    s32 col_center, row_center;
    s32 data;
    s32 adj_i;
    s32 adj_j;
    s32 *ptr = (s32 *)g_stage_collision;

    i = 0xFF;
    do {
        *ptr = 0;
        i--;
        ptr++;
    } while (i >= 0);

    func_8003F268();

    col_center = (D_800A3708->work.t[0] + 0x7D00) / 2000;
    row_center = (D_800A3708->work.t[2] + 0x7D00) / 2000;

    for (i = 0; i < 16; i++) {
        adj_i = i - 8;
        {
            u32 y = (u32)(row_center + adj_i);
            if (y < 0x20) {
                data = D_80094A6C[i];
                for (j = 0; j < 16; j++) {
                    adj_j = j - 8;
                    {
                        u32 x = (u32)(col_center + adj_j);
                        if (x < 0x20) {
                            s32 bits = (data >> ((15 - j) * 2)) & 3;
                            g_stage_collision[(y << 5) + x] |= bits;
                        }
                    }
                }
            }
        }
    }
}
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x8;
    }
}
void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    s32 s0;
    a0 += 0x7D00;
    a1 += 0x7D00;
    s3 = a0 / 2000;
    s1 = a0 - s3 * 2000;
    s0 = a1 / 2000;
    s2 = s0;
    s0 = a1 - s2 * 2000;
    if (s1 < 1000) {
        s1 = -1;
    } else {
        s1 = 1;
    }
    if (s0 < 1000) {
        s0 = -1;
    } else {
        s0 = 1;
    }
    stage_SetCollision(s3, s2, 2);
    stage_SetCollision(s3 + s1, s2, 2);
    stage_SetCollision(s3, s2 + s0, 2);
    stage_SetCollision(s3 + s1, s2 + s0, 2);
}

void stage_SetCollision(s32 a0, s32 a1, s32 a2) {
    g_stage_collision[a1 * 32 + a0] = a2 & 3;
}

u32 stage_GetCollision(s32 a0, s32 a1) {
    return g_stage_collision[a1 * 32 + a0];
}

void stage_ClearLighting(void) {
    g_game_flag_b = 0;
    g_game_flag_a = 0;
    g_stage_light_pos[2] = 0;
    g_stage_light_pos[1] = 0;
    g_stage_light_pos[0] = 0;
    g_stage_light_dir[2] = 0;
    g_stage_light_dir[1] = 0;
    g_stage_light_dir[0] = 0;
}

void stage_SetLightPosDir(s32 a0, s32 a1, s32 a2) {
    g_stage_light_pos[a2] = a0;
    g_stage_light_dir[a2] = a1;
}

void stage_ApplyLighting(void) {
    sys_StubEmpty3(g_stage_light_pos[0], g_stage_light_dir[0], 0);
    sys_StubEmpty3(g_stage_light_pos[1], g_stage_light_dir[1], 1);
    sys_StubEmpty3(g_stage_light_pos[2], g_stage_light_dir[2], 2);
}

void func_8003F62C(s32 *a0) {
    s16 *s0;
    s32 *s1 = a0;
    s0 = (s16 *)s1[9];
    if (s0 == 0) return;
    if (s0[3]) {
        func_8004016C(*(s16 *)((u8 *)s1 + 4));
        func_8003F824(s1, 0);
    }
    if (s0[1]) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8(s0);
    func_8001924C((s32 *)((u8 *)s0 + 0x418), s0[0]);
    if (s0[1]) {
        func_80040068((u8 *)s0);
        s0[1] = 0;
    }
}
typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[5];
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ s32 quads[3][4];
} Func8003F6D8Inner; /* size 0xB4; sits at +0x1C of each 0xD0-byte record, records start at arg0+8 */

void func_8003F6D8(s16 *arg0) {
    s32 i;
    s32 j;

    for (i = 0; i < arg0[0]; i++) {
        s32 off = i * 0xD0 + 8;
        Func8003F6D8Inner *in = (Func8003F6D8Inner *)((u8 *)arg0 + off + 0x1C);
        for (j = 0; j < in->count; j++) {
            s32 *obj = in->objs[j] + 6;
            gte_SetMatrixRotTransIR(obj, in->quads[j], in->pairs[j]);
            gte_SetMatrixRotTransIR(obj, in->quads[j] + 2, in->pairs[j] + 8);
        }
    }
}

void func_8003F7F4(void) {
    obj_ClearAll();
    sys_StubEmpty2();
    g_game_flag_a = 0;
    g_game_flag_b = 0;
}
typedef struct {
    /* 0x00 */ s32 v[4];
} SceneQuad;

typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ SceneQuad quad;
    /* 0x14 */ u8 *obj;
    /* 0x18 */ u8 *cur;
    /* 0x1C */ Func8003F6D8Inner inner;
} SceneRec; /* size 0xD0 */

typedef struct {
    /* 0x000 */ s16 count;
    /* 0x002 */ s16 unk2;
    /* 0x004 */ s16 unk4;
    /* 0x006 */ s16 unk6;
    /* 0x008 */ SceneRec recs[5];
    /* 0x418 */ SceneQuad quads[5];
    /* 0x468 */ u8 data[1];
} Scene;

extern u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur);
void func_8003FECC(s32 *a0, s32 *a1, s16 *a2);

void func_8003F824(u8 *arg0, s32 arg1) {
    Scene *sc;
    s16 *cmds;
    u8 *cur;
    SceneRec *rec;
    u8 *obj;
    s32 i;
    s16 c;

    sc = *(Scene **)(arg0 + 0x24);
    if (sc == 0) return;
    cmds = *(s16 **)(arg0 + 0x28);
    cur = sc->data;
    if (*cmds == -3) {
        *(Scene **)(arg0 + 0x24) = 0;
        return;
    }
    sc->count = 0;
    sc->unk2 = 0;
    sc->unk4 = 0;
    sc->unk6 = 0;
    for (i = 0; *cmds != -3; i++) {
        c = *cmds;
        if (c != -2) {
            if (sc->count >= 5) {
                func_80052C10();
            }
            rec = &sc->recs[sc->count];
            obj = ((u8 **)(arg0 + 0x1A34))[i];
            rec->cur = cur;
            rec->obj = obj;
            cur = func_8003FA24(rec, cmds, cur);
            sc->quads[sc->count] = rec->quad;
            *obj = 0xD;
            sc->count++;
            if (*cmds >= 0) {
                cmds++;
                while (*cmds >= 0) {
                    cmds++;
                }
            }
            if (*cmds == -1) {
                cmds++;
                func_8003FECC((s32 *)arg0, (s32 *)rec, cmds);
            }
        }
        while (*cmds++ != -2) {
        }
    }
    if (arg1) {
        func_80045A28(*(s16 *)(arg0 + 4), cur - *(u8 **)(arg0 + 0x1C));
    }
}
extern s32 obj_CalcOffset(s32, s32);
extern s32 func_80017D84(u8 *);
extern u16 **D_80103608[];
extern s16 D_80094AEC[];
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2);

u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur) {
    struct SceneObjInit {
        s16 count;
        s16 flags;
        u8 *points;
        s16 *groups;
        void *matrix;
        u8 *point_end;
        s32 unk14;
        s32 unk18;
        s32 unk1C;
    } init;
    u8 *obj;
    u16 *src;
    u16 *block;
    s16 count;
    u16 flags;
    s32 mode;
    s32 point_count;
    s32 value;
    s32 n;
    s16 *packet;
    u16 *dst;

    obj = rec->obj;
    dst = (u16 *)cur;
    src = D_80103608[*(s16 *)(obj + 4)][*(s16 *)(obj + 2)];
    count = *src;
    init.count = count;
    init.points = cur;
    src += 2;
    count--;
    if (count != -1) {
        do {
            dst[0] = *src++;
            dst[1] = *src++;
            dst[2] = *src++;
            dst += 4;
            count--;
        } while (count != -1);
    }
    cur = (u8 *)dst;
    if ((u32)src & 3) {
        src++;
    }

    block = src;
    point_count = 0;
    count = *src++;
    while (count != 0) {
        flags = *src++;
        if (flags & 8) {
            point_count += count * 6;
        } else {
            point_count += count * 3;
        }
        src += D_80094AEC[(flags >> 3) & 3] * count;
        count = *src++;
    }

    init.point_end = cur;
    cur = cur + obj_CalcOffset(init.count, point_count);
    cur = ((u32)cur & 3) ? cur + 2 : cur;

    src = block;
    packet = (s16 *)0x1F800000;
    init.groups = packet;
    for (n = *(s16 *)src++, count = n; n != 0; n = *(s16 *)src++, count = n) {
        flags = *src++;
        mode = ((s16)flags >> 3) & 3;
        if (((s16)flags >> 3) & 1) {
            while (--count != -1) {
                if (((s16)flags >> 3) & 2) {
                    value = ((u32)src[9] << 16) | src[8];
                } else {
                    value = ((u32)src[8] << 16) | src[7];
                }
                *packet++ = 4;
                *packet++ = 2;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                *packet++ = (u32)value >> 24;
                src += D_80094AEC[mode];
            }
        } else {
            while (--count != -1) {
                /* FAKE: identical arms (gouraud and flat triangle records keep
                 * the colour at the same offset); jump2 cross-jumping merges
                 * them, leaving loop.c's hoisted `& 2` test. */
                if (((s16)flags >> 3) & 2) {
                    value = ((u32)src[7] << 16) | src[6];
                } else {
                    value = ((u32)src[7] << 16) | src[6];
                }
                *packet++ = 3;
                *packet++ = 2;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                src += D_80094AEC[mode];
            }
        }
        *packet++ = 0;
    }

    cur = ((u32)cur & 3) ? cur + 2 : cur;
    func_80045230((s32)cur);
    if (*src != 0) {
        func_80052C10(D_80010D8C);
    }
    func_8003FE40((s16 *)init.points, init.count, cmds);

    init.matrix = obj + 0x18;
    init.flags = 0xE00;
    *(s16 *)rec = func_80017D84((u8 *)&init);
    *(u8 **)(obj + 0x60) = init.point_end;
    rec->inner.count = 0;
    rec->inner.objs[4] = 0;
    cur = ((u32)cur & 3) ? cur + 2 : cur;
    *(u16 *)((u8 *)rec + 4) = *(u16 *)rec;
    *(u8 **)((u8 *)rec + 8) = obj + 0x18;
    *(u8 **)((u8 *)rec + 0xC) = *(u8 **)(obj + 0x60);
    *(void **)((u8 *)rec + 0x10) = (u8 *)rec + 0x2C;
    *((u8 *)rec + 6) = 0;
    *((u8 *)rec + 7) = 0;
    return cur;
}
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    s32 i;
    i = 0;
    if (a1 > i) {
        s16 fill = -256;
        s16 *p = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)p + 6) = fill;
            p = (s16 *)((u8 *)p + 8);
        }
    }

    {
        s32 count;
        count = a2[0];
        a2++;
        if (count >= 0) {
            do {
                int val;
                val = a2[0];
                a2++;
                count--;
                while (count != -1) {
                    s32 addr;
                    i = a2[0];
                    a2++;
                    count--;
                    addr = (i << 3) + (s32)a0;
                    *(s16 *)(addr + 6) = val;
                }
                count = a2[0];
                a2++;
            } while (count >= 0);
        }
    }
    return (s16 *)a2;
}

void func_8003FECC(s32 *a0, s32 *a1, s16 *a2)
{
  s32 t2;
  u8 *new_var2;
  s16 v1;
  unsigned int t3;
  s32 *a3;
  s32 *t1;
  u16 t0;
  t2 = *((s32 *) (((u8 *) a1) + 0x1C));
  ;
  t0 = (u16) a2[0];
  a1 = (s32 *) (((u8 *) a1) + 0x1C);
  if (a2[0] == (-2))
  {
    goto end;
  }
  t3 = -2;
  a3 = (s32 *) ((t2 * 0x10) + (s32)(u8 *) a1);
  t1 = (s32 *) ((t2 * 4) + (s32)(u8 *) a1);
  loop:
  {
    a2++;
    {
      s32 v0;
      v1 = 1;
      v0 = (s32) (((u8 *) a0) + ((s32) ((((s16) t0) * 0x68) + 0x94)));
      t1[v1] = v0;
    }
    {
      u16 val = (u16) (*(a2++));
      t2++;
      *((u16 *) (((u8 *) a3) + 0x84)) = val;
    }
    *((u16 *) (((u8 *) a3) + 0x86)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x88)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x8C)) = (u16) (*(a2++));
    new_var2 = ((u8 *) a3) + 0x90;
    *((u16 *) (((u8 *) a3) + 0x8E)) = (u16) (*(a2++));
    *((u16 *) new_var2) = (u16) (*(a2++));
    {
      s32 val78 = (s32) (*(a2++));
      /* FAKE: split advance (+8 +8); the extra refs seat this pointer in $a3
         ahead of the halfword temp (combine re-merges to one addiu). */
      a3 = (s32 *) (((u8 *) a3) + 8);
      a3 = (s32 *) (((u8 *) a3) + 8);
      *((s32 *) (((u8 *) t1) + 0x78)) = val78;
    }
    t0 = (u16) (*a2);
    v1 = *a2;
    /* FAKE: split advance (+2 +2); the extra refs seat this pointer in $t1
       ahead of the entry counter (combine re-merges to one addiu). */
    t1 = (s32 *) (((u8 *) t1) + 2);
    t1 = (s32 *) (((u8 *) t1) + 2);
  }

  if (v1 != ((s16) t3))
  {
    goto loop;
  }
  end:
  a1[0] = t2;

  a1[5] = t2;
}
s32 math_AlignUp4(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}
void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        v1[3] = 1;
    }
}
void func_8003FFE0(void) {
    s32 *v0 = (s32 *)func_8004153C();
    if (v0) {
        s16 *v1 = (s16 *)v0[9];
        if (v1) {
            v1[1] = 1;
        }
    }
}
void func_8004001C(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 1;
        a0[0xE + i * 0xD0] = 1;
    }
}
void func_80040068(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 0;
        a0[0xE + i * 0xD0] = 0;
    }
}
void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;
        }
    }
}
/* The variable compare `s2[0] > s0` is load-bearing — target's 0x28 frame is the combine-leftover of the folded
 * guard (phantom slot sp+20); a literal `> 0` compare yields frame 0x20 + RA
 * swap. Every statement here is live. Do not respell. */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}

void func_8004016C(void) {
    void *v0 = func_8004153C();
    if (v0) {
        func_800400F8(v0);
    }
}

void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;
        *(s16 *)((s32)v1 + 6) = 1;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A3218 = 0;
s32 D_800A321C = 1;
RECT D_800A3220 = { 0x3F0, 0x1DC, 0x10, 0x24 };
s32 D_800A3228 = -1;
s32 D_800A322C = 0;
s32 D_800A3230 = 0;  /* reached gp-relative by func_8003EDC0: size from the blob label */
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 D_800A3818;
