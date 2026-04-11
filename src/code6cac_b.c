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
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);

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
extern void func_80037774(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
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
extern u8 D_80106A73;
extern s32 *func_80077D00(void);
extern s16 D_80101ED6;
extern s32 g_file_disc_size;
extern s32 func_8002BC68(s32 a0);
extern void func_8001F860(s16 *arg0, s32 arg1);
extern void calc_loc_mat_fw(s32 a0);
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_RODATA("asm/rodata", jtbl_8001042C);
INCLUDE_ASM("asm/funcs", func_80026DA4);
s32 func_800272FC(s32 a0) {
    s32 v1;
    if (a0 < 0) {
        a0 = -a0;
    }
    if (a0 < 0xA01) {
        if (a0 < 0x600) {
            v1 = 1;
        } else {
            v1 = 2;
        }
    } else {
        v1 = 0;
    }
    return v1;
}
void func_80027334(s16 *arg0) {
    arg0[0x1E] = 0x3F5;
    arg0[0x1F] = 0x2B6;
    arg0[0x20] = 0x77A;
    arg0[0x21] = 0xBEE;
    arg0[0x22] = 0x8C;
    arg0[0x23] = 0x227;
    arg0[0x27] = 0x8A0;
    arg0[0x28] = 0xF0E;
    arg0[0x24] = 0;
    arg0[0x25] = 0;
    arg0[0x26] = 0;
    arg0[0x29] = 0xBCD;
}
void func_8002738C(s32 a0, s32 a1) {
    if (D_800A38DC != 0) {
        return;
    }
    switch (a1) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            *(&D_800A376A + a0) |= 0x10;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            *(&D_800A376A + a0) |= 0x01;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
            *(&D_800A376A + a0) |= 0x02;
            break;
        case 14:
        case 15:
        case 16:
        case 17:
            *(&D_800A376A + a0) |= 0x04;
            break;
        case 18:
        case 19:
        case 20:
        case 21:
            *(&D_800A376A + a0) |= 0x08;
            break;
    }
}
void func_80027438(u8 *a0, s32 a1, s16 a2) {
    s16 v1;
    v1 = D_800A38DC;
    if (v1 == 5) {
        return;
    }
    if ((u32)a1 >= 0x16) {
        return;
    }
    switch (a1) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            *(u16 *)(a0 + 0x272) += a2;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            *(u16 *)(a0 + 0x26C) = 0;
            *(u16 *)(a0 + 0x90) = 0;
            *(u16 *)(a0 + 0x8A) = 0;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
            *(u16 *)(a0 + 0x270) += a2;
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
            *(u16 *)(a0 + 0x26E) += a2;
            break;
    }
}
INCLUDE_ASM("asm/funcs", func_800274BC);
INCLUDE_ASM("asm/funcs", cpu_side_move_dir);
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir  |  160i  |  x4 size collision */
extern s32 func_80032854(s32, s32, u8 *, s16 *);
void func_800278C0(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 v1_obj;
    u8 *arg_a2 = stack_a2;
    s32 arg_v1 = stack_v1;

    if (a0 == 1) {
        return;
    }

    if (arg_v1 != 0) {
        v1_obj = *ptr;
        cmd = 0x2B;
        goto call_with_field4;
    }

    v1_obj = *ptr;
    {
        s16 f86 = *(s16 *)(v1_obj + 0x86);
        s16 f8E = *(s16 *)(v1_obj + 0x8E);

        if (f86 == f8E) {
            a0 = *(s16 *)(v1_obj + 0x4);
            cmd = 0x28;
            goto do_call;
        }

        {
            s16 f88 = *(s16 *)(v1_obj + 0x88);
            if (f86 == f88 && a3 != 0) {
                a0 = *(s16 *)(v1_obj + 0x4);
                cmd = 0x27;
                goto do_call;
            }
        }
    }

    if (cmd == 0) {
        v1_obj = *ptr;
        cmd = 0x22;
        goto call_with_field4;
    }

    if (cmd < 6) {
        cmd = 0x23;
        v1_obj = *ptr;
        goto call_with_field4;
    }

    v1_obj = *ptr;
    cmd = 0x24;

call_with_field4:
    a0 = *(s16 *)(v1_obj + 0x4);

do_call:
    func_80032854(a0, cmd, arg_a2, (s16 *)0);
}
/* TABLED: -16 bytes. 6 params, prologue register shuffling (t0/a1/a2/v1 reorder), lhu+sll+sra vs lh. */
s32 func_8002798C(u8 *a0) {
    s32 ret = 0;
    u16 v1 = *(u16 *)(a0 + 0x6A);
    
    if (v1 == 6 || v1 == 0x25 || v1 == 0x33 || v1 == 4 || v1 == 0x14) {
        goto check_88;
    }
    {
        s16 v40 = *(s16 *)(a0 + 0x40);
        if (v40 < *(u8 *)(a0 + 0xA5)) {
            goto check_88;
        }
        if (!(*(u8 *)(a0 + 0xA6) < v40)) {
            goto final_1;
        }
    }
    
check_88:
    {
        s16 v88 = *(s16 *)(a0 + 0x88);
        s32 a0_6a;
        if (v88 == -1) {
            goto done;
        }
        a0_6a = *(u16 *)(a0 + 0x6A);
        if ((u16)a0_6a == 0xF) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x1C) < 2) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x1E) < 2) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x20) < 2) {
            goto final_1;
        }
        if ((u16)a0_6a == 0x2B) {
            goto final_1;
        }
        goto done;
    }
    
final_1:
    ret = 1;
done:
    return ret;
}


extern s32 game_GetPlayerData(s32);
void func_80027A58(s32 *a0) {
    s16 v1 = *(s16 *)((u8 *)a0 + 0x86);
    if (v1 == *(s16 *)((u8 *)a0 + 0x88)) {
        if (*(s16 *)((u8 *)a0 + 0x8A)) {
            if (func_8002798C()) {
                s32 v0 = game_GetPlayerData(*(s16 *)((u8 *)a0 + 4));
                func_80030900(a0, *(s32 *)(v0 + 0x4C) + 0x14);
                *(s16 *)((u8 *)a0 + 0x8A) = 0;
                *(s16 *)((u8 *)a0 + 0x86) = *(u16 *)((u8 *)a0 + 0x84);
            }
        }
    }
}
INCLUDE_RODATA("asm/rodata", jtbl_80010548);
INCLUDE_ASM("asm/funcs", func_80027AD8);
INCLUDE_ASM("asm/funcs", saTan2KabutoWareMove);
/* kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  215i */
void func_8002872C(void) {
    s32 i;
    s32 offset;
    u8 *base;
    register s32 cmp_a1 asm("a1");
    register s32 cmp_a2 asm("a2");

    i = 0;
    offset = 0;

    do {
        base = &D_80101EC8 + offset;

        cmp_a1 = *(s16 *)(base + 0xC);
        if (cmp_a1 != 0x1B) goto next;

        if (*(s16 *)(base + 0x46) != 0) goto next;

        cmp_a2 = *(u16 *)(base + 0x6A);
        asm("" : "+r"(cmp_a2));
        if (cmp_a2 != 0xB) goto next;

        if (*(s16 *)(base + 0x40) != *(u8 *)(base + 0xA7)) goto next;

        {
            s32 *ptr = *(s32 **)base;
            register s32 a0_raw asm("a0");
            s32 v1;

            a0_raw = *(u16 *)((u8 *)ptr + 0x6A);
            asm("" : "+r"(a0_raw));
            v1 = a0_raw & 0xFFFF;

            if (v1 == 0x26) goto match;
            if (v1 == cmp_a1) goto match;
            if (v1 == 2) goto match;
            if (v1 == 0x15) goto match;
            if ((u32)(a0_raw - 0x24) < 2) goto match;
            if (v1 == 8) goto match;
            if ((u32)(a0_raw - 0x22) < 2) goto match;
            if (v1 == 0) goto match;
            if (v1 == 0x10) goto match;
            if (v1 == 0x13) goto match;
            if ((u32)(a0_raw - 0x30) < 2) goto match;
            if (v1 == 0x1A) goto match;
            if (v1 == cmp_a2) goto match;
            if (v1 == 0x12) goto match;
            if (v1 == 0x2A) goto match;
            if (v1 == 0xC) goto match;
            if (v1 != 0x19) goto next;
            asm("" : : "r"(a0_raw));

        match:
            if (!(D_800A387C < D_800A3134)) goto next;

            *(s16 *)((u8 *)*(s32 **)base + 0x286) = 0x1C;
            func_80027A58(*(s32 **)base);

            {
                s32 *p2 = *(s32 **)base;
                if (*(u16 *)((u8 *)p2 + 0x6A) == 0x25) {
                    *(u16 *)((u8 *)p2 + 0x86) = *(u16 *)((u8 *)p2 + 0x84);
                }
            }
        }

    next:
        i++;
        offset += 0x44C;
    } while (i < 2);
}
INCLUDE_ASM("asm/funcs", saTan3MainJump);
/* kengo:HIGH  |  sa_tan3/saTan3MainJump  |  492i  |  +3 near-exact */
void func_8002906C(void) {
    s16 *ptr = snd_GetSeId();
    while (*(s16 *)ptr != 0) {
        *(s16 *)((u8 *)ptr + 2) = 0;
        ptr = (s16 *)((u8 *)ptr + 0x10);
    }
}
INCLUDE_ASM("asm/funcs", DispPracticeMenuTex_C);
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
INCLUDE_ASM("asm/funcs", func_80029454);
INCLUDE_ASM("asm/funcs", func_8002A458);
INCLUDE_ASM("asm/funcs", calc_loc_mat_fw);
/* kengo:MED  |  se_fc/calc_loc_mat_fw  |  1074i  |  -38 3.5% no-affinity fallback */
INCLUDE_ASM("asm/funcs", func_8002BC68);
INCLUDE_ASM("asm/funcs", func_8002BEA0);
void func_8002C0DC(void) {
    register s32 var_s1 asm("s1");
    register u8 *var_s0 asm("s0");
    register s32 temp_s2 asm("s2");

    var_s1 = 0;
    var_s0 = &D_80101EC8;
    temp_s2 = func_8002BC68(D_800A371C);

    do {
        u8 *ptr;
        s32 arg1, arg2;
        var_s1++;
        ptr = *(u8 **)var_s0;
        arg1 = *(s32 *)(var_s0 + 0xD8);
        arg2 = *(s32 *)(var_s0 + 0xE0);
        arg1 = *(s32 *)(ptr + 0xD8) - arg1;
        arg2 = *(s32 *)(ptr + 0xE0) - arg2;
        func_8001F860((s16 *)var_s0, func_8007FD5C(arg1, arg2));
        var_s0 += 0x44C;
    } while (var_s1 < 2);

    {
        s32 idx;
        s32 chk;
        idx = D_800A38AE;
        chk = D_800A376E;
        var_s0 = &D_80101EC8 + idx * 0x44C;

        if (chk == 0) {
            if (D_800A3758 == 0xFF) {
                if (*(u8 *)(var_s0 + 0xAA) == *(s16 *)(var_s0 + 0x40)) {
                    calc_loc_mat_fw(1);
                }
            }
        }

        {
            s32 v1;
            v1 = *(s16 *)(var_s0 + 0x40);
            if (v1 < (s32)D_800A38E8) {
                return;
            }
            if (v1 >= *(u8 *)(var_s0 + 0xAA)) {
                return;
            }
            if (D_800A371C + 0xC8 >= temp_s2) {
                return;
            }
            {
                u8 *v1ptr;
                v1ptr = *(u8 **)var_s0;
                *(s16 *)(var_s0 + 0x286) = 4;
                *(s16 *)(v1ptr + 0x286) = 5;
            }
        }
    }
}
INCLUDE_ASM("asm/funcs", PutRobShadow);
/* kengo:MED  |  am_rmd/PutRobShadow  |  252i */
INCLUDE_ASM("asm/funcs", func_8002C61C);
INCLUDE_ASM("asm/funcs", func_8002CA8C);
INCLUDE_ASM("asm/funcs", special_camera_Init);
/* kengo:HIGH  |  nm_special_cam/special_camera_Init  |  370i */
INCLUDE_ASM("asm/funcs", func_8002D320);
INCLUDE_ASM("asm/funcs", saTan5TakeAnim2);
/* kengo:MED  |  sa_tan5/saTan5TakeAnim2  |  154i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", saTan0KiWareMoveA);
/* kengo:MED  |  sa_tan0/saTan0KiWareMoveA  |  212i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", saTan0KiWareMoveB);
/* kengo:MED  |  sa_tan0/saTan0KiWareMoveB  |  212i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", func_8002DE20);
INCLUDE_ASM("asm/funcs", pad_main_control);
/* kengo:HIGH  |  is_pad/pad_main_control  |  98i */
INCLUDE_ASM("asm/funcs", saSeInit);
/* kengo:MED  |  sa_se/saSeInit  |  123i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", func_8002EA24);
INCLUDE_ASM("asm/funcs", DispSchoolBG);
/* kengo:LOW  |  su_menu_single/_DispSchoolBG  |  188i  |  PS2 UI — reverted */
void func_8002EECC(void *arg0, void *arg1) {
    s16 temp_a3;
    s16 temp_t0;
    s16 temp_t1;
    s16 temp_t2;
    s16 temp_v0;
    s16 temp_v1;
    s32 temp_a2;
    s32 temp_v1_2;

    temp_t2 = *(s16 *)((u8 *)arg0 + 0xA);
    temp_t1 = *(s16 *)((u8 *)arg0 + 0xE);
    temp_t0 = *(s16 *)((u8 *)arg0 + 8);
    temp_a3 = *(s16 *)((u8 *)arg0 + 0x10);
    temp_a2 = (temp_t2 * temp_t1) - (temp_t0 * temp_a3);
    temp_v1 = *(s16 *)((u8 *)arg0 + 2);
    temp_v0 = *(s16 *)((u8 *)arg0 + 4);
    temp_v1_2 = (s32) ((*(s16 *)((u8 *)arg0 + 0) * (temp_a2 >> 0xC)) + (*(s16 *)((u8 *)arg0 + 6) * ((s32) ((temp_v1 * temp_a3) - (temp_v0 * temp_t1)) >> 0xC)) + (*(s16 *)((u8 *)arg0 + 0xC) * ((s32) ((temp_v0 * temp_t0) - (temp_v1 * temp_t2)) >> 0xC))) >> 0xC;
    *(s16 *)((u8 *)arg1 + 0) = (s16) (temp_a2 / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 2) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0x10)) - (*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 0xE))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 4) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 8)) - (*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0xA))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 6) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 6) * *(s16 *)((u8 *)arg0 + 0x10)) - (*(s16 *)((u8 *)arg0 + 0xA) * *(s16 *)((u8 *)arg0 + 0xC))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 8) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 0xC)) - (*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0x10))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xA) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0xA)) - (*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 6))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xC) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 8) * *(s16 *)((u8 *)arg0 + 0xC)) - (*(s16 *)((u8 *)arg0 + 6) * *(s16 *)((u8 *)arg0 + 0xE))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xE) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0xE)) - (*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0xC))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0x10) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 6)) - (*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 8))) / temp_v1_2);
}
INCLUDE_ASM("asm/funcs", func_8002F2D0);
INCLUDE_ASM("asm/funcs", func_8002F770);
INCLUDE_ASM("asm/funcs", cpu_check_tubazeri);
/* kengo:HIGH  |  nm_cpu/cpu_check_tubazeri  |  76i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", coli_check_circle_hit_line);
/* kengo:HIGH  |  is_coli/coli_check_circle_hit_line  |  92i */
INCLUDE_ASM("asm/funcs", func_8002FF20);
INCLUDE_ASM("asm/funcs", func_800300B4);
void func_80030208(void) {
    u8 *base;
    s32 i;
    u8 *p;
    s16 v1;
    s32 lookup;
    u8 *ptr1;
    u8 *ptr0;

    base = (u8 *)&D_80106A78;
    i = 0;
    p = base + 0xA;
loop:
    v1 = *(s16 *)(p - 8);
    if (v1 == -1) goto increment;
    if (*(u8 *)(p - 2) != 0) {
        func_800300B4(base);
        i++;
        goto next;
    }
    if (*(s16 *)base < 2) goto increment;

    lookup = (&D_8008EB80)[v1];
    if ((u16)(v1 - 0x12) < 0xC) {
        lookup = *(u8 *)(p + 1);
        goto call_funcs;
    }
    if (v1 != 0xE) {
        goto call_funcs;
    }
    if (*(u8 *)(p - 5) != 2) {
        goto call_with_a1;
    }
    lookup += 3;

call_funcs:
    ;
call_with_a1:
    ptr1 = base + 0x2C;
    ptr0 = base + 0x54;
    func_80049718(lookup, 1, ptr1, ptr0);
    saSeInit_2(*(u8 *)p, lookup, ptr1, ptr0);

increment:
    i++;
next:
    p += 0x64;
    base += 0x64;
    if (i < 12) goto loop;
}
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle;
    s16 cos_val;
    s16 sin_val;
    s32 vx;
    s32 vz;
    s32 rx;
    s32 rz;
    s32 v48;
    angle = func_8007FD5C(a1[0], a1[2]);
    cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    vx = *((s32 *)(((u8 *)a0) + 0x44));
    do {
        sin_val = *((&Judge) + (angle & 0xFFF));
        rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
        vz = *((s32 *)(((u8 *)a0) + 0x4C));
        rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
        rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
        v48 = *((s32 *)(((u8 *)a0) + 0x48));
        *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
        *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
        if (v48 < 0) {
            v48 += 3;
        }
    } while (0);
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
void func_8003043C(void) {
    s32 i = 0;
    s16 neg = -1;
    u8 val = 0xFF;
    s32 off = 0;
    do {
        *(s16 *)((u8 *)&D_80106A7A + off) = neg;
        *(u8 *)((u8 *)&D_80106A82 + off) = val;
        off += 0x64;
    } while (++i < 12);
}

void func_8003047C(u8 *a0) {
    s32 i;
    u8 *table;
    s32 neg1;
    u8 *a2;
    s16 val;
    s16 idx;

    *(u16 *)(a0 + 0x330) = 0;
    i = 0;
    table = &D_8008E338;
    neg1 = -1;
    a2 = a0;
    goto loop_start;
loop_start:
    idx = *(s16 *)(a0 + 0xA);
    val = (s8)*(table + idx * 5 + i);
    *(s16 *)(a2 + 0x332) = val;
    if (val != neg1) {
        u16 cnt = *(u16 *)(a0 + 0x330);
        *(u16 *)(a0 + 0x330) = cnt + 1;
    }
    i++;
    if (i < 5) {
        a2 += 2;
        goto loop_start;
    }

    idx = *(s16 *)(a0 + 0xA);
    {
        s16 idx2 = *(s16 *)(a0 + 0x4);
        *(&D_800A36F2 + idx2) = *(&D_8008E338 + idx * 5);
    }
}


void func_80030524(void) {
    s32 i = 0;
    s32 neg = -1;
    s16 *p = &D_80106A7A;
    s32 off = 0;
    do {
        if (*p != neg) {
            if (*(u8 *)((u8 *)&D_80106A80 + off) != 0) {
                *p = neg;
            }
        }
        p = (s16 *)((u8 *)p + 0x64);
        off += 0x64;
    } while (++i < 12);
}
INCLUDE_ASM("asm/funcs", coli_hit_body_weapon);
/* kengo:HIGH  |  is_coli/coli_hit_body_weapon  |  148i */
/* TABLED: -4 bytes, beqz delay slot scheduling (GCC fills with move v1,s2 instead of move a2,v0) */
INCLUDE_ASM("asm/funcs", cpu_check_tubazeri_2);
typedef struct { s32 x, y, z; } Vec3_copy;
extern s32 rng_Next(void);
extern s32 *coli_hit_body_weapon(s32 *, s32);
void func_80030900(u8 *a0, s32 *a1) {
    s32 *p;
    s32 rnd;
    s32 i;

    p = coli_hit_body_weapon((s32 *)a0, *(s16 *)(a0 + 0x332));
    *((u8 *)p + 4) = 0;
    *(Vec3_copy *)((u8 *)p + 0x2C) = *(Vec3_copy *)a1;
    *(s32 *)((u8 *)p + 0x44) = (rng_Next() & 0xFF) - 0x80;
    *(s32 *)((u8 *)p + 0x48) = -(rng_Next() & 0x3F) - 0x80;
    *(s32 *)((u8 *)p + 0x4C) = (rng_Next() & 0xFF) - 0x80;
    rnd = rng_Next();
    if (rnd & 0x1000) {
        *(s16 *)((u8 *)p + 0x5C) = (rnd & 0x3FF) + 0x200;
    } else {
        *(s16 *)((u8 *)p + 0x5C) = -(rnd & 0x3FF) - 0x200;
    }
    *(s16 *)((u8 *)p + 0x5E) = (rng_Next() & 0x7FF) - 0x400;
    *(s16 *)((u8 *)p + 0x60) = 0;
    *((u8 *)p + 7) = 1;
    for (i = 0; i < *(s16 *)(a0 + 0x330) - 1; i++) {
        *(u16 *)(a0 + 0x332 + i * 2) = *(u16 *)(a0 + 0x334 + i * 2);
    }
    *(s16 *)(a0 + 0x330) = (s16)(*(u16 *)(a0 + 0x330) - 1);
}
void cpu_set_move_command_and_dir(s32 *a0, s32 a1, s32 *a2) {
    s32 *p;
    s32 rnd;

    p = coli_hit_body_weapon(a0, a1);
    *((u8 *)p + 4) = 0;
    *(Vec3_copy *)((u8 *)p + 0x2C) = *(Vec3_copy *)a2;
    *(s32 *)((u8 *)p + 0x44) = (rng_Next() & 0xFF) - 0x80;
    *(s32 *)((u8 *)p + 0x48) = -(rng_Next() & 0x3F) - 0x80;
    *(s32 *)((u8 *)p + 0x4C) = (rng_Next() & 0xFF) - 0x80;
    rnd = rng_Next();
    if (rnd & 0x1000) {
        *(s16 *)((u8 *)p + 0x5C) = (rnd & 0x3FF) + 0x200;
    } else {
        *(s16 *)((u8 *)p + 0x5C) = -(rnd & 0x3FF) - 0x200;
    }
    *(s16 *)((u8 *)p + 0x5E) = (rng_Next() & 0x7FF) - 0x400;
    *(s16 *)((u8 *)p + 0x60) = 0;
    *((u8 *)p + 7) = 1;
    *((u8 *)p + 0xB) = *(u16 *)((u8 *)a0 + 0x12);
}


s32 func_80030B10(u8 *arg0, s32 arg1) {
    register u8 *ptr asm("a0") = arg0;
    register s32 a1val asm("a1") = arg1;
    register s32 count asm("a2") = *(s16 *)(ptr + 0x330);
    if (count == 0xC) {
        return 0;
    }
    {
        register s32 v1 asm("v1") = *(s16 *)(ptr + 0x88);
        if (v1 == -1) {
            goto L80030B7C;
        }
        __asm__ volatile("" :: "r"(v1));
    }
    if (a1val != *(s16 *)(ptr + 0x14)) {
        goto L80030B7C;
    }
    if (count <= 0) {
        goto L80030B68;
    }
    {
        register s32 v0 asm("v0") = count << 1;
        register u8 *v1 asm("v1");
        __asm__ volatile("addu %0,%1,$4" : "=r"(v1) : "r"(v0));
        do {
            v0 = *(u16 *)(v1 + 0x330);
            count--;
            *(u16 *)(v1 + 0x332) = (u16)v0;
            v1 -= 2;
        } while (count > 0);
    }
L80030B68:
    {
        register u16 v0 asm("v0") = *(u16 *)(ptr + 0x330);
        *(u16 *)(ptr + 0x332) = (u16)a1val;
        v0++;
        *(u16 *)(ptr + 0x330) = v0;
        goto L80030B9C;
    }
L80030B7C:
    {
        register u16 v0 asm("v0") = *(u16 *)(ptr + 0x330);
        register s32 v1 asm("v1") = (s32)v0 + 1;
        __asm__ volatile("" :: "r"(v0), "r"(v1));
        *(u16 *)(ptr + 0x330) = (u16)v1;
        *(u16 *)(ptr + (s16)v0 * 2 + 0x332) = (u16)a1val;
    }
L80030B9C:
    return 1;
}

INCLUDE_ASM("asm/funcs", func_80030BA8);
void func_80030D04(void) {
    s32 i = 0;
    s32 neg = -1;
    u16 *p = (u16 *)&D_80106A7A;
    do {
        if ((u32)(*p - 0x12) < 12u) {
            *p = neg;
        }
        p = (u16 *)((u8 *)p + 0x64);
    } while (++i < 12);
}
void func_80030D48(void) {
}
s32 func_80030D50(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = (arg0 - arg1) & 0xFFF;
    if (arg0 >= 0x800) {
        arg0 -= 0x1000;
    }
    return arg1 + ((arg0 * arg2) >> 12);
}
INCLUDE_ASM("asm/funcs", func_80030D7C);
INCLUDE_ASM("asm/funcs", func_80031890);
INCLUDE_ASM("asm/funcs", func_80031B24);
void func_80032040(void) {
    s32 i;
    for (i = 0x84; i >= 0; i -= 0x2C) {
        (&D_80104E88)[i] = 0;
    }
}
extern s32 func_80032854(s32, s32, u8 *, s16 *);
u8 *func_80032064(u8 *a0, s32 a1) {
    register u8 *src asm("a2");
    register s32 type asm("a3");
    s32 mul;
    s32 sw_val;
    s32 i;
    u8 *ptr;
    u8 *s0;
    s16 sp_area[2];

    src = a0;
    type = a1;
    mul = 0x50;
    sw_val = -0xC8;
    i = 0;
    ptr = &D_80104E88;

    for (i = 0; i < 4; i++) {
        s0 = ptr;
        if (*s0 == 0) break;
        ptr = s0 + 0x2C;
    }
    if (i == 4) return 0;

    *s0 = type;
    *(s0 + 1) = 1;
    *(s0 + 2) = 0;
    *(s0 + 3) = *(u16 *)(src + 4);
    *(s32 *)(s0 + 4) = *(s32 *)(src + 0xF4);
    {
        s32 v1 = *(s16 *)(src + 0x1A);
        if (v1 < 0) v1 += 0x1F;
        *(s32 *)(s0 + 8) = *(s32 *)(src + 0xBC) - (v1 >> 5);
    }
    *(s32 *)(s0 + 0xC) = *(s32 *)(src + 0xFC);
    *(s32 *)(s0 + 0x1C) = ((s32)*(&Judge + (*(u16 *)(src + 0x1CA) & 0xFFF)) * mul) >> 12;
    *(s32 *)(s0 + 0x20) = sw_val;
    *(s32 *)(s0 + 0x24) = ((s32)*(&Judge + ((*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF)) * mul) >> 12;
    *(Vec3_copy *)(s0 + 0x10) = *(Vec3_copy *)(s0 + 4);
    *(s32 *)(s0 + 0x28) = *(s32 *)(src + 0xBC);
    sp_area[1] = *(u16 *)(src + 0x1CA);
    {
        s32 a0_arg = *(u8 *)(src + 0xB2);
        s32 cmd = 0xD;
        u8 *v1 = s0 + 4;
        if (type == 1) cmd = 0xC;
        func_80032854(a0_arg, cmd, v1, sp_area);
    }
    return s0;
}
/* TABLED: same size, score 1450 — GCC body scheduling entirely different from target (delay slot fill choices, register allocation for intermediates) */
INCLUDE_ASM("asm/funcs", myRobGeneiDraw2);
INCLUDE_ASM("asm/funcs", Pad_Prs);
/* kengo:HIGH  |  is_pad/Pad_Prs  |  111i */
void cpu_get_dist_2(u8 *a0) {
    register u8 *v1 asm("v1");
    register u8 v0 asm("v0");
    register u32 a2 asm("a2");
    register u8 a1 asm("a1");

    v1 = *(u8 **)(a0 + 0x58);
    v0 = 0xFF;
    a0[0xA1] = v0;
    a0[0xA3] = v0;
    a0[0xA2] = v0;
    a0[0xA4] = v0;
    a0[0xAA] = 0;
    a0[0xA7] = 0;
    a0[0xA8] = 0;
    a0[0xA5] = 0;
    a0[0xA6] = v0;
    a0[0xAB] = v0;
    a0[0xAC] = v0;

    v0 = v1[4];
    v1 += 5;
    if (v0 == 0) return;

    do {
        a2 = v0;
        if (a2 == 0xFF) {
            v1 += 6;
        } else if (a2 < 0x80) {
            v1++;
        } else {
            a2 -= 0x80;
            a1 = *v1;
            v1++;
            if (a2 < 12) {
                switch (a2) {
                    case 0: a0[0xA1] = a1; break;
                    case 1: a0[0xA3] = a1; break;
                    case 2: a0[0xA7] = a1; break;
                    case 3: a0[0xA8] = a1; break;
                    case 4: a0[0xA9] = a1; break;
                    case 5: a0[0xA5] = a1; break;
                    case 6: a0[0xA6] = a1; break;
                    case 7: a0[0xA2] = a1; break;
                    case 8: a0[0xA4] = a1; break;
                    case 9: a0[0xAA] = a1; break;
                    case 10: a0[0xAB] = a1; break;
                    case 11: a0[0xAC] = a1; break;
                }
            }
        }
        v0 = *v1;
        v1++;
    } while (v0 != 0);
}
INCLUDE_ASM("asm/funcs", func_800325E0);
INCLUDE_RODATA("asm/rodata", jtbl_800105D0);
INCLUDE_ASM("asm/funcs", func_80032854);
INCLUDE_RODATA("asm/rodata", jtbl_80010698);
INCLUDE_ASM("asm/funcs", func_80032C50);
void cpu_check_same_dir_timer(s32 *a0) {
    u8 *s0;
    register s32 a1val asm("a1");
    s32 *base = a0;
    s32 *p;

    p = *(s32 **)((u8 *)base + 0x58);
    a1val = *(u8 *)((u8 *)p + 4);
    if (a1val == 0) goto done;
    s0 = (u8 *)p + 5;

loop:
    a1val = a1val & 0xFF;
    if (a1val == 0xFF) {
        u8 b0 = s0[0];
        u8 b1 = s0[1];
        u8 b2 = s0[2];
        u8 b3 = s0[3];
        s32 packed;
        s16 shift;
        s32 mask;

        packed = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
        shift = *(s16 *)((u8 *)base + 0xA);
        mask = 1 << shift;
        if ((packed & mask) != 0) {
            s0 += 4;
        } else {
            s0 += 6;
        }
        goto next;
    }

    if ((u32)a1val < 0x80) {
        u8 val = s0[0];
        s16 dir = *(s16 *)((u8 *)base + 0x40);
        s0 += 1;
        if ((val & 0xFF) == dir) {
            {
                register s32 *arg0 asm("a0") = base;
                asm("" : "=r"(arg0) : "0"(arg0));
                func_80032C50(arg0, a1val - 1);
            }
            goto next;
        }
        if (dir < val) {
            goto done;
        }
        asm volatile("");
        goto next;
    }

    s0 += 1;

next:
    a1val = *s0;
    s0 += 1;
    if (a1val != 0) goto loop;

done:
    return;
}
/* kengo:HIGH  |  nm_cpu/cpu_check_same_dir_timer  |  63i */
s32 func_80033498(void) {
    switch ((s16)(*(u16 *)&D_800A36A4 - 2)) {
    case 0:
        return 0;
    case 2:
        return 1;
    case 5:
        return 2;
    case 6:
        return 3;
    case 16:
        return 4;
    case 22:
        return 5;
    default:
        return 0xFF;
    }
}
extern u8 D_800A391D;
void func_80033510(void) {
    s32 i = 3;
    s16 *p1 = &D_800A3756;
    do {
        *p1 = 0;
        i--;
        p1--;
    } while (i >= 0);
    i = 5;
    {
        u8 *p2 = &D_800A391D;
        do {
            *p2 = 0;
            i--;
            p2--;
        } while (i >= 0);
    }
}
void func_80033550(s32 *arg0)
{
  int new_var;
  register s32 *a3_arg asm("a3") = arg0;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  do { idx = new_var; w0 = a3_arg[0]; } while (0);
  w1 = a3_arg[1];
  w2 = a3_arg[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}

INCLUDE_ASM("asm/funcs", mk_leaf_newpos);
void func_80033898(void) {
    gpu_EnableDisplay();
    D_800A37B8 = 0;
    D_800A3834 = 3;
}
INCLUDE_ASM("asm/funcs", cpu_set_move_command_and_dir_for_no_action);
/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action  |  189i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", mottest_disp);
/* TABLED: score 235, 94/94 insns. GCC eliminates andi v1,a0,0xFF after lbu (knows 0-255), inline asm prevents load interleaving. */
void func_80033D38(void) {
    register u8 *t1 asm("t1") = (u8 *)&g_file_disc_size;
    register s32 a3 asm("a3");
    register s32 a0 asm("a0");
    register s32 v1 asm("v1");
    register s32 t0 asm("t0");
    register u8 *a2 asm("a2");
    register u8 *v0p asm("v0");

    __asm__ volatile ("li %0,3" : "=r"(a3));
    a0 = D_800A3858;
    __asm__ volatile ("addiu %0,%1,-1" : "=r"(v1) : "r"(a3));
loop_1:
    __asm__ volatile ("sll %0,%1,3
	addu %0,%2,%0" : "=r"(v0p) : "r"(v1), "r"(t1));
    if (*((s32 *)(v0p + 0xC)) >= a0) {
        a3 = v1;
        v1 = a3 - 1;
        if (a3 > 0) {
            goto loop_1;
        }
    }
    D_800A38E9 = a3;
    if (a3 < 3) {
        t0 = 2;
        if (a3 < 2) {
            a2 = t1 + 0x10;
        loop_2:
            {
                s32 w0 = *((s32 *)(a2 + 0));
                s32 w1 = *((s32 *)(a2 + 4));
                *((s32 *)(a2 + 8)) = w0;
                *((s32 *)(a2 + 0xC)) = w1;
                __asm__ volatile ("" ::: "memory");
                t0 -= 1;
                a2 -= 8;
            }
            if (a3 < t0) goto loop_2;
        }
        {
            register u8 *pv1 asm("v1") = t1 + (a3 * 8);
            *((u8 *)(pv1 + 8)) = (u8)D_80101ED2;
            *((u8 *)(pv1 + 9)) = (u8)D_80101ED6;
            *((s32 *)(pv1 + 0xC)) = D_800A3858;
        }
    }
}
INCLUDE_ASM("asm/funcs", func_80033DF4);
void func_80033FE4(void) {
    s32 v1;
    if (D_800A36F0 != 0) {
        v1 = 6;
        if (*((u8 *)&D_8008D9EC + D_80101ED2) != 0) {
            v1 = 7;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A3781 != 0) {
        v1 = 8;
        if (*((u8 *)&D_8008D9EC + D_80101ED2) != 0) {
            v1 = 9;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A38E9 < 3) {
        D_800A3834 = 0x1A;
    } else {
        D_800A3834 = 8;
    }
}


void func_800340A0(void) {
    u8 p1, p2, round;

    p1 = D_800A3898;
    if ((u8)p1 == D_800A37F8) {
        round = D_800A3874;
        *(&D_800A377C + round) = 0;
    } else {
        p2 = D_800A3899;
        if ((u8)p2 == D_800A37F8) {
            round = D_800A3874;
            *(&D_800A377C + round) = 1;
        } else if ((u8)p2 < (u8)p1) {
            round = D_800A3874;
            *(&D_800A377C + round) = 0;
        } else if ((u8)p1 < (u8)p2) {
            round = D_800A3874;
            *(&D_800A377C + round) = 1;
        } else {
            if ((u8)D_800A38AA < (u8)D_800A38AB) {
                D_800A3898 = p1 + 1;
                round = D_800A3874;
                *(&D_800A377C + round) = 0;
            } else if ((u8)D_800A38AB < (u8)D_800A38AA) {
                D_800A3899 = p2 + 1;
                round = D_800A3874;
                *(&D_800A377C + round) = 1;
            } else {
                round = D_800A3874;
                *(&D_800A377C + round) = 2;
            }
        }
    }
    *(&D_800F65F8 + (D_800A3874 * 2)) = D_800A3898;
    *(&D_800F65F9 + (D_800A3874 * 2)) = D_800A3899;
    D_800A3874 = D_800A3874 + 1;
}
void func_80034200(void) {
    register s32 a2 asm("a2");
    register s32 a3 asm("a3");
    register s32 a0 asm("a0");
    register u8 *a1 asm("a1");
    register u8 *v1 asm("v1");
    register u8 *t0 asm("t0");
    register s32 t1 asm("t1");
    register s32 t3 asm("t3");
    register s32 t2 asm("t2");
    s32 count;
    volatile char _pad[8];
    (void)_pad;

    a2 = 0;
    a3 = 0;
    count = D_800A389B;
    g_disp_enable = DISP_LOADING;
    a0 = (count & 0) + 0;
    if (count <= 0) {
        goto end;
    }
    t3 = D_800A3874;
    t2 = 3;
    a1 = &D_800F65F8;

outerloop:
    t1 = (a3 < t3);
    v1 = a1;
    t0 = a1 + 2;
innerloop:
    {
        s32 v0;
        if (t1) {
            v0 = ((s32)*v1) << a2;
        } else {
            v0 = t2 << a2;
        }
        a0 |= v0;
        v1++;
        a2 += 2;
    }
    if ((s32)v1 < (s32)t0) goto innerloop;
    {
        s32 v0;
        v0 = D_800A389B;
        a3++;
        if (a3 < v0) { a1 += 2; goto outerloop; }
    }

end:
    D_800A3784 = a0;
}
extern void func_80034200(void);
extern void func_800372C0(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
extern void func_80034200(void);
extern void func_800372C0(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
void func_800342A0(void) {
    func_80034200();
    if (D_800A3874 == D_800A389B) {
        g_disp_enable = DISP_DISABLED;
        g_disp_fade = 0;
        D_800A3834 = 0x14;
    } else {
        s32 v1;
        u8 *a1 = &D_801027A0;
        u8 *a0 = &D_801027D8;
        D_800A38F4 = 0;
        D_800A3899 = 0;
        D_800A3898 = 0;
        D_800A38AB = 0;
        D_800A38AA = 0;
        D_800A3816 = 0;
        D_800A391E = 0x50;
        D_800A381E = 0;
        D_800A37E1 = 0;
        D_800A36E8 = 0;
        D_800A38B8 = 0;
        D_800A3920 = 0;
        v1 = 0;
loop:
        {
            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a1;
            v0 = *((u8 *)v0 + v1);
            *((u8 *)&D_8010277C + v1) = v0;
        }
        {
            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a0;
            v0 = *((u8 *)v0 + v1);
            *((u8 *)&D_8010277E + v1) = v0;
        }
        v1++;
        if (v1 < 2) goto loop;
        gpu_InitDisplay();
        func_800372C0();
        D_800A3834 = 0;
    }
}


void func_800343F0(void) {
    s8 val_85 = (s8)D_80102785;
    s8 val_86 = (s8)D_80102786;
    s8 val_84 = (s8)D_80102784;
    s32 val_87 = *(s8 *)&D_80102787;

    D_800A36F6 = 0;
    D_800A38DC = val_85;
    D_800A38BA = val_86;
    D_800A3140 = val_87;
    D_800A36A4 = val_84;
    player_SetCharId(0, 0);
    player_SetCharId(1, 0);
    D_800A376A = 0;
    D_800A376B = 0;
    D_800A380C = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}


INCLUDE_ASM("asm/funcs", DispSamnailWindow);
/* kengo:LOW  |  su_menu_vs/_DispSamnailWindow  |  149i  |  PS2 UI — reverted */
INCLUDE_ASM("asm/funcs", func_80034708);
/* TABLED: -4 bytes, score 1980. Target alternates v1/a0 for g_file_flags address — unreproducible register allocation pattern */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    *ptr = val2;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    *ptr = val2;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    *ptr = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
/* TABLED: -4 bytes. Branch inversion (beqz→bnez+ori delay slot), load order (lbu before lw vs lw,lbu fill), byte caching. Best: volatile ptr + inverted cond gives bnez+ori but lbu before lw. */
INCLUDE_ASM("asm/funcs", func_8003504C);
INCLUDE_ASM("asm/funcs", single_game_SetWazaData);
void func_80035430(void) {
}
