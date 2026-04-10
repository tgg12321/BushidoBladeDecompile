#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern s32 D_800F33D8;
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
extern void func_8003A39C(void);
extern void sys_VSync(s32);
extern void gpu_LoadImage(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern void game_Cleanup(void);
extern s32 func_800371E8(s16);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern s32 D_800A3894;
extern u32 D_800A3D30;
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
extern s32 D_800A3894;
extern u8 D_80102781;
extern u8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 *func_8004153C(s32);
extern void func_800432A0(s32, s32, s32, s32, s32);
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void initLoadImage(u32 *, s16 *, s32, s32);
extern s32 func_8003F268(void);
extern s32 func_80052C28(s32, s32);
extern s32 func_800788B0(void);
extern void func_800372C0(void);
extern void func_800548DC(void);
extern s32 func_8005FC9C(s32, s32);
extern s32 func_80054F68(void);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void func_8003B5A4(void);
extern s32 func_8005E54C(s32, s32, s32);
extern void func_8005C650(s32, s32, s32);
extern s32 *func_80077D00(void);
extern void func_80060758(void);
extern void func_8001CD68(u8 *);
extern void func_80046BF4(s16 *, s32, s32);
extern void game_StageInit(s32);
extern s32 func_800600C8(s32, s32, s32);
extern void func_8001DA2C(void);
extern void func_8003B10C(s32);
extern void func_8005FBC8(s32, s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_8001DBE4(void);
extern void func_80041BF4(s32, s32, s32);
extern void func_8004659C(s32);
extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern void func_800342A0(void);
extern s32 func_80022408(s32 *);
extern void gpu_StoreImage(s32 *, u16 *);
extern void func_80052BE4(u8 *);
extern void func_8003F388(s16 *);
extern void func_80037774(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
extern s8 D_8008EA70;
extern s32 D_8009060C;
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern u8 D_800A377C;
extern u8 D_800A37A8;
extern u16 D_800A3904;
extern u8 D_800F65F8;
extern s32 g_anim_func_table;
extern s32 g_pad_data;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s16 D_80101ED6;
extern s32 g_file_disc_size;
extern s32 D_80106A58;
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_ASM("asm/funcs", func_8003B9D0);
INCLUDE_ASM("asm/funcs", md_game_check_change_sub_mode);
/* kengo:HIGH  |  md_game/md_game_check_change_sub_mode  |  87i */

extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80078824(s32);
extern void func_80035FA8(void);
extern void func_80036FD4(s32, s32);
extern void func_80037260(void);
void func_8003BE10(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    obj_InitAll();
    func_80078824((s32)0x80118800);
    func_80035FA8();
    {
        s32 v0 = func_80036EA8(5, 0x20);
        func_80036FD4(v0, 4);
    }
    func_80037260();
    D_800A3834 = 0xB;
    gpu_DisableDisplay();
}
void func_8003BEA8(void) {
    s32 s0 = 0;
    s32 v0;
    s32 v1;

    v0 = func_800788B0();
    v1 = D_80102794 & 0x40;
    v0 = (u32)v0 > 0u;
    if (v1 != 0) {
        v0 = 1;
    }
    if (v0 != 0) {
        func_800372C0();
        {
            s32 *ptr = &g_file_disc_size;
            s32 old_val = *ptr;
            s32 new_val = old_val | D_800A37A4;
            if (new_val != old_val) {
                *ptr = new_val;
                s0 = 1;
            }
        }
        {
            s32 stage_u = (s32)(u16)D_80101ED2;
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
            s32 bit = (s16)D_80101ED2;
            u8 val;
            if (bit >= 0xC) {
                bit = bit - 9;
            }
            {
                u8 *vptr = &g_file_disc_type;
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
    gpu_EnableDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    v = func_80045814();
    func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    game_Init();
    D_800A3834 = 8;
}
INCLUDE_ASM("asm/funcs", cpu_side_move_dir_2);
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_2  |  160i  |  x4 size collision */
void func_8003C2C0(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_8005FC9C(D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    if (func_80054F68() == 0 || (D_80102794 & 0x400040) != 0) {
        if ((u32)(D_800A38A4 - 6) < 2u && D_800A3781 != 0) {
            s32 stage = (s16)D_80101ED2;
            s32 newval = 8;
            if (*((u8 *)&D_8008D9EC + stage) != 0) {
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
extern u8 D_800A377C[];
extern u8 D_800F65F8[];
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
void func_8003C42C(void) {
    s32 counts[8];
    s32 i;
    s32 v0;
    v0 = D_800A389B;
    i = 0;
    counts[2] = 0;
    counts[1] = 0;
    counts[0] = 0;
    if (v0 > 0) {
        s32 *base = counts;
        s32 n = v0;
        do {
            s32 idx = D_800A377C[i];
            base[idx]++;
            i++;
        } while (i < n);
    }
    v0 = counts[0];
    if (v0 != counts[1]) {
        v0 = counts[0] < counts[1];
    } else {
        v0 = D_800A389B;
        i = 0;
        counts[1] = 0;
        counts[0] = 0;
        if (v0 > 0) {
            s32 *base2 = counts;
            s32 n2 = v0;
            u8 *tbl = D_800F65F8;
            do {
                s32 j = 0;
                s32 *p = base2;
                u8 *q = tbl;
                do {
                    *p += *q++;
                    j++;
                    p++;
                } while (j < 2);
                i++;
                tbl += 2;
            } while (i < n2);
        }
        v0 = counts[0];
        if (v0 != counts[1]) {
            v0 = v0 < counts[1];
        } else {
            v0 = 2;
        }
    }
    D_800A382D = v0;
    do { } while (0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x15;
}
INCLUDE_ASM("asm/funcs", func_8003C560);
INCLUDE_ASM("asm/funcs", SetCurrentCursor);
/* kengo:LOW  |  su_menu_edit/_SetCurrentCursor  |  104i  |  PS2 UI — reverted */
void func_8003C8B4(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_80060768(D_800A38B4, 1, D_800A38E9);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((D_80102794 & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        func_80033FE4();
    }
}
void func_8003C958(void) {
    gpu_InitDisplay();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", func_8003C9A4);
void func_8003CCCC(void) {
    gpu_InitDisplay();
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    gpu_DisableDisplay();
}
void func_8003CD10(void) {
    s32 *a0 = (s32 *)&D_800F6608;
    s16 *a1 = (s16 *)((u8 *)a0 + 0x10);
    s32 ret;

    func_8003F1E4(0);
    a0[0] = 0;
    D_800F660C = -0xBB8;
    D_800F6610 = 0;
    *a1 = 0x20;
    D_800F661C = 0;
    D_800F6620 = 0x2710;
    D_800F661A = (s16)(D_800A36AC << 2);
    func_80046BF4((s16 *)a0, a1, 0x2710);
    game_StageInit(1);

    ret = func_800600C8(D_800A391F, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 >= 0x97 || (D_80102794 & 0x400040) != 0) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
extern u16 D_80101ED6;
extern s32 D_800A3818;
extern void func_8001DA2C(void);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8003E22C(void);
extern void func_8003F218(s32);
extern s32 func_80022408(s32 *);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
void func_8003CE18(void) {
    s32 s0;
    s32 v0;
    s8 player;

    func_8001DA2C();
    func_800372C0();
    gpu_InitDisplay();
    gpu_EnableDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8003E22C();
    func_8003F218(0);
    v0 = disp_CalcFov(0x2D);
    func_8007EFFC(v0);
    player = D_800A3748;
    {
        u16 val = *((u16 *)((u8 *)&D_80101ED6 + player * 1100));
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
        s32 *addr = (s32 *)&D_80101FBC;
        s32 result;
        if (D_800A3748 == 0) {
            addr = (s32 *)((u8 *)addr + 0x44C);
        }
        result = func_80022408(addr);
        D_800A3818 = result;
        func_80054884(0x16, s0, result, (s32)D_80101EDA, (s32)D_80102326, -1, -1, 0);
    }
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x1D;
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", func_8003CF84);
void func_8003D2C4(void) {
    gpu_LoadImage((s32)&D_800A3220, (s32)&D_80090178);
}
extern s32 D_800A3364;
extern s32 D_800A3218;
extern s32 D_800A321C;
extern s32 D_800A3358;
extern s32 D_800A335C;
extern s32 D_800A3360;
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
    register u32 code asm("a1") = 0xE100001F;
    register u32 mask_lo asm("a2") = 0x00FF0000;
    u8 *base;
    register u8 *p asm("v1");
    register u32 mask_hi asm("a3");
    u32 *ot;
    u32 tag;

    base = (u8 *)&D_800A3D30;
    __asm__ volatile("ori %0, %0, 0xFFFF" : "=r"(mask_lo) : "0"(mask_lo), "r"(base));
    p = base + (D_800A3218 << 3);
    mask_hi = (u32)0xFF000000;

    p[3] = 1;
    *(u32 *)(p + 4) = code;

    ot = (u32 *)D_800A374C;
    tag = *(u32 *)p;
    tag = (tag & mask_hi) | (*ot & mask_lo);
    *(u32 *)p = tag;
    *ot = (*ot & mask_hi) | ((u32)p & mask_lo);
}
extern u32 D_800A3930;
void func_8003D39C(s32 x, s32 y, s32 ch, s32 color) {
    register s32 sx asm("t0") = x;
    register s32 sy asm("t1") = y;
    s32 count;
    u8 *p;
    register u32 tag asm("a1");
    register s32 scolor asm("t2") = color;

    count = D_800A3358;
    if (count == 0x20) return;

    {
        register u32 mask_lo asm("a3") = 0x00FFFFFF;
        u32 *ot;
        u32 mask_hi;
        u32 cmd_tag;
        u32 base;

        D_800A3358 = count + 1;
        __asm__ volatile("" ::: "memory");
        cmd_tag = 0x74000000;
        base = (count << 4) + (u32)&D_800A3930;
        {
            register u8 *pp asm("a0") = (u8 *)((D_800A3218 << 9) + base);

            pp[3] = 3;
            pp[7] = 0x74;
            pp[0xC] = ((ch & 7) * 8) - 64;
            pp[0xD] = ((ch >> 5) * 8) - 32;
            *((u16 *)(pp + 0xE)) = ((ch << 3) & 0xC0) | 0x773F;

            {
                u32 hc;
                __asm__ volatile("sra %0, %1, 1" : "=r"(hc) : "r"(scolor));
                tag = *((u32 *)pp);
                *((u32 *)(pp + 4)) = hc | cmd_tag;
            }

            ot = (u32 *)D_800A374C;
            mask_hi = 0xFF000000;
            *((s16 *)(pp + 8)) = sx;
            *((s16 *)(pp + 0xA)) = sy;

            tag = (tag & mask_hi) | ((*ot) & mask_lo);
            *((u32 *)pp) = tag;
            *ot = ((*ot) & mask_hi) | (((u32)pp) & mask_lo);
        }
    }
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
INCLUDE_ASM("asm/funcs", DispSleepMenuTex);
/* kengo:LOW  |  su_menu_home/_DispSleepMenuTex  |  146i  |  PS2 UI — reverted */
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
extern s32 func_8003D888(u32 *, s32);
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    s32 new_var2;
    u8 *new_var;
    u8 *p = base;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        do {
            nbits = func_8003D888((s32 *)base, 4);
            if (nbits == 0) {
                nbits = 16;
            }
            new_var = p;
            val = (s16)func_8003D888((s32 *)base, nbits);
        } while (0);
        if (!val) {
        }
        sign_bit = nbits - 1;
        new_var2 = 1;
        if ((val >> sign_bit) & new_var2) {
            val = val | (0xFFFF << sign_bit);
        }
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(new_var + 0xC) + val);
        i++;
        p += 2;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}

s32 func_8003D888(u32 *a0_param, s32 a1_param)
{
  register u32 *a2 asm("$6");
  register s32 a3 asm("$7");
  register s32 v0 asm("$2");
  register u32 v1 asm("$3");
  register s32 a0 asm("$4");
  register s32 a1 asm("$5");

  __asm__ __volatile__("addu\t$6, $4, $0" : "=r"(a2));
  __asm__ __volatile__("addu\t$7, $5, $0" : "=r"(a3));
  a1 = *((s32 *)((u8 *)a2 + 8));
  if (a1 < a3) {
    a3 -= a1;
    a0 = 1;
    v0 = (a0 << a1) - 1;
    a0 = (a0 << a3) - 1;
    v1 = *((u32 *)((u8 *)a2 + 4));
    a1 = *(s32 *)a2;
    v1 &= v0;
    v0 = a1 + 4;
    *(u32 *)a2 = v0;
    v0 = 32 - a3;
    a1 = *(s32 *)a1;
    v1 <<= a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v0 = (u32)a1 >> v0;
    v0 &= a0;
    v1 |= v0;
    *((u32 *)((u8 *)a2 + 4)) = a1;
  } else {
    v1 = *((u32 *)((u8 *)a2 + 4));
    v0 = a1 - a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v1 >>= v0;
    v0 = 1;
    v0 = (v0 << a3) - 1;
    v1 &= v0;
  }
  return (s32)v1;
}
extern s32 light_effect_col;
extern s32 D_800A4340;
void func_8003D91C(void) {
    s16 buf[4];
    s16 s1 = 0x1E0;
    s16 s0 = 0x140;
    buf[0] = 0;
    buf[1] = s1;
    buf[2] = s0;
    buf[3] = 1;
    func_8003D9A0(buf, 0x1F, &light_effect_col);
    buf[2] = 0x40;
    buf[0] = s0;
    buf[1] = s1;
    buf[3] = 8;
    func_8003D9A0(buf, 0x13, &D_800A4340);
}
typedef struct { s32 w[6]; } Copy24;
void func_8003D9A0(s16 *a0, s32 a1, u32 *a2) {
    register s16 *s0 asm("s0") = a0;
    register u32 *s1 asm("s1") = a2;
    s32 s4, s3;
    s32 s2;

    asm("" : : "r"(s1));
    s4 = s0[0];
    s3 = s0[1];

    if (a1 != 0) {
        s2 = a1 - 1;
        do {
            s16 v0;
            v0 = (u16)s0[1] + (u16)s0[3];
            s0[1] = v0;
            if (v0 >= 0x200) {
                s0[1] = s3;
                s0[0] = (u16)s0[0] + (u16)s0[2];
            }
            initLoadImage(s1, s0, s4, s3);
            *(Copy24 *)((u8 *)s1 + 0x18) = *(Copy24 *)s1;
            s1 = (u32 *)((u8 *)s1 + 0x30);
        } while (--s2 != -1);
    }
}
INCLUDE_ASM("asm/funcs", func_8003DA8C);
INCLUDE_ASM("asm/funcs", func_8003DBE4);
void func_8003DDF8(u32 arg0) {
    u32 *ptr = (u32 *)D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}
INCLUDE_ASM("asm/funcs", func_8003DE14);
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
extern void func_8007B6C8(s16 *, s32, s32);
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
    func_8007B6C8(buf, 0x140, 0x1E8);
    if (arg0 == 0) {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x140, 0xE8);
    } else {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x1C0, 0xE8);
    }
    gpu_DrawSync(0);
    func_8003E120();
end:
    D_800A3228 = arg0;
}
extern s32 D_800A3228;
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
extern s32 D_800A3228;
s32 func_8003E2A0(void) {
    return D_800A3228;
}
extern u16 g_game_p1_ctrl;
void func_8003E2AC(void) {
    u16 *p = &g_game_p1_ctrl;
    *p = *p & 0xFFFD;
}
u32 func_8003E2C8(void) {
    return D_800905F8;
}
INCLUDE_ASM("asm/funcs", replay_camera_get_attack_number);
/* kengo:HIGH  |  nm_replay_cam/replay_camera_get_attack_number  |  242i */
void func_8003E6A0(s32 arg0, s32 arg1) {
    replay_camera_get_attack_number(D_80101E3C, D_80101E44, arg0, arg1);
}
INCLUDE_ASM("asm/funcs", DispHira);
/* kengo:MED  |  am_rmd/DispHira  |  299i */
INCLUDE_ASM("asm/funcs", func_8003EB84);
INCLUDE_ASM("asm/funcs", md_game_check_mode);
/* kengo:HIGH  |  md_game/md_game_check_mode  |  234i */
