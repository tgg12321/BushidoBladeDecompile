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
extern u8 D_800F33D8[];
extern u32 D_800A378C;
extern u32 D_80101E3C;
extern u32 D_80101E44;

/* Extern function declarations */
extern void func_80023CB4(s32, s32);
extern s32 func_80037110(s32);
extern void game_FrameInit(void);
extern void game_FrameLoop(void);
extern void func_800194F4(void);
extern void seq_Reset(void);
extern void VSync(s32);
extern void LoadImage(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern s32 D_800109BC;
extern void game_Cleanup(void);
extern s32 func_800371E8(s16);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern u8 *D_800A3894;
extern s16 D_800A38C4;
extern s16 D_80101F32;
extern void func_80035F30(s32, s32, s32, s32);
extern void obj_InitChars(void);
extern void obj_Reset(void);
extern void obj_InitTask(void);
extern void obj_InitPair(void);
extern void player_SetCharId(s32, s32);
extern void func_80021974(s32);
extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
extern s32 D_80101E70;

extern u8 D_80102781;


extern u8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern s32 func_80036D88(void);
extern void func_800174F4(void);
extern s32 D_800A384C;
extern s32 ratan2(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void special_camera_get_rot_dir(s32 *);
extern void StopPAD(void);
extern void StopCallback(void);
extern s32 D_800A3210;
extern void AddCOMB(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern u8 D_8008E6A4;
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A36C0;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern u8 D_800A377C;
extern u8 D_800A37A8;
extern s16 D_800A3904;
extern u8 D_800F65F8;
extern s32 g_anim_func_table;
extern s32 g_pad_data;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s16 D_80101ED6;
extern s32 g_file_disc_size;

/* GP-relative extern data (for decompiled functions) */
extern s32 D_800A3730;
extern s16 D_800A379E;
extern s16 D_800A3814;
extern s16 D_800A37C8;
extern s32 D_800A31F4;
extern u8 D_800A38CC;
extern u8 D_800A382C;
extern s32 D_800A3734;
extern s32 D_800A373C;
extern s32 D_800A3810;
extern s32 D_800A3738;
extern u8 D_800A320C;
extern s32 D_800A37DC;
extern s32 D_800A37F0;
extern s32 D_800A37FC;
extern s32 D_800A3800;
extern s32 D_800A3838;
extern s32 D_800A383C;
extern s32 D_800A3848;
extern s32 D_800A3850;
extern s32 D_800A31F8;
extern u8 D_800A36F8;
extern s32 D_800A36EC;
extern u8 D_800A3782;
extern u8 D_800A3203;
extern u8 D_800A31FC;
extern s32 D_80106A50;
extern s32 D_800A38C8;
extern u8 D_80102810;
extern s32 D_800A3870;
extern s32 D_800A3688;
extern s32 D_800A3698;
extern u8 D_800A37D0;
extern s32 D_800A3915_ext;
extern s32 D_800A36F4_ext;

/* Extern function declarations for decompiled functions */
extern s32 TestEvent(s32);
extern void CloseEvent(s32);
extern void EnableEvent(s32);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void read(s32, s32 *, s32);
extern void close(s32);
extern s32 firstfile(s32 *, s32 *);
extern s32 nextfile(s32 *);
extern void ResetRCnt(s32);
extern s32 GetRCnt(s32);
extern void StopCARD(void);
extern void DelCOMB(void);
extern void func_8006BEC4(s32, s32);
extern void func_8003E22C(void);
extern void game_SetPlayerCount(s32);
extern s32 disp_CalcFov(s32);
extern void SetGeomScreen(s32);
extern void func_8001B6F4(void);
extern void func_80022568(u8 *);
extern s32 g_str_memcard_fmt;
extern s32 D_80102810;
extern s32 D_800F34D8;
extern s32 D_800A31F0;
extern s32 D_800A3794;
extern s32 func_80037A20(s32, s32);
extern void func_80037F40(void *);
extern s32 func_80037AA4(void);
extern s32 func_80037B00(s32);
extern s32 func_80037B90(s32, s32, s32, void *, s32);
extern s32 func_80037C34(s32, s32, s32, void *, s32, s32, s32);

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

s32 func_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    sprintf(buf, &D_800109C8, a0, a1);
    return format(buf);
}

typedef struct { s32 w[4]; } Quad;
void func_80037F40(u8 *a0) {
    s32 checksum;
    u8 *p;
    s32 i;

    checksum = 0;
    p = (u8 *)&g_file_disc_size;
    i = 0;
    do {
        checksum += *p++;
        i++;
    } while ((u32)i < 0x24);

    i = 0;
    {
        Quad *end = (Quad *)&D_80106A70;
        u8 *base = a0;
        u8 *base2 = base;
        s32 offset = 0;
        do {
            Quad *dst = (Quad *)(offset + (s32)base);
            Quad *src = (Quad *)&g_file_disc_size;
            for (;;) {
                *dst = *src;
                src++;
                dst++;
                if (src != end) continue;
                *(s32 *)dst = *(s32 *)src;
                break;
            }
            *(s32 *)(base2 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            base2 += 4;
            i++;
            offset += 0x24;
        } while (i < 3);
        *(s32 *)(base + 0xFC) = 0;
    }
}
typedef struct { s32 w0, w1, w2, w3; } CopyBlock;

INCLUDE_ASM("asm/funcs", func_8003800C);
/* kengo:HIGH  |  is_damage_calc/damage_DebugDisp  |  79i */

void func_80038148(void) {
    u8 *p = D_800F33D8;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}
extern u8 D_8008F1C0[];
/* Rodata moved from asm/data/101C.rodata_pre_post.s (rodata-cleanup project,
 * docs/rodata-cleanup-project.md, 2026-06-09). func_80038170 (this file) is the
 * sole owner — uses these as &-addressed byte/word lookups. Declared as u32
 * arrays since the content is word-aligned. D_80010A2C ends with the literal
 * save-file id "BASLUS-00663BUSHIDO2" (per the asm/data block's content). */
const u32 D_800109EC[16] = {
    0x77DF7FFF, 0x635E6B9F, 0x56FD5B1E, 0x427C4ABD,
    0x2DFB323C, 0x199B21BB, 0x0D3A115A, 0x8000051A,
    0, 0, 0, 0, 0, 0, 0, 0,
};
const u32 D_80010A2C[38] = {
    0x00000000, 0x00055110, 0x00000000, 0x003EC5A3,
    0x38531000, 0x002958EA, 0x7DBA8400, 0x00038DDC,
    0xB7411000, 0x001CBBDD, 0xCDDB9400, 0x000111AD,
    0x668EE800, 0x0002008E, 0x6216A900, 0x0017008E,
    0x6AC63210, 0x001800AE, 0x449A8410, 0x002903DD,
    0x38500000, 0x006B08EB, 0x06D93000, 0x006B5DE7,
    0x006CE500, 0x006DCD91, 0x0009C300, 0x007EEA20,
    0x00001000, 0x008EB200, 0x00000000, 0x00020000,
    0x4C534142, 0x302D5355, 0x33363630, 0x48535542,
    0x324F4449, 0x00000000,
};
extern u8 D_800A3200;
extern u8 D_800A3201;
extern u8 *strcpy(u8 *, u8 *);

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    mask = D_80106A50;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = D_8008F204[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    strcpy(out + 4, D_8008F1C0);

    out[0x22] = D_8008F1A8[s1 * 2 + 0];
    out[0x23] = D_8008F1A8[s1 * 2 + 1];
    out[0x3C] = D_8008F1A8[s2 * 2 + 0];
    out[0x3D] = D_8008F1A8[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = D_8008F19C[s3 * 2 + 0];
        out[0x43] = D_8008F19C[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}

void func_800383A4(void) {
    s32 var_v1;
    s32 var_v0;
    s32 temp_s0;
    s32 var_s1;
    u16 temp_v0;

    var_v1 = D_800A31F4;
    if (var_v1 != 1) goto state_other;

    var_v1 = D_800A31F8;
    if (var_v1 == -1) goto sub_inc;

    if (var_v1 >= 0) goto positive_path;
    if (var_v1 == -2) goto neg2_handler;
    return;

positive_path:
    if (var_v1 >= 3) return;
    if (var_v1 <= 0) return;
    if (D_800A37C8 == 0) { D_800A31F4 = 5; return; }
    if (D_800A37C8 == 3) goto set_state_7;
    D_800A31F4 = 3;
    return;

neg2_handler:
    if (D_800A37C8 != 3) goto neg2_else;
set_state_7:
    D_800A31F4 = 7;
    return;
neg2_else:
    D_800A31F4 = 0;
    D_800A379E = 0xA;
    return;

sub_inc:
    temp_v0 = (u16)D_800A3814 + 1;
    D_800A3814 = temp_v0;
    if ((s16)temp_v0 < 4) return;
    var_v0 = 8;
    goto finish;

state_other:
    var_v0 = 5;
    if (var_v1 == var_v0) goto state_5;
    if (var_v1 < 6) {
        var_v0 = 3;
        if (var_v1 == var_v0) goto state_3;
        return;
    }
    var_v0 = 7;
    if (var_v1 == var_v0) goto state_7;
    return;

state_3:
    D_800A379E = 1;
    func_80037A20(0, 0);
    temp_s0 = func_80037AA4();
    if (func_80037B00(D_800A31F0) != 0) {
        var_s1 = 0;
        if (D_800A37C8 == 1) {
            var_v0 = 0xD;
            goto finish;
        }
        goto setup_load;
    }
    var_s1 = 1;
    if (temp_s0 == 0) {
        var_v0 = 7;
        goto finish;
    }
setup_load:
    D_800A38CC = 1;
    func_80038148();
    func_80038170(D_800F33D8);
    func_80037F40(D_800F33D8 + 0x100);
    if (func_80037C34(0, 0, D_800A31F0, D_800F33D8, 1, 0x200, var_s1) != 0) {
        close(D_800A3794);
        var_v0 = 3;
        goto finish;
    }
    D_800A31F4 = 4;
    return;

state_5:
    func_80037A20(0, 0);
    func_80037AA4();
    if (func_80037B00(D_800A31F0) == 0) {
        var_v0 = 0xE;
        goto finish;
    }
    D_800A379E = 4;
    func_80038148();
    if (func_80037B90(0, 0, D_800A31F0, D_800F33D8, 0x200) != 0) {
        close(D_800A3794);
        var_v0 = 6;
        goto finish;
    }
    D_800A31F4 = 6;
    return;

state_7:
    var_v0 = func_80037F08(0, 0);
    if (var_v0 != 0) {
        var_v0 = 0xB;
        goto finish;
    }
    var_v0 = 0xC;
finish:
    D_800A379E = var_v0;
    D_800A31F4 = 0;
}

/* kengo:HIGH  |  is_pad/pad_FuncAnalog  |  173i */
extern s32 func_8003800C(s32 *);
/* func_80038658 — CD-load/save state-machine completion handler: dispatches
 * on D_800A31F4 (state 4 = post-read, state 6 = post-write), reaps
 * func_800378A8()'s status, closes the file handle, and posts a result code
 * to D_800A379E. The ret==0 ("still pending") paths route through the shared
 * fail_store end label (the shared-end-label recipe,
 * .claude/rules/shared-end-label.md) so GCC cannot constant-fold the
 * per-state fail codes. */
void func_80038658(void) {
    s32 ret;
    s32 fail;

    switch (D_800A31F4) {
    case 4:
        ret = func_800378A8();
        if (ret == 0) {
            fail = 1;
            goto fail_store;
        }
        close(D_800A3794);
        if (ret == 1) {
            D_800A379E = 2;
        } else {
            D_800A379E = 3;
        }
        D_800A31F4 = 0;
        return;
    case 6:
        ret = func_800378A8();
        if (ret == 0) {
            fail = 4;
            goto fail_store;
        }
        close(D_800A3794);
        if (ret == 1) {
            D_800A379E = 5;
            if (func_8003800C(&D_800F34D8) == 0) {
                D_800A379E = 0xF;
            }
        } else {
            D_800A379E = 6;
        }
        D_800A31F4 = 0;
        return;
    }
    return;

fail_store:
    D_800A379E = fail;
}
s32 func_80038734(void) {
    if ((u32)D_800A31F4 < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    func_800383A4();
    func_80038658();
    return D_800A379E;
}
void func_8003877C(void) {
    D_800A379E = 4;
    D_800A3814 = 0;
    D_800A37C8 = 0;
    D_800A31F4 = 1;
}
void func_8003879C(void) {
    D_800A379E = 1;
    D_800A37C8 = 1;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387C0(void) {
    D_800A379E = 1;
    D_800A37C8 = 2;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387E8(void) {
    D_800A379E = 9;
    D_800A37C8 = 3;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
extern u8 D_800A3203;
extern u8 D_800A31FC;
extern void func_8003877C(void);
extern s32 func_80038734(void);
extern void func_8006BEC4(s32, s32);

s32 func_8003880C(void) {
    s32 s0;
    s32 v0;

    s0 = 0;
    if (D_800A3203) {
        D_800A3203 = 0;
        D_800A31FC = 1;
        func_8003877C();
    }
    v0 = func_80038734();
    switch (v0 - 4) {
    case 0:
        break;
    case 1:
        s0 = 1;
        break;
    case 2:
        s0 = -1;
        break;
    case 3:
        s0 = -1;
        break;
    case 4:
        s0 = -1;
        break;
    case 5:
        s0 = -1;
        break;
    case 6:
        s0 = -1;
        break;
    case 7:
        s0 = -1;
        break;
    case 8:
        s0 = -1;
        break;
    case 9:
        s0 = -1;
        break;
    case 10:
        s0 = -1;
        break;
    case 11:
        s0 = -1;
        break;
    default:
        s0 = -1;
        break;
    }
    func_8006BEC4(0, -1);
    if (s0) {
        D_800A3203 = 1;
        D_800A31FC = 0;
    }
    return s0;
}
s32 func_800388A8(void) {
    extern u8 D_800A3204;
    extern u8 D_800A3318;
    s32 result = 0;
    u32 buttons;
    if (D_800A3204 != 0) {
        D_800A3204 = 0;
        D_800A3318 = 0;
    }
    buttons = D_80102794;
    if (buttons & 0x400040) {
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A3318 == 0) {
            result = 1;
        } else {
            result = -1;
        }
    } else if (buttons & (u32)0x80008000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3318 = 0;
    } else if (buttons & 0x20002000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3318 = 1;
    }
    func_8006BEC4(0x13, D_800A3318);
    if (result != 0) {
        D_800A3204 = 1;
    }
    return result;
}
s32 func_80038988(void) {
    extern u8 D_800A3205;
    extern u8 D_800A331C;
    extern u8 D_800A3320;
    extern u8 D_800A3324;
    extern u8 D_800A3328;
    extern u8 D_800A332C;
    extern u8 D_800A3330;
    extern u8 D_800A3334;
    extern u8 D_800A3338;
    extern u8 D_800A333C;
    s32 result = 0;
    s32 v0;
    s32 sel;

    if (D_800A3205) {
        D_800A331C = 0;
        D_800A3320 = 0;
        D_800A3324 = 0;
        D_800A3328 = 0;
        D_800A332C = 0;
        D_800A31FC = 1;
        func_8003877C();
        D_800A3205 = 0;
        D_800A3330 = 0x5A;
        D_800A3334 = 0;
        D_800A3338 = 0;
        D_800A333C = 0;
    }

    v0 = func_80038734();

    if (D_800A333C != 0) {
        func_8006BEC4(0xA, -1);
        goto timer;
    }

    sel = 0;
    if (D_800A31FC == 0) {
        if (D_800A3338 == 1) {
            switch (v0 - 4) {
            case 0: sel = 2; break;
            case 1: sel = 3; break;
            case 2:
            case 6:
            case 10:
            case 11: sel = 4; break;
            case 4: sel = -1; break;
            default: sel = 0; break;
            }
        } else {
            switch (v0 - 4) {
            case 0: sel = 2; break;
            case 1: sel = 3; break;
            case 2: sel = 4; break;
            case 10: sel = 1; break;
            case 11: sel = 0xB; break;
            case 4: sel = -1; break;
            case 6: sel = 0x14; break;
            default: sel = 0; break;
            }
        }
    }

    if (sel >= 0) {
        func_8006BEC4(sel, -1);
    }

    if (D_800A31FC != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            D_800A331C++;
            if ((u8)D_800A331C >= 5) {
                D_800A333C = 1;
                D_800A3330 = 0x5A;
                break;
            }
            func_8003877C();
            break;
        case 11:
            D_800A332C++;
            if ((u8)D_800A332C >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        case 6:
            D_800A3328++;
            if ((u8)D_800A3328 >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        case 10:
            D_800A3324++;
            if ((u8)D_800A3324 >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        case 2:
            D_800A31FC = 0;
            break;
        case 1:
            D_800A3320++;
            if ((u8)D_800A3320 >= 5) {
                D_800A31FC = 0;
                D_800A3338 = 1;
            }
            func_8003877C();
            break;
        default:
            break;
        }
    } else {
        switch (v0 - 5) {
        case 0:
        case 1:
        case 5:
        case 9:
        case 10:
            goto timer;
        case 3:
            D_800A333C = 1;
            D_800A3330 = 0x5A;
            break;
        default:
            break;
        }
    }

    goto end;

timer:
    D_800A3330--;
    if ((u8)D_800A3330 == 0 || (D_80102794 & 0x100010)) {
        func_8005C650(2, 0x7F, 0x7F);
        result = 1;
    }

end:
    if (result != 0) {
        D_800A3205 = 1;
    }
    return result;
}


s32 func_80038C70(void) {
    extern u8 D_800A3207;
    extern u8 D_800A334C;
    extern u8 D_800A3350;
    extern u8 D_800A3354;
    extern u8 D_800A3206;
    extern u8 D_800A3340;
    extern u8 D_800A3344;
    extern u8 D_800A3348;
    extern void func_8006BEC4(s32, s32);
    extern void func_8005C650(s32, s32, s32);
    extern void func_8003877C(void);
    extern void func_8003879C(void);
    extern void func_800387C0(void);
    extern void func_800387E8(void);
    s32 result = 0;
    s32 sel2 = -1;
    s32 v0;
    s32 sel;

    if (!D_800A3207) {
        D_800A3207 = 1;
        D_800A334C = 0x5A;
        D_800A3350 = 0;
        D_800A3354 = 0;
        D_800A31FC = 0;
    }

    v0 = func_80038734();

    if (D_800A3354 != 0) {
        func_8006BEC4(0xA, -1);
        D_800A334C--;
        if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
            func_8005C650(2, 0x7F, 0x7F);
            D_800A3207 = 1;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            D_800A3354 = 0;
            D_800A31FC = 0;
        }
        goto end;
    }

    if (D_800A3207 == 1) {
        v0 = 0;
    } else if (D_800A3207 == 2) {
    } else if (D_800A3207 == 3) {
    } else if (D_800A3207 == 4) {
        v0 = 0x11;
    }

    sel = 0;
    if (D_800A31FC != 0) {
        goto sel_dispatch;
    }

    if (D_800A3207 == 3) {
        if (v0 == 8) {
            goto case8_sel;
        }
        if (v0 == 10) {
            goto case12_sel;
        }
        goto case9_11_sel;
    }

    switch (v0) {
    case 0:
        sel = 0x11;
        goto load_sel2;
    case 13:
    case 17:
        sel = 6;
    load_sel2:
        sel2 = D_800A3350;
        goto sel_dispatch;
    case 1:
        sel = (-(D_800A38CC != 0)) & 7;
        goto sel_dispatch;
    case 2:
        sel = 8;
        goto sel_dispatch;
    case 3:
        sel = 9;
        goto sel_dispatch;
    case 8:
    case8_sel:
        sel = -1;
        goto sel_dispatch;
    case 7:
        sel = 5;
        goto sel_dispatch;
    case 10:
        if (D_800A3206 == 0) {
            D_800A3350 = 1;
        }
        if (D_80102794 & 0x400040) {
            D_800A3206 = 0;
            func_8005C650(1, 0x7F, 0x7F);
            sel = 0xD;
            if (D_800A3350 == 0) {
                func_800387E8();
                goto sel_dispatch;
            }
            v0 = 0;
            D_800A3207 = 5;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            sel = -1;
            goto sel_dispatch;
        }
        sel = 0xC;
        if ((D_80102794 & 0xA000A000U) != 0) {
            D_800A3206 = 1;
        }
        goto load_sel2;
    case 9:
    case 11:
    case9_11_sel:
        sel = 0xD;
        goto sel_dispatch;
    case 12:
    case12_sel:
        sel = 0xF;
        goto sel_dispatch;
    default:
        sel = 0;
        goto sel_dispatch;
    }

sel_dispatch:
    if (sel >= 0) {
        func_8006BEC4(sel, sel2);
    }

    if (D_800A31FC != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            D_800A3348++;
            if (((u8)D_800A3348) >= 5) {
                D_800A3354 = 1;
                D_800A334C = 0x5A;
                break;
            }
            func_8003877C();
            break;
        case 1: case 2: case 3:
        case 9: case 10: case 11:
            D_800A3340++;
            if (((u8)D_800A3340) >= 5) {
                D_800A31FC = 0;
                func_8003879C();
                break;
            }
            func_8003877C();
            break;
        case 6:
            D_800A3344++;
            if (((u8)D_800A3344) >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        default:
            break;
        }
    } else if (D_800A3207 == 3) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            result = 1;
            break;
        case 1: case 2: case 3:
        case 5: case 7: case 8: case 9: case 10: case 11:
        default:
            func_8003879C();
            D_800A3207 = 2;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 6:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
                result = 1;
                break;
            }
            break;
        }
    } else {
        switch (v0) {
        case 8:
            D_800A3354 = 1;
            D_800A334C = 0x5A;
            break;
        case 2: case 3: case 7: case 12:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
                if (v0 != 7) {
                    result = 1;
                    break;
                }
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                D_800A3354 = 0;
                D_800A31FC = 0;
            }
            break;
        case 11:
            D_800A31FC = 1;
            func_8003877C();
            D_800A3207 = 3;
            D_800A334C = 0x5A;
            break;
        case 13:
            D_800A3207 = 4;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 0:
            if (D_800A3207 == 5) {
                D_800A3207 = 1;
                break;
            }
            goto sw4_L2;
        case 17:
        sw4_L2:
            if (D_80102794 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                if (v0 == 0) {
                    if (D_800A3350 != 0) {
                        result = 1;
                        break;
                    }
                    D_800A3348 = 0;
                    D_800A3340 = 0;
                    D_800A3344 = 0;
                    D_800A31FC = 1;
                    func_8003877C();
                } else {
                    if (D_800A3350 != 0) goto area_c_long;
                    func_800387C0();
                }
                D_800A3207 = 2;
                break;
            area_c_long:
                v0 = 0;
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                break;
            }
            goto sw4_buttons;
        case 10:
        sw4_buttons:
            if (D_80102794 & 0x80008000U) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3350 = 0;
                break;
            }
            if (D_80102794 & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3350 = 1;
            }
            break;
        default:
            break;
        }
    }

    if (v0 < 11) {
        if (v0 >= 9) {
            goto d_check;
        }
        if (v0 == 1) {
            goto d_check;
        }
        goto end;
    }
    if (v0 != 17) {
        goto end;
    }
d_check:
    if (D_800A31F8 == -1) {
        D_800A3354 = 1;
        D_800A334C = 0x5A;
    }

end:
    if (result != 0) {
        D_800A3207 = 0;
        D_800A31FC = 0;
    }
    return result;
}
/* kengo:MED  |  is_motion/motion_SetMotion  |  425i  |  -23 5.4% */
s32 *func_800392B8(void) {
    return (s32 *)D_800F33D8;
}
void func_800392C8(void) {
    u8 val;
    int new_var;
    s32 i;
    s32 j;

    val = 0xFF;
    i = 0x1F0;
    D_800A36EC = (u8 *)D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
loop1:
    *(&D_80101BF0 + i) = val;
    i -= 0x10;
    if (i >= 0) goto loop1;

    new_var = -1;
    j = 0xB30;
loop2:
    *((s16 *)((u8 *)D_800F68E0 + j)) = new_var;
    j -= 0x10;
    if (j >= 0) goto loop2;
}
void func_80039320(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    u8 *p;
    s16 *q;
    s16 val;
    s16 newval;

    i = 0;
    newval = 0xFF;
    p = &D_80101BF0;

    do {
        if (*p == D_800A36F8) {
            *p = newval;
        }
        i++;
        p += 0x10;
    } while (i < 0x20);

    q = D_800F68E0;
    i = 0;
    do {
        val = *q;
        if (val != -1) {
            newval = val + 1;
            *q = newval;
            if ((s16)newval - *(u8 *)((u8 *)q + 2) >= 0x101) {
                *q = -1;
            }
        }
        i++;
        q = (s16 *)((u8 *)q + 0x10);
    } while (i < 0xB4);

    D_800A379C = 0;
    D_800A3714 = 0;
}
INCLUDE_ASM("asm/funcs", func_800393C8);
/* kengo:MED  |  sa_se/saSeInit_2  |  123i  |  x2 size collision */
void func_800395B4(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    u8 *slot;
    u8 idx;
    u8 sentinel;

    if (D_800A3208 == 0) {
        idx = D_800A379C;
        slot = &D_80101BF0 + (u32)(idx & 0xFF) * 0x10;
        if ((u32)(idx & 0xFF) < 0x20U) {
            sentinel = 0xFF;
loop:
            if (*slot != sentinel) {
                D_800A379C = idx + 1;
                idx = idx + 1;
                slot += 0x10;
                if ((u32)(idx & 0xFF) < 0x20U) {
                    goto loop;
                }
            }
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            slot[1] = arg0;
            slot[2] = arg1;
            slot[0] = tmp;
            *(s16 *)&slot[4] = (s16)arg2[0];
            *(s16 *)&slot[6] = (s16)arg2[1];
            *(s16 *)&slot[8] = (s16)arg2[2];
            if (arg3 != NULL) {
                *(u16 *)&slot[0xA] = arg3[0];
                *(u16 *)&slot[0xC] = arg3[1];
                *(u16 *)&slot[0xE] = arg3[2];
            }
        }
    }
}
void func_80039680(u8 *a0) {
    s16 idx;
    u8 *base;
    u8 *dest;

    idx = *(s16 *)(a0 + 4);
    base = (u8 *)(D_800A36EC + D_800A36F8 * 56);
    dest = base + idx * 28;

    *(s16 *)(dest + 4) = *(s32 *)(a0 + 0xF4);
    *(s16 *)(dest + 8) = *(s32 *)(a0 + 0xFC);
    *(s16 *)(dest + 6) = *(s32 *)(a0 + 0xF8);

    {
        u16 v = *(u16 *)(a0 + 0x1CA);
        u8 b = *(u8 *)(a0 + 0xB3);
        *(s16 *)(dest + 0xA) = (v & 0xFFF) | (b << 12);
    }

    *(s16 *)(dest + 0xC) = *(s32 *)(a0 + 0x148);
    *(u8 *)(dest + 0x14) = *(u16 *)(a0 + 0x1E6) >> 2;
    *(u8 *)(dest + 0x15) = *(u16 *)(a0 + 0x1E8) >> 2;
    *(u8 *)(dest + 0x16) = *(u16 *)(a0 + 0x1EA) >> 2;
    *(s32 *)(dest + 0) = *(s32 *)(a0 + 0x50);
    *(u8 *)(dest + 0x17) = 0;

    if (*(u8 *)(a0 + 0x60) != 0) {
        *(u8 *)(dest + 0x17) = 1;
    }
    if (*(u8 *)(a0 + 0x61) != 0) {
        *(u8 *)(dest + 0x17) |= 2;
    }

    *(s16 *)(dest + 0xE) = *(u16 *)(a0 + 0x64);
    *(s16 *)(dest + 0x10) = *(u16 *)(a0 + 0x66);
    *(s16 *)(dest + 0x12) = *(u16 *)(a0 + 0x68);
    *(u8 *)(dest + 0x18) = *(u8 *)(a0 + 0x62);
    *(u8 *)(dest + 0x19) = *(u16 *)(a0 + 0x40);
}
void func_800397A0(void) {
    u8 val = D_800A36F8;
    if (val == 0x77) {
        D_800A36F8 = 0;
        D_800A3782 = 1;
    } else {
        D_800A36F8 = val + 1;
    }
}
void func_800397D4(void) {
    gpu_EnableDisplay();
    func_8003E22C();
    game_SetPlayerCount(0);
    SetGeomScreen(disp_CalcFov(0x2D));
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_8001B6F4();
    game_Cleanup();
    D_800A37D0 = 0;
    D_800A3834 = 5;
}
extern s32 func_80053584(s32 *, s32 *, s32 *, s32 *);
extern s32 func_80054434(void);
void func_8003984C(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 sp10[3];
    s32 sp20[3];
    s32 sp30[4];
    s32 sp40[2];
    s32 mid_x, mid_y, mid_z;
    s32 result;

    mid_x = (s32)(arg0[0x198 / 4] + arg0[0x1A4 / 4]) / 2;
    sp10[0] = mid_x;
    mid_y = (s32)(arg0[0x19C / 4] + arg0[0x1A8 / 4]) / 2;
    sp10[1] = mid_y - 0x190;
    mid_z = (s32)(arg0[0x1A0 / 4] + arg0[0x1AC / 4]) / 2;
    sp20[0] = mid_x;
    sp20[1] = mid_y + 0x190;
    sp10[2] = mid_z;
    sp20[2] = mid_z;
    if (func_80053584(sp10, sp20, sp30, sp40) != 0) {
        result = func_80054434();
        *arg1 = result;
        if (result == 7) {
            goto neg;
        }
        if (result == 0) {
            goto neg;
        }
        *arg2 = ((0x2A >> result) ^ 1) & 1;
    } else {
        *arg1 = -1;
neg:
        *arg2 = -1;
    }
}
INCLUDE_ASM("asm/funcs", func_8003993C);
void func_8003A174(void) {
    s32 neg1;
    EnterCriticalSection();
    neg1 = -1;
    do {
        D_800A3738 = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    } while (D_800A3738 == neg1);
    neg1 = -1;
    do {
        D_800A3810 = OpenEvent(0xF000000B, 0x8000, 0x2000, 0);
    } while (D_800A3810 == neg1);
    ExitCriticalSection();
    neg1 = -1;
    VSync(2);
    AddCOMB();
    do {
        D_800A373C = open(&D_800A3210, 2);
    } while (D_800A373C == neg1);
    neg1 = -1;
    do {
        D_800A3734 = open(&D_800A3210, 0x8001);
    } while (D_800A3734 == neg1);
    _comb_control(2, 0, 0);
    _comb_control(1, 3, 0xE100);
    _comb_control(1, 4, 1);
}
void func_8003A264(void) {
    close(D_800A3734);
    close(D_800A373C);
    EnterCriticalSection();
    CloseEvent(D_800A3738);
    CloseEvent(D_800A3810);
    ExitCriticalSection();
    VSync(2);
    DelCOMB();
    _comb_control(1, 1, 0);
}
s32 func_8003A2DC(void) {
    return (_comb_control(0, 0, 0) & 0x180) == 0;
}
void func_8003A308(void) {
    if (_comb_control(3, 1, 0) != 0) {
        D_800A38A0 = 1;
    } else {
        D_800A38A0 = 0;
    }
    _comb_control(3, 0, 1);
}
void func_8003A360(void) {
    EnableEvent(D_800A3810);
    EnableEvent(D_800A3738);
    D_800A320C = 1;
    D_800A3730 = 0;
}
void func_8003A39C(void) {
    D_800A320C = 0;
    D_800A3730 = 0;
    _comb_control(2, 0, 0);
    _comb_control(1, 1, 0);
    func_8003A264();
    D_800A3834 = 8;
}
void func_8003A3F0(void) {
    func_8003A39C();
    D_800A3928 = 1;
}
void func_8003A41C(void) {
    D_800A3730 = 1;
}
s32 func_8003A42C(s32 a0, u32 a1) {
    if (a1 > 0x10000) {
        D_800A382C = 0;
        return 0;
    }
    return 1;
}
s32 func_8003A450(void) {
    s32 s1;
    s32 s0;

    s1 = GetRCnt(0xF2000001);
    if (s1 >= 0x401) {
        ResetRCnt(0xF2000001);
        s1 = 0;
    }

    while (1) {
        while (1) {
            if (_comb_control(3, 1, 0) != 0) {
                break;
            }
            if (GetRCnt(0xF2000001) - s1 >= 0x7801) {
                return 0;
            }
        }

        s0 = 0;
        do {
            if (_comb_control(3, 1, 0) == 0) {
                break;
            }
            s0++;
        } while (s0 < 1000);

        if (s0 >= 1000) {
            break;
        }
    }

    _comb_control(1, 1, 1);
    D_800A382C = 1;
    _comb_control(4, 0, (s32)&func_8003A42C);
    write(D_800A373C, &D_800A3698, 8);
    _comb_control(4, 0, 0);
    _comb_control(1, 1, 0);
    return D_800A382C;
}
void func_8003A574(void) {
    read(D_800A3734, &D_800A3688, 8);
}
extern s32 D_800A38D0;
s32 func_8003A5A0(void) {
    s32 s0;
    s32 s1;
    s32 a1;
    s32 a0;
    s32 v0;

    s1 = 0;
    s0 = GetRCnt(0xF2000001);
    if (s0 >= 0x401) {
        goto overflow;
    }
    goto loop_check;
overflow:
    ResetRCnt(0xF2000001);
    s0 = 0;
loop_check:
    if (TestEvent(D_800A3738) != 0) {
        goto success;
    }
    if (TestEvent(D_800A3810) == 0) {
        goto poll;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    _comb_control(2, 0, 0);
    s0 = 0;
    func_8003A574();
    ResetRCnt(0xF2000001);
poll:
    if (((_comb_control(0, 0, 0) >> 7) & 3) == 1) {
        goto loop_check;
    }
    if (GetRCnt(0xF2000001) - s0 < 0x3C01) {
        goto loop_check;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    goto overflow;
success:
    a1 = D_800A3688;
    a0 = D_800A368C;
    v0 = a1 ^ (a1 >> 16);
    v0 = v0 ^ (a0 >> 16);
    v0 = v0 & 0xFFFF;
    if ((a0 & 0xFFFF) == v0) {
        goto match;
    }
    D_800A38D0 += 1;
    return 0;
ret0_tramp:
    return 0;
match:
    D_800A36C0 = a1;
    D_800A36C4 = a0;
    return 1;
}
/* kengo:HIGH  |  is_pad/pad_ClearAppliBuffer  |  87i */
s32 func_8003A6FC(u32 arg0) {
    s32 count = 0;
    s32 i;
    for (i = 0; i < 32; i++) {
        count += (arg0 >> i) & 1;
    }
    return count;
}
extern s32 D_800A38A0;
extern s16 D_800A36C2;
extern s32 D_800A36D0;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern s32 D_800A369C;
extern u16 D_800A37C4;
extern u8 D_800A3916;
extern s32 D_800A3908;
extern s32 D_800A38FC;

typedef s32 (*FuncBufType)(void *);

INCLUDE_ASM("asm/funcs", func_8003A728);

void func_8003AA48(void) {
    s16 buf[12];
    *(s32 *)&buf[4] = 0;
    buf[1] = 4;
    buf[0] = 4;
    func_8003A728((s32)buf);
}
void func_8003AA78(void) {
    D_800A3870 = 1;
    VSync(2);
    func_8003AA48();
    VSync(2);
}
void func_8003AAB0(void) {
    s32 val;
    D_800A3870 = 2;
    VSync(2);
    val = 2;
    do {
        func_8003AA48();
        if (D_800A320C == 0) {
            goto end;
        }
        ResetRCnt(0xF2000001);
        do {
        } while (GetRCnt(0xF2000001) < 0x100);
    } while (D_800A3870 == val);
end:
    VSync(2);
    func_8003AA48();
    VSync(2);
}
