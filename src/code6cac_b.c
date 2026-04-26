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
extern u8 D_8008E914;
extern u8 D_8008EA00;
extern s16 D_800A3750;
extern s32 func_8001DB58(void);
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
extern s8 D_80102781;
extern u8 D_800A3768;
extern u8 D_800A36A8;


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
extern u8 D_8008E908;
extern u8 D_8008EC24;
extern s32 D_80106A50;
extern s32 func_8007FD5C(s32, s32);
extern s32 func_80079154(void);
extern void func_8007F87C(s32, s32 *);
extern void func_8007FA1C(s32, s32 *);
extern s16 D_80101E74;
extern void motutil_CheckKamaeReq(void);

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern s32 stage_GetDataPtr(void);

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
extern s32 D_80106A58;
extern s16 D_80101ED6;
extern s32 g_file_disc_size;
extern s32 func_8002BC68(s32 a0);
extern s32 D_80102410;
extern s32 D_80102408;
extern s32 D_80101FC4;
extern s32 D_80101FBC;
extern s16 D_800A3824;
extern s16 D_800A3876;
extern s16 D_800A38A8;
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
extern u8 D_8008D118;
void func_800274BC(s32 *arg0, s16 *arg1) {
    u32 dist_sq = (u32)((arg0[0] * arg0[0]) + (arg0[1] * arg0[1]) + (arg0[2] * arg0[2]));
    u32 log2_val;
    u8 *new_var;
    if (dist_sq < 0x400) {
        log2_val = ((u32)(*((&D_8008D118) + dist_sq))) >> 3;
    } else {
        s32 sp_tmp;
        register s32 t4_v asm("t4");
        t4_v = (s32)dist_sq;
        __asm__ volatile(".word 0x488CF000" : : "r"(t4_v));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        t4_v = (s32)(&sp_tmp);
        __asm__ volatile(".word 0xE99F0000" : : "r"(t4_v));
        {
            u32 clz = sp_tmp;
            u32 v0_m = clz & (-2);
            u32 v1_m = 0x16 - v0_m;
            u32 idx = dist_sq >> v1_m;
            u32 hi = (u32)((u8)(*((new_var = &D_8008D118) + idx)));
            do { v0_m = 0x13 - (v1_m >> 1); } while (0);
            log2_val = (hi << 16) >> v0_m;
        }
    }
    arg1[0] = (s16)(((-arg0[0]) << 12) / ((s32)log2_val));
    arg1[1] = (s16)(((-arg0[1]) << 12) / ((s32)log2_val));
    arg1[2] = (s16)(((-arg0[2]) << 12) / ((s32)log2_val));
}
extern void *func_80021424(s32, u16, s32);
extern s32 func_80021A98(s16, void *, s16);
extern s32 func_80032854(s16, s32, s32 *, s32);
void cpu_side_move_dir(s32 arg0)
{
    volatile s32 sp10;
    volatile s32 _g14;
    volatile s32 sp18;
    volatile s32 _g1C;
    int new_var;
    volatile s32 sp20;
    volatile s32 _g24;
    volatile s32 sp28;
    volatile s32 _g2C;
    s32 temp_s2;
    s32 v0_v;
    s32 var_a3;
    s32 v1_v;
    u8 cnt;
    s32 *stage_ptr;
    s32 s0_addr;
    temp_s2 = *((s16 *) (arg0 + 4));
    stage_ptr = (s32 *)stage_GetDataPtr();
    cnt = *((u8 *) (arg0 + 0x34C));
    if (((0, cnt)) < 0x40U) {
        *((u8 *) (arg0 + 0x34C)) = cnt + 1;
    }
    var_a3 = 0x3C - (((*((u8 *) (arg0 + 0x34C))) - 1) * 4);
    if (var_a3 < 0xA) {
        var_a3 = 0xA;
    }
    if (D_800A36A4 == 3) {
        v1_v = 0x2EE0;
        if ((*((s32 *) ((*((s32 *) arg0)) + 0xF4))) >= 0x3E9) {
            v1_v = -0x2710;
        }
        sp20 = v1_v;
        {
            s32 a2_v = 0x1770;
            if ((*((s32 *) ((*((s32 *) arg0)) + 0xFC))) > 0) {
                a2_v = -0x1770;
            }
            sp28 = a2_v;
            {
                s32 cnv = 0x64 - var_a3;
                sp10 = ((v1_v * var_a3) + ((*((s32 *) (arg0 + 0xF4))) * cnv)) / 100;
                v0_v = ((a2_v * var_a3) + ((*((s32 *) (arg0 + 0xFC))) * cnv)) / 100;
            }
        }
    } else {
        s32 *p = (s32 *) (((s32) stage_ptr) + (((D_800A36A4 * 0xC) + (temp_s2 * 3)) * 2));
        sp10 = *((s16 *) p);
        v0_v = *((s16 *) (((s32) p) + 4));
    }
    sp18 = v0_v;
    {
        s32 dx = sp10 - (*((s32 *) (arg0 + 0xF4)));
        sp10 = dx;
        sp18 = v0_v - (*((s32 *) (arg0 + 0xFC)));
        *((s32 *) (arg0 + 0xF4)) = (*((s32 *) (arg0 + 0xF4))) + dx;
        *((s32 *) (arg0 + 0xFC)) = (*((s32 *) (arg0 + 0xFC))) + sp18;
        *((s32 *) (arg0 + 0xD8)) = (*((s32 *) (arg0 + 0xD8))) + sp10;
        *((s32 *) (arg0 + 0xE0)) = (*((s32 *) (arg0 + 0xE0))) + sp18;
        s0_addr = arg0 + 0x5E;
        *((s32 *) (arg0 + 0xB8)) = (*((s32 *) (arg0 + 0xB8))) + sp10;
        new_var = (*((s32 *) (arg0 + 0xC0))) + sp18;
        *((s32 *) (arg0 + 0x104)) = 0;
        *((s32 *) (arg0 + 0x108)) = 0;
        *((s32 *) (arg0 + 0x10C)) = 0;
        *((s32 *) (arg0 + 0x134)) = 0;
        *((s32 *) (arg0 + 0x138)) = 0;
        *((s32 *) (arg0 + 0x13C)) = 0;
        *((s32 *) (arg0 + 0xC0)) = new_var;
    }
    {
        void *r1 = func_80021424(arg0, *((u16 *) (*((s32 *) (arg0 + 0x50)))), s0_addr);
        void *r2 = func_80021424(arg0, *((u16 *) (((s32) r1) + 0x3A)), s0_addr);
        func_80021A98((s16) temp_s2, r2, *((s16 *) (arg0 + 0x5E)));
    }
    func_80032854(*((s16 *) (arg0 + 4)), 0x30, (s32 *)(arg0 + 0xF4), 0);
}
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
s32 saTan2KabutoWareMove(u8 *arg0, u8 *arg1) {
    u16 temp_a1;
    u8 *temp_s4;
    s32 temp_v1;
    s32 var_s1;
    s16 var_v0;
    s32 ret;

    temp_a1 = *(u16 *)(arg0 + 0x6A);
    temp_s4 = *(u8 **)(arg0);
    temp_v1 = temp_a1 & 0xFFFF;
    ret = 1;
    if (temp_v1 != 4) {
        if (temp_v1 != 0x14) {
            u16 temp_v0 = *(u16 *)(temp_s4 + 0x6A);
            if ((temp_v0 != 4) && (temp_v0 != 0x14)) {
                s32 d_val;
                s32 temp_a1_2;
                s32 temp_s5;

                if (temp_v1 != 0x13) {
                    var_s1 = 0;
                    if (((u32)(temp_a1 - 0x19) >= 2U) && (temp_v1 != 2) && (temp_v1 != 0x26) && (temp_v1 != 0x1B) && (temp_v1 != 0x15) && (temp_v1 != 0x25) && (temp_v1 != 0x2C) && (temp_v1 != 0xC)) {
                        goto block_13;
                    }
                    goto block_15;
                }
                var_s1 = 0;
            block_15:
                d_val = D_800A3824;
                temp_a1_2 = (d_val >> *(s16 *)(arg0 + 4)) & 1;
                temp_s5 = (d_val >> *(s16 *)(temp_s4 + 4)) & 1;
                if (*(s16 *)(arg0 + 0x8C) != 0) {
                    var_s1 = temp_a1_2 == 0;
                }
                if (var_s1 != 0) {
                    s16 temp_v1_2 = *(s16 *)(arg0 + 0xC);
                    if (temp_v1_2 != 0x1D) {
                        if (temp_v1_2 != 0xE) {
                            goto block_20;
                        }
                        return ret;
                    }
                    goto block_49;
                }
            block_20:
                {
                    s32 temp_v0_2 = temp_a1_2 * 2;
                    s16 temp_v1_3 = *(s16 *)(temp_s4 + temp_s5 * 2 + 0x288);
                    if (temp_v1_3 == 0) {
                        if (*(s16 *)(arg0 + temp_v0_2 + 0x288) > 0) {
                            var_v0 = 0x19;
                            if (var_s1 == 0) {
                            set_0xB:
                                var_v0 = 0xB;
                            }
                        do_store_calls:
                            *(s16 *)(arg0 + 0x286) = var_v0;
                        do_calls:
                            func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                            func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            return ret;
                        }
                        goto block_49;
                    }
                    {
                        u8 *temp_s3 = arg0 + temp_v0_2;
                        s16 temp_v0_3 = *(s16 *)(temp_s3 + 0x288);
                        s16 var_v0_2;
                        if (temp_v0_3 == temp_v1_3) {
                            func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                            func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            if (*(s16 *)(temp_s3 + 0x288) == 5) {
                                if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
                                    var_v0_2 = 0x19;
                                    if (var_s1 == 0) {
                                        var_v0_2 = 0xB;
                                    }
                                    goto block_48;
                                }
                                D_800A38A8 = 1;
                                D_800A3876 = -1;
                                return ret;
                            }
                            var_v0_2 = 0x19;
                            if (var_s1 == 0) {
                                var_v0_2 = 0xB;
                            }
                            goto block_48;
                        }
                        if (temp_v1_3 < temp_v0_3) {
                            if (var_s1 != 0) {
                                *(s16 *)(arg0 + 0x286) = 0x19;
                            } else {
                                goto set_0xB;
                            }
                            goto do_calls;
                        }
                        func_80032854(*(s16 *)(arg0 + 4), 0x26, arg1, (s16 *)0);
                        func_80032854(*(s16 *)(arg0 + 4), 0x2D, arg1, (s16 *)0);
                        var_v0_2 = 0x1A;
                        if (var_s1 == 0) {
                            u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);
                            s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);
                            s32 temp_a0_2 = *(s32 *)(temp_a0 + 0x118);
                            s32 var_a1 = temp_a0_2;
                            s32 temp_v1_5 = (s32)((&Judge)[((temp_v1_4 + 0x400) & 0xFFF)] * *(s32 *)(temp_a0 + 0x114) + (&Judge)[(temp_v1_4 & 0xFFF)] * *(s32 *)(temp_a0 + 0x11C)) >> 0xC;
                            s32 var_v0_3;
                            if (temp_a0_2 < 0) {
                                var_a1 = -temp_a0_2;
                            }
                            var_v0_3 = temp_v1_5;
                            if (temp_v1_5 < 0) {
                                var_v0_3 = -temp_v1_5;
                            }
                            if (var_v0_3 < var_a1) {
                                var_v0_2 = 0x14;
                                if (temp_a0_2 > 0) {
                                    var_v0_2 = 0x13;
                                }
                            } else {
                                var_v0_2 = 0x15;
                                if (temp_v1_5 <= 0) {
                                    var_v0_2 = 0x16;
                                }
                            }
                        }
                    block_48:
                        *(s16 *)(arg0 + 0x286) = var_v0_2;
                    }
                }
            block_49:
                return ret;
            }
            goto block_13;
        }
        return ret;
    }
block_13:
    return ret;
}
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
s32 func_8002BC68(s32 arg0) {
    s32 temp_a3;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v1_3;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    u8 *t2_base;
    u8 *t3_base;

    temp_a3 = D_80101FA0 - D_801023EC;
    temp_t1 = D_80101FA8 - D_801023F4;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    t2_base = &D_80101EC8;
    t3_base = t2_base + 0x44C;
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32) (*((&D_8008D118) + temp_a0))) >> 3;
    } else {
        s32 sp_tmp;
        register s32 t4_v asm("t4");
        t4_v = (s32) temp_a0;
        __asm__ volatile(".word 0x488CF000" : : "r"(t4_v));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        t4_v = (s32) (&sp_tmp);
        __asm__ volatile(".word 0xE99F0000" : : "r"(t4_v));
        {
            u32 clz = sp_tmp;
            u32 v0_m = clz & (-2);
            u32 v1_m = 0x16 - v0_m;
            u32 idx = temp_a0 >> v1_m;
            u32 hi = (u32) ((u8) (*((&D_8008D118) + idx)));
            do { v0_m = 0x13 - (v1_m >> 1); var_t0 = (hi << 16) >> v0_m; } while (0);
        }
    }
    if (((s32) var_t0) < arg0) {
        var_a0 = ((arg0 - ((s32) var_t0)) * 0x50) / 100;
    } else {
        s32 v1_2 = arg0 - ((s32) var_t0);
        var_a0 = v1_2 >> 4;
        if (v1_2 < 0) {
            var_a0 = (v1_2 + 0xF) >> 4;
        }
    }
    temp_v0 = arg0 - 0x64;
    temp_v1_3 = -var_a0;
    *((s32 *) (t2_base + 0x134)) = (temp_a3 * var_a0) / temp_v0;
    *((s32 *) (t2_base + 0x13C)) = (temp_t1 * var_a0) / temp_v0;
    *((s32 *) (t3_base + 0x134)) = (temp_a3 * temp_v1_3) / temp_v0;
    *((s32 *) (t3_base + 0x13C)) = (temp_t1 * temp_v1_3) / temp_v0;
    return (s32) var_t0;
}
s32 func_8002BEA0(void) {
    s32 temp_a3;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v1_3;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    u8 *t2_base;
    u8 *t3_base;

    temp_a3 = D_80101FBC - D_80102408;
    temp_t1 = D_80101FC4 - D_80102410;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    t2_base = &D_80101EC8;
    t3_base = t2_base + 0x44C;
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32) (*((&D_8008D118) + temp_a0))) >> 3;
    } else {
        s32 sp_tmp;
        register s32 t4_v asm("t4");
        t4_v = (s32) temp_a0;
        __asm__ volatile(".word 0x488CF000" : : "r"(t4_v));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        t4_v = (s32) (&sp_tmp);
        __asm__ volatile(".word 0xE99F0000" : : "r"(t4_v));
        {
            u32 clz = sp_tmp;
            u32 v0_m = clz & (-2);
            u32 v1_m = 0x16 - v0_m;
            u32 idx = temp_a0 >> v1_m;
            u32 hi = (u32) ((u8) (*((&D_8008D118) + idx)));
            do { v0_m = 0x13 - (v1_m >> 1); var_t0 = (hi << 16) >> v0_m; } while (0);
        }
    }
    if (((s32) var_t0) < 0x44C) {
        var_a0 = ((0x44C - ((s32) var_t0)) * 0x50) / 100;
    } else {
        s32 v1_2 = 0x44C - ((s32) var_t0);
        var_a0 = v1_2 >> 4;
        if (v1_2 < 0) {
            var_a0 = (v1_2 + 0xF) >> 4;
        }
    }
    temp_v0 = 0x3E8;
    __asm__ volatile("" : "=r"(temp_v0) : "0"(temp_v0));
    temp_v1_3 = -var_a0;
    *((s32 *) (t2_base + 0x134)) = (temp_a3 * var_a0) / temp_v0;
    *((s32 *) (t2_base + 0x13C)) = (temp_t1 * var_a0) / temp_v0;
    *((s32 *) (t3_base + 0x134)) = (temp_a3 * temp_v1_3) / temp_v0;
    *((s32 *) (t3_base + 0x13C)) = (temp_t1 * temp_v1_3) / temp_v0;
    return (s32) var_t0 - 0x44C;
}

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
s32 func_8002D320(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    register s32 result asm("v0");
    __asm__ volatile (
        ".set\tnoreorder\n"
        ".set\tnoat\n"
        "addiu  $29, $29, -8\n"
        "lw     $8, 24($29)\n"
        "bnez   $4, 7f\n"
        " nop\n"
        "lw     $3, 96($5)\n"
        "lw     $2, 0($6)\n"
        "lw     $3, 0($3)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lw     $3, 96($5)\n"
        "sh     $2, 248($5)\n"
        "lw     $2, 4($6)\n"
        "lw     $3, 4($3)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lw     $3, 96($5)\n"
        "sh     $2, 250($5)\n"
        "lw     $2, 8($6)\n"
        "lw     $3, 8($3)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "sh     $2, 252($5)\n"
        "addiu  $2, $5, 248\n"
        "addu   $12, $2, $0\n"
        "lwc2   $0, 0($12)\n"
        "lwc2   $1, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        "addiu  $2, $5, 256\n"
        "addu   $12, $2, $0\n"
        "swc2   $25, 0($12)\n"
        ".word  0xE99A0004\n"
        ".word  0xE99B0008\n"
        "7:\n"
        "lw     $4, 256($5)\n"
        "negu   $6, $7\n"
        "slt    $2, $4, $6\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "slt    $2, $7, $4\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "lw     $3, 260($5)\n"
        "nop\n"
        "slt    $2, $3, $6\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "slt    $2, $7, $3\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "mult   $4, $4\n"
        "mflo   $2\n"
        "nop\n"
        "nop\n"
        "mult   $3, $3\n"
        "mflo   $3\n"
        "addu   $4, $2, $3\n"
        "slt    $2, $8, $4\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "subu   $4, $8, $4\n"
        "sltiu  $2, $4, 1024\n"
        "beqz   $2, 2f\n"
        " nop\n"
        "lui    $1, %%hi(D_8008D118)\n"
        "addu   $1, $1, $4\n"
        "lbu    $2, %%lo(D_8008D118)($1)\n"
        "j      3f\n"
        " srl   $4, $2, 3\n"
        "2:\n"
        "bltz   $4, 4f\n"
        " addu  $3, $0, $0\n"
        "addu   $12, $4, $0\n"
        ".word  0x488CF000\n"
        "nop\n"
        "nop\n"
        "addu   $12, $29, $0\n"
        ".word  0xE99F0000\n"
        "lw     $3, 0($29)\n"
        "4:\n"
        "addiu  $2, $0, -2\n"
        "and    $2, $3, $2\n"
        "addiu  $3, $0, 22\n"
        "subu   $3, $3, $2\n"
        "srlv   $2, $4, $3\n"
        "lui    $1, %%hi(D_8008D118)\n"
        "addu   $1, $1, $2\n"
        "lbu    $4, %%lo(D_8008D118)($1)\n"
        "srl    $3, $3, 1\n"
        "addiu  $2, $0, 19\n"
        "subu   $2, $2, $3\n"
        "sll    $4, $4, 16\n"
        "srlv   $4, $4, $2\n"
        "3:\n"
        "addu   $7, $0, $0\n"
        "lw     $3, 176($5)\n"
        "nop\n"
        "bgez   $3, 10f\n"
        " addu  $6, $0, $0\n"
        "j      11f\n"
        " addu  $6, $3, $0\n"
        "10:\n"
        "slt    $2, $6, $3\n"
        "beqz   $2, 11f\n"
        " nop\n"
        "addu   $7, $3, $0\n"
        "11:\n"
        "lw     $3, 192($5)\n"
        "nop\n"
        "slt    $2, $3, $6\n"
        "beqz   $2, 12f\n"
        " slt   $2, $7, $3\n"
        "j      13f\n"
        " addu  $6, $3, $0\n"
        "12:\n"
        "beqz   $2, 13f\n"
        " nop\n"
        "addu   $7, $3, $0\n"
        "13:\n"
        "lw     $5, 264($5)\n"
        "nop\n"
        "subu   $2, $5, $4\n"
        "slt    $2, $7, $2\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "addu   $2, $5, $4\n"
        "slt    $2, $2, $6\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "addiu  $2, $0, 1\n"
        "1:\n"
        "addiu  $29, $29, 8\n"
        ".set\tat\n"
        ".set\treorder\n"
        : "=r"(result)
    );
    return result;
}
s32 saTan5TakeAnim2(s32 threshold, s32 r_sq, s32 *p1, s32 *p2) {
    s32 x1, z1, x2, z2;

    x1 = p1[0];
    x2 = p2[0];

    if (x1 < x2) {
        if (x2 < -threshold) return 0;
        if (threshold >= x1) goto z_check;
        return 0;
    } else {
        if (x1 < -threshold) return 0;
        if (threshold < x2) return 0;
    }

z_check:
    z1 = p1[1];
    z2 = p2[1];

    if (z1 < z2) {
        if (z2 < -threshold) return 0;
        if (threshold >= z1) goto dist_calc;
        return 0;
    } else {
        if (z1 < -threshold) return 0;
        if (threshold < z2) return 0;
    }

dist_calc:
    {
        s32 ax = p2[0] - p1[0];
        s32 x1r = p1[0];
        s32 az = p2[1] - p1[1];
        s32 z1r = p1[1];

        s32 ax_sq = ax * ax;
        s32 az_sq = az * az;
        s32 cx = ax * x1r;
        s32 cz = az * z1r;
        s32 x1_sq = x1r * x1r;
        s32 z1_sq = z1r * z1r;

        s32 dot2 = (cx + cz) * 2;
        s32 dot2_9 = dot2 >> 9;
        s32 c_val = ((x1_sq + z1_sq) - r_sq) >> 9;
        s32 dist_sq = ax_sq + az_sq;

        s32 disc = (dot2_9 * dot2_9) - ((dist_sq >> 9) * (c_val << 2));
        s32 sqrt_val;
        s32 result;

        if (disc < 0) {
            result = 0;
        } else {
            if ((u32)disc < 0x400u) {
                sqrt_val = (&D_8008D118)[disc] >> 3;
            } else {
                s32 sp_tmp;
                s32 lzcr = 0;
                if (disc >= 0) {
                    register s32 t4_v asm("t4");
                    t4_v = disc;
                    __asm__ volatile(".word 0x488CF000" : : "r"(t4_v));
                    __asm__ volatile("nop");
                    __asm__ volatile("nop");
                    t4_v = (s32)(&sp_tmp);
                    __asm__ volatile(".word 0xE99F0000" : : "r"(t4_v));
                    lzcr = sp_tmp;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tval = (&D_8008D118)[(u32)disc >> shift];
                    s32 half = (u32)shift >> 1;
                    sqrt_val = (u32)(tval << 16) >> (0x13 - half);
                }
            }

            {
                s32 sq = sqrt_val << 9;
                s32 neg_b = -dot2;
                s32 denom = dist_sq * 2;
                s32 t1_val = ((neg_b + sq) << 8) / denom;
                s32 t2_val = ((neg_b - sq) << 8) / denom;

                result = 0;
                if (t1_val >= 0) {
                    result = t2_val < 0x101;
                }
            }
        }
        return result;
    }
}













/* kengo:MED  |  sa_tan5/saTan5TakeAnim2  |  154i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", saTan0KiWareMoveA);
/* kengo:MED  |  sa_tan0/saTan0KiWareMoveA  |  212i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", saTan0KiWareMoveB);
/* kengo:MED  |  sa_tan0/saTan0KiWareMoveB  |  212i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", func_8002DE20);
s32 pad_main_control(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 x0 = arg0[0];
    s32 z0 = arg0[2];
    s32 x1 = arg1[0];
    s32 z1 = arg1[2];
    s32 x2 = arg2[0];
    s32 z2 = arg2[2];
    s32 px = arg3[0];
    s32 pz = arg3[2];
    s32 center_x = (x0 + x1 + x2) / 3;
    s32 center_z = (z0 + z1 + z2) / 3;
    s32 dz = z1 - z0;
    s32 dx = x1 - x0;
    s32 cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
    s32 cross_point = (dz * (px - x0)) - (dx * (pz - z0));

    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }

    dz = z2 - z0;
    dx = x2 - x0;
    cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
    cross_point = (dz * (px - x0)) - (dx * (pz - z0));
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }

    dz = z2 - z1;
    dx = x2 - x1;
    cross_center = (dz * (center_x - x1)) - (dx * (center_z - z1));
    cross_point = (dz * (px - x1)) - (dx * (pz - z1));
    return (cross_center ^ cross_point) >= 0;
}
/* kengo:HIGH  |  is_pad/pad_main_control  |  98i */
void saSeInit(u8 *arg0) {
    s32 *mat;
    s32 sp10;
    s32 dist_sq;
    s32 log2_val;
    s32 shift;
    s32 idx;
    s32 z_diff;

    *(s32 *)(arg0 + 0xA8) = *(s32 *)(*(s32 *)(arg0 + 0x64) + 0) - *(s32 *)(*(s32 *)(arg0 + 0x60) + 0);
    *(s32 *)(arg0 + 0xAC) = *(s32 *)(*(s32 *)(arg0 + 0x64) + 4) - *(s32 *)(*(s32 *)(arg0 + 0x60) + 4);
    z_diff = *(s32 *)(*(s32 *)(arg0 + 0x64) + 8) - *(s32 *)(*(s32 *)(arg0 + 0x60) + 8);
    *(s32 *)(arg0 + 0xB0) = z_diff;
    *(s16 *)(arg0 + 0xFA) = 0x800 - func_8007FD5C(*(s32 *)(arg0 + 0xA8), z_diff);

    dist_sq = (*(s32 *)(arg0 + 0xA8) * *(s32 *)(arg0 + 0xA8))
            + (z_diff * z_diff);

    if ((u32)dist_sq < 0x400) {
        log2_val = ((u32)*(&D_8008D118 + dist_sq)) >> 3;
    } else {
        idx = 0;
        if (dist_sq >= 0) {
            register s32 t4 asm("t4") = dist_sq;

            __asm__ volatile(".word 0x488CF000" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            t4 = (s32)&sp10;
            __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
            idx = sp10;
        }

        idx &= -2;
        shift = 0x16 - idx;
        idx = (u32)dist_sq >> shift;
        log2_val = ((u32)*(&D_8008D118 + idx) << 16) >> (0x13 - (shift >> 1));
    }

    mat = (s32 *)(arg0 + 0xD8);
    *(s16 *)(arg0 + 0xF8) = 0x800 - func_8007FD5C(*(s32 *)(arg0 + 0xAC), log2_val);

    *(s16 *)(arg0 + 0xD8) = 0x1000;
    *(s16 *)(arg0 + 0xDA) = 0;
    *(s16 *)(arg0 + 0xDC) = 0;
    *(s16 *)(arg0 + 0xDE) = 0;
    *(s16 *)(arg0 + 0xE0) = 0x1000;
    *(s16 *)(arg0 + 0xE2) = 0;
    *(s16 *)(arg0 + 0xE4) = 0;
    *(s16 *)(arg0 + 0xE6) = 0;
    *(s16 *)(arg0 + 0xE8) = 0x1000;

    func_8007FA1C(*(s16 *)(arg0 + 0xFA), mat);
    func_8007F87C(*(s16 *)(arg0 + 0xF8), mat);

    {
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(mat));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile(".word 0x48CD0000" : : "r"(t5));
        __asm__ volatile(".word 0x48CE0800" : : "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);

            __asm__ volatile(".word 0x48CD1000" : : "r"(t5));
            __asm__ volatile(".word 0x48CE1800" : : "r"(t6));
            __asm__ volatile(".word 0x48CF2000" : : "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)(arg0 + 0xA8);
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 <<= 16;
        t5 |= t6;
        __asm__ volatile(".word 0x488D0000" : : "r"(t5));
        __asm__ volatile(".word 0xC9810008" : : "r"(t4));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4A486012");
        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        __asm__ volatile(".word 0xE9990000" : : "r"(t4));
        __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
        __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
    }
}
/* kengo:MED  |  sa_se/saSeInit  |  123i  |  x2 size collision */
s32 func_8002EA24(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    register s32 result asm("v0");
    __asm__ volatile (
        ".set\tnoreorder\n"
        ".set\tnoat\n"
        "addu   $8, $4, $0\n"
        "lw     $3, 96($8)\n"
        "lw     $2, 0($5)\n"
        "lw     $3, 0($3)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lw     $3, 96($8)\n"
        "sh     $2, 248($8)\n"
        "lw     $2, 4($5)\n"
        "lw     $3, 4($3)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lw     $3, 96($8)\n"
        "sh     $2, 250($8)\n"
        "lw     $2, 8($5)\n"
        "lw     $3, 8($3)\n"
        "addiu  $29, $29, -8\n"
        "subu   $2, $2, $3\n"
        "sh     $2, 252($8)\n"
        "addiu  $2, $8, 248\n"
        "addu   $12, $2, $0\n"
        "lwc2   $0, 0($12)\n"
        "lwc2   $1, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        "addiu  $2, $8, 256\n"
        "addu   $12, $2, $0\n"
        "swc2   $25, 0($12)\n"
        ".word  0xE99A0004\n"
        ".word  0xE99B0008\n"
        "lw     $5, 256($8)\n"
        "negu   $9, $6\n"
        "slt    $2, $5, $9\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "slt    $2, $6, $5\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "lw     $3, 260($8)\n"
        "nop\n"
        "slt    $2, $3, $9\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "slt    $2, $6, $3\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "mult   $5, $5\n"
        "mflo   $2\n"
        "nop\n"
        "nop\n"
        "mult   $3, $3\n"
        "mflo   $3\n"
        "addu   $4, $2, $3\n"
        "slt    $2, $7, $4\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "subu   $4, $7, $4\n"
        "sltiu  $2, $4, 1024\n"
        "beqz   $2, 2f\n"
        " nop\n"
        "lui    $1, %%hi(D_8008D118)\n"
        "addu   $1, $1, $4\n"
        "lbu    $2, %%lo(D_8008D118)($1)\n"
        "j      3f\n"
        " srl   $4, $2, 3\n"
        "2:\n"
        "bltz   $4, 4f\n"
        " addu  $3, $0, $0\n"
        "addu   $12, $4, $0\n"
        ".word  0x488CF000\n"
        "nop\n"
        "nop\n"
        "addu   $12, $29, $0\n"
        ".word  0xE99F0000\n"
        "lw     $3, 0($29)\n"
        "4:\n"
        "addiu  $2, $0, -2\n"
        "and    $2, $3, $2\n"
        "addiu  $3, $0, 22\n"
        "subu   $3, $3, $2\n"
        "srlv   $2, $4, $3\n"
        "lui    $1, %%hi(D_8008D118)\n"
        "addu   $1, $1, $2\n"
        "lbu    $4, %%lo(D_8008D118)($1)\n"
        "srl    $3, $3, 1\n"
        "addiu  $2, $0, 19\n"
        "subu   $2, $2, $3\n"
        "sll    $4, $4, 16\n"
        "srlv   $4, $4, $2\n"
        "3:\n"
        "addu   $5, $0, $0\n"
        "lw     $2, 176($8)\n"
        "nop\n"
        "bgez   $2, 5f\n"
        " addu  $6, $0, $0\n"
        "j      6f\n"
        " addu  $6, $2, $0\n"
        "5:\n"
        "addu   $5, $2, $0\n"
        "6:\n"
        "lw     $3, 264($8)\n"
        "nop\n"
        "subu   $2, $3, $4\n"
        "slt    $2, $5, $2\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "addu   $2, $3, $4\n"
        "slt    $2, $2, $6\n"
        "bnez   $2, 1f\n"
        " addu  $2, $0, $0\n"
        "addiu  $2, $0, 1\n"
        "1:\n"
        "addiu  $29, $29, 8\n"
        ".set\tat\n"
        ".set\treorder\n"
        : "=r"(result)
    );
    return result;
}
void DispSchoolBG(s16 *arg0, s16 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    s32 sp10;
    s32 dist_sq;
    s32 log2_val;
    s32 idx;
    s32 shift;
    s32 *mat;
    u8 *sp_base;
    s32 z_prod;
    s32 x_prod;
    s32 y_prod;

    *(s16 *)0x1F8003B2 = 0x800 - func_8007FD5C(arg1[0], arg1[2]);

    sp_base = (u8 *)0x1F8002B8;

    {
        s32 x = arg1[0];
        s32 y = arg1[2];
        dist_sq = (x * x) + (y * y);
    }

    if ((u32)dist_sq < 0x400) {
        log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
    } else {
        idx = 0;
        if (dist_sq >= 0) {
            register s32 t4 asm("t4") = dist_sq;
            __asm__ volatile(".word 0x488CF000" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            t4 = (s32)&sp10;
            __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
            idx = sp10;
        }
        idx &= -2;
        shift = 0x16 - idx;
        idx = (u32)dist_sq >> shift;
        log2_val = ((u32)*(&D_8008D118 + idx) << 16) >> (0x13 - (shift >> 1));
    }

    *(s16 *)(sp_base + 0xF8) = 0x800 - func_8007FD5C(arg1[1], log2_val);
    mat = (s32 *)(sp_base + 0xD8);

    *(s16 *)(sp_base + 0xD8) = 0x1000;
    *(s16 *)(sp_base + 0xDA) = 0;
    *(s16 *)(sp_base + 0xDC) = 0;
    *(s16 *)(sp_base + 0xDE) = 0;
    *(s16 *)(sp_base + 0xE0) = 0x1000;
    *(s16 *)(sp_base + 0xE2) = 0;
    *(s16 *)(sp_base + 0xE4) = 0;
    *(s16 *)(sp_base + 0xE6) = 0;
    *(s16 *)(sp_base + 0xE8) = 0x1000;

    func_8007FA1C(*(s16 *)(sp_base + 0xFA), mat);
    func_8007F87C(*(s16 *)(sp_base + 0xF8), mat);

    {
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(mat));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile(".word 0x48CD0000" : : "r"(t5));
        __asm__ volatile(".word 0x48CE0800" : : "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);

            __asm__ volatile(".word 0x48CD1000" : : "r"(t5));
            __asm__ volatile(".word 0x48CE1800" : : "r"(t6));
            __asm__ volatile(".word 0x48CF2000" : : "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)arg0;
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 <<= 16;
        t5 |= t6;
        __asm__ volatile(".word 0x488D0000" : : "r"(t5));
        __asm__ volatile(".word 0xC9810008" : : "r"(t4));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4A486012");
    }

    arg0 = (s16 *)(sp_base + 0xA8);
    {
        register s32 t4 asm("t4");
        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"((s32)arg0));
        __asm__ volatile(".word 0xE9990000" : : "r"(t4));
        __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
        __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
    }

    z_prod = *(s32 *)(sp_base + 0xB0) * arg3;
    if (z_prod < 0) {
        z_prod += 0xFF;
    }
    x_prod = *(s32 *)(sp_base + 0xA8) * arg4;
    *(s32 *)(sp_base + 0xB0) = z_prod >> 8;
    if (x_prod < 0) {
        x_prod += 0xFF;
    }
    y_prod = *(s32 *)(sp_base + 0xAC) * arg4;
    *(s32 *)(sp_base + 0xA8) = x_prod >> 8;
    if (y_prod < 0) {
        y_prod += 0xFF;
    }
    *(s32 *)(sp_base + 0xAC) = y_prod >> 8;

    *(s16 *)(sp_base + 0xD8) = 0x1000;
    *(s16 *)(sp_base + 0xDA) = 0;
    *(s16 *)(sp_base + 0xDC) = 0;
    *(s16 *)(sp_base + 0xDE) = 0;
    *(s16 *)(sp_base + 0xE0) = 0x1000;
    *(s16 *)(sp_base + 0xE2) = 0;
    *(s16 *)(sp_base + 0xE4) = 0;
    *(s16 *)(sp_base + 0xE6) = 0;
    *(s16 *)(sp_base + 0xE8) = 0x1000;

    func_8007F87C(-*(s16 *)(sp_base + 0xF8), mat);
    func_8007FA1C(-*(s16 *)(sp_base + 0xFA), mat);

    {
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(mat));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile(".word 0x48CD0000" : : "r"(t5));
        __asm__ volatile(".word 0x48CE0800" : : "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);

            __asm__ volatile(".word 0x48CD1000" : : "r"(t5));
            __asm__ volatile(".word 0x48CE1800" : : "r"(t6));
            __asm__ volatile(".word 0x48CF2000" : : "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)arg0;
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 <<= 16;
        t5 |= t6;
        __asm__ volatile(".word 0x488D0000" : : "r"(t5));
        __asm__ volatile(".word 0xC9810008" : : "r"(t4));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4A486012");
    }

    {
        register s32 t4 asm("t4");
        __asm__ volatile("addu %0, %1, $0" : "=r"(t4) : "r"((s32)arg2));
        __asm__ volatile(".word 0xE9990000" : : "r"(t4));
        __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
        __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
    }
}
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
s32 cpu_check_tubazeri(s32 *a0, s32 *a1, s32 *a2) {
    s32 *sp380;
    s32 result;

    __asm__ volatile (
        "lw     $2, 0($5)\n"
        "lw     $3, 0($4)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 864($1)\n"
        "lw     $2, 4($5)\n"
        "lw     $3, 4($4)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 868($1)\n"
        "lw     $2, 8($5)\n"
        "lw     $3, 8($4)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 872($1)\n"
        "lw     $2, 0($6)\n"
        "lw     $3, 0($4)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 880($1)\n"
        "lw     $2, 4($6)\n"
        "lw     $3, 4($4)\n"
        "nop\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 884($1)\n"
        "lw     $2, 8($6)\n"
        "lw     $3, 8($4)\n"
        "lui    $7, 0x1F80\n"
        "ori    $7, $7, 0x0360\n"
        "subu   $2, $2, $3\n"
        "lui    $1, 0x1F80\n"
        "sw     $2, 888($1)\n"
        "addu   $12, $7, $0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        "lui    $7, 0x1F80\n"
        "ori    $7, $7, 0x0370\n"
        "addu   $12, $7, $0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        "lui    $2, 0x1F80\n"
        "ori    $2, $2, 0x0380\n"
        "addu   $12, $2, $0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        : "=r"(sp380)
    );

    result = func_8007FD5C(sp380[0], *(s32 *)0x1F800388);
    if (*(s32 *)0x1F800384 > 0) {
        result += 0x800;
    }
    return result;
}
/* kengo:HIGH  |  nm_cpu/cpu_check_tubazeri  |  76i  |  x2 size collision */
s32 coli_check_circle_hit_line(s32 *arg0) {
    s32 result;

    __asm__ volatile (
        "lh     $3, 4($4)\n"
        "nop\n"
        "sll    $2, $3, 5\n"
        "addu   $2, $2, $3\n"
        "sll    $2, $2, 3\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 192($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $4, 180($1)\n"
        "nop\n"
        "subu   $3, $3, $4\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 864($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 196($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $4, 184($1)\n"
        "nop\n"
        "subu   $3, $3, $4\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 868($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 200($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $4, 188($1)\n"
        "nop\n"
        "subu   $3, $3, $4\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 872($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 204($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $4, 180($1)\n"
        "nop\n"
        "subu   $3, $3, $4\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 880($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 208($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $4, 184($1)\n"
        "nop\n"
        "subu   $3, $3, $4\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 884($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $3, 212($1)\n"
        "lui    $1, 0x1F80\n"
        "addu   $1, $2, $1\n"
        "lw     $2, 188($1)\n"
        "lui    $5, 0x1F80\n"
        "ori    $5, $5, 0x0360\n"
        "subu   $3, $3, $2\n"
        "lui    $1, 0x1F80\n"
        "sw     $3, 888($1)\n"
        "addu   $12, $5, $0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        "lui    $5, 0x1F80\n"
        "ori    $5, $5, 0x0370\n"
        "addu   $12, $5, $0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        "lui    $5, 0x1F80\n"
        "ori    $5, $5, 0x0380\n"
        "addu   $12, $5, $0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        "lui    $2, 0x1F80\n"
        "lw     $2, 900($2)\n"
        : "=r"(result)
    );
    return result > 0;
}


/* kengo:HIGH  |  is_coli/coli_check_circle_hit_line  |  92i */
void func_8002FF20(u8 *arg0, u8 arg1) {
    s32 sp10[8];
    s32 *temp_s1;
    s32 *temp_s2;
    s32 v0;

    *(u8 *)(arg0 + 8) = 1;
    *(u8 *)(arg0 + 9) = arg1;
    v0 = game_GetPlayerData(*(u8 *)(arg0 + 6) < 1);
    temp_s1 = (s32 *)(arg0 + 0xC);
    temp_s2 = *(s32 **)((*(u8 *)(arg0 + 9) * 4) + v0);
    *(s16 *)(arg0 + 0xC) = 0x1000;
    *(s16 *)(arg0 + 0xE) = 0;
    *(s16 *)(arg0 + 0x10) = 0;
    *(s16 *)(arg0 + 0x12) = 0;
    *(s16 *)(arg0 + 0x14) = 0x1000;
    *(s16 *)(arg0 + 0x16) = 0;
    *(s16 *)(arg0 + 0x18) = 0;
    *(s16 *)(arg0 + 0x1A) = 0;
    *(s16 *)(arg0 + 0x1C) = 0x1000;
    func_8007F87C(*(s16 *)(arg0 + 0x54), (s32)temp_s1);
    func_8007FA1C(*(s16 *)(arg0 + 0x56), (s32)temp_s1);
    func_8007FBBC(*(s16 *)(arg0 + 0x58), (s32)temp_s1);
    func_8002EECC(temp_s2, sp10);
    func_8007E4DC(sp10, temp_s1, temp_s1);

    *(s32 *)(arg0 + 0x2C) = *(s32 *)(arg0 + 0x2C) - *(s32 *)((u8 *)temp_s2 + 0x14);
    *(s32 *)(arg0 + 0x30) = *(s32 *)(arg0 + 0x30) - *(s32 *)((u8 *)temp_s2 + 0x18);
    *(s32 *)(arg0 + 0x34) = *(s32 *)(arg0 + 0x34) - *(s32 *)((u8 *)temp_s2 + 0x1C);

    {
        s32 *mat_ptr = sp10;
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");
        __asm__ volatile ("addu %0, %1, $0" : "=r"(t4) : "r"(mat_ptr));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile (".word 0x48CD0000" :: "r"(t5));
        __asm__ volatile (".word 0x48CE0800" :: "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);
            __asm__ volatile (".word 0x48CD1000" :: "r"(t5));
            __asm__ volatile (".word 0x48CE1800" :: "r"(t6));
            __asm__ volatile (".word 0x48CF2000" :: "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)(arg0 + 0x2C);
        register s32 t4 asm("t4");
        register s32 t6 asm("t6");
        register s32 t5 asm("t5");
        __asm__ volatile ("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile (".word 0x488D0000" :: "r"(t5));
        __asm__ volatile (".word 0xC9810008" :: "r"(t4));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4A486012");
        __asm__ volatile ("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        __asm__ volatile (".word 0xE9990000" :: "r"(t4));
        __asm__ volatile (".word 0xE99A0004" :: "r"(t4));
        __asm__ volatile (".word 0xE99B0008" :: "r"(t4));
    }

    *(s32 *)(arg0 + 0x2C) = *(s32 *)(arg0 + 0x2C) >> 1;
    *(s32 *)(arg0 + 0x30) = *(s32 *)(arg0 + 0x30) >> 1;
    *(s32 *)(arg0 + 0x34) = *(s32 *)(arg0 + 0x34) >> 1;
}

void func_800300B4(u8 *arg0) {
    s32 result[3];
    s16 out[4];
    s32 buf[8];
    s32 *a0_ptr;
    u8 se_id;

    a0_ptr = *(s32 **)(((s32 *)game_GetPlayerData(*(u8 *)(arg0 + 6) < 1)) + *(u8 *)(arg0 + 9));

    {
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");
        __asm__ volatile ("addu %0, %1, $0" : "=r"(t4) : "r"(a0_ptr));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile (".word 0x48CD0000" :: "r"(t5));
        __asm__ volatile (".word 0x48CE0800" :: "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);
            __asm__ volatile (".word 0x48CD1000" :: "r"(t5));
            __asm__ volatile (".word 0x48CE1800" :: "r"(t6));
            __asm__ volatile (".word 0x48CF2000" :: "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)(arg0 + 0x2C);
        register s32 t4 asm("t4");
        register s32 t6 asm("t6");
        register s32 t5 asm("t5");
        __asm__ volatile ("addu %0, %1, $0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile (".word 0x488D0000" :: "r"(t5));
        __asm__ volatile (".word 0xC9810008" :: "r"(t4));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4A486012");
    }

    {
        s32 *res_ptr = result;
        register s32 t4 asm("t4") = (s32)res_ptr;
        __asm__ volatile (".word 0xE9990000" :: "r"(t4));
        __asm__ volatile (".word 0xE99A0004" :: "r"(t4));
        __asm__ volatile (".word 0xE99B0008" :: "r"(t4));
    }

    result[0] += *(s32 *)((u8 *)a0_ptr + 0x14);
    result[1] += *(s32 *)((u8 *)a0_ptr + 0x18);
    result[2] += *(s32 *)((u8 *)a0_ptr + 0x1C);

    func_8007E4DC(a0_ptr, (s32 *)(arg0 + 0xC), buf);
    func_8002F2D0(buf, out);

    se_id = *(&D_8008EB80 + *(s16 *)(arg0 + 2));
    func_80049718(se_id, 1, result, out);
    saSeInit_2(*(u8 *)(arg0 + 0xA), se_id, result, out);
}
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
extern s32 *coli_hit_body_weapon(s32 *, s32);
extern s32 func_80032854(s32, s32, u8 *, s16 *);
s32 cpu_check_tubazeri_2(s32 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 *s2;
    s32 s3;
    s32 *a2;

    count = *(s16 *)((u8 *)a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)((u8 *)a0 + 0x88) == -1) {
        v0 = s1 << 1;
        goto after_sll;
    }
    v0 = *(s16 *)((u8 *)a0 + 0x332);
    v1 = *(s16 *)((u8 *)a0 + 0x14);
    s1 = (u32)(v0 ^ v1) < 1;
do_sll:
    v0 = s1 << 1;
after_sll:
    s2 = (s32 *)((u8 *)a0 + v0);
    s3 = *(s16 *)((u8 *)s2 + 0x332);
    a2 = coli_hit_body_weapon(a0, s3);
    v0 = s1;
    if (v0 < *(s16 *)((u8 *)a0 + 0x330) - 1) {
        s32 *ptr = s2;
        do {
            *(u16 *)((u8 *)ptr + 0x332) = *(u16 *)((u8 *)ptr + 0x334);
            v0++;
            ptr = (s32 *)((u8 *)ptr + 2);
        } while (v0 < *(s16 *)((u8 *)a0 + 0x330) - 1);
    }

    *(u16 *)(0, (u8 *)a0 + 0x330) = *(u16 *)((u8 *)a0 + 0x330) - 1;
    v1 = *(s16 *)((u8 *)a2 + 2);
    if (v1 == 0xE) {
        s32 a0_arg = D_800A36F2 ^ 0xE;
        func_80032854(a0_arg != 0, 0x2F, (u8 *)a2 + 0x2C, 0);
    } else {
        s32 a0_arg = v1 ^ D_800A36F2;
        func_80032854(a0_arg != 0, 0x2A, (u8 *)a2 + 0x2C, 0);
    }
    return s3;
}
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

s32 func_80030BA8(u8 *arg0) {
    s32 i = 0;
    s32 new_var;
    s32 neg1 = -1;
    u8 *p = (u8 *)&D_80106A7A;
    s32 old_val;

    loop:;
    {
        u16 val = *(u16 *)p;
        s32 sval;
        if ((unsigned)(val - 0x12) < 12u) {
            goto next;
        }
        sval = (s16)val;
        if (sval == neg1) {
            goto next;
        }
        if (*(s32 *)(p + 0x4E) != 0) {
            goto next;
        }
        {
            s32 bc = *(s32 *)(arg0 + 0xBC);
            s32 pos2e = *(s32 *)(p + 0x2E);
            if (bc - 0x64 >= pos2e) {
                goto next;
            }
            if (pos2e >= bc + 0x64) {
                goto next;
            }
        }
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - *(s32 *)(p + 0x2A);
            s32 dz = *(s32 *)(arg0 + 0xFC) - *(s32 *)(p + 0x32);
            new_var = 0xF423F;
            i++;
            if (dx * dx + dz * dz > new_var) {
                goto loop_test;
            }
        }
        if (func_80030B10(arg0, sval) == 0) {
            return -1;
        }
        old_val = (s32)(*(s16 *)p);
        *(s16 *)p = (s16)neg1;
        if (old_val == 0xE) {
            s32 a0val = D_800A36F2 ^ 0xE;
            func_80032854(a0val != 0, 0x2F, arg0 + 0xF4, 0);
        } else {
            func_80032854(*(s16 *)(arg0 + 4), 0x11, arg0 + 0xF4, 0);
        }
        return old_val;
    }
    next:
    i++;
    loop_test:
    if (i < 12) {
        p += 0x64;
        goto loop;
    }
    return -1;
}
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
void func_80031890(s32 arg0, s32 arg1, s32 arg2) {
    s32 *mat;
    s32 angle;
    s32 rot;

    if (*(s16 *)(arg1 + 2) != 14) {
        s32 vx = *(s32 *)(arg1 + 0x44);
        s32 vz = *(s32 *)(arg1 + 0x4C);
        s32 cur = *(s16 *)(arg1 + 0x5E);
        s32 mag = vx * vx + vz * vz;
        s32 dv;
        if (rng_Next() & 1) {
            dv = mag / 64;
        } else {
            dv = (-mag) / 64;
        }
        *(s16 *)(arg1 + 0x5E) = cur + dv;
    }

    mat = (s32 *)(arg0 + 0xD8);
    angle = (&D_8008EBA0)[arg2] & 0xFFF;

    {
        s32 pos0 = *(s32 *)(arg1 + 0x2C);
        s32 pos1 = *(s32 *)(arg1 + 0x30);
        s32 pos2 = *(s32 *)(arg1 + 0x34);

        *(s16 *)(arg0 + 0xD8) = 0x1000;
        *(s16 *)(arg0 + 0xDA) = 0;
        *(s16 *)(arg0 + 0xDC) = 0;
        *(s16 *)(arg0 + 0xDE) = 0;
        *(s16 *)(arg0 + 0xE0) = 0x1000;
        *(s16 *)(arg0 + 0xE2) = 0;
        *(s16 *)(arg0 + 0xE4) = 0;
        *(s16 *)(arg0 + 0xE6) = 0;
        *(s16 *)(arg0 + 0xE8) = 0x1000;

        rot = ((pos0 << 4) + pos1 + (pos2 << 3)) & 0x7FF;
        rot -= 0x400;
    }

    func_8007FA1C(angle, mat);
    func_8007F87C(rot, mat);

    {
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu\t%0,%1,$0" : "=r"(t4) : "r"(mat));
        t5 = *(s32 *)(t4);
        t6 = *(s32 *)(t4 + 4);
        __asm__ volatile(".word 0x48CD0000" : : "r"(t5));
        __asm__ volatile(".word 0x48CE0800" : : "r"(t6));
        t5 = *(s32 *)(t4 + 8);
        t6 = *(s32 *)(t4 + 0xC);
        {
            register s32 t7 asm("t7") = *(s32 *)(t4 + 0x10);
            __asm__ volatile(".word 0x48CD1000" : : "r"(t5));
            __asm__ volatile(".word 0x48CE1800" : : "r"(t6));
            __asm__ volatile(".word 0x48CF2000" : : "r"(t7));
        }
    }

    {
        s32 vtx_addr = (s32)(arg1 + 0x44);
        register s32 t4 asm("t4");
        register s32 t5 asm("t5");
        register s32 t6 asm("t6");

        __asm__ volatile("addu\t%0,%1,$0" : "=r"(t4) : "r"(vtx_addr));
        t6 = *(u16 *)(t4 + 4);
        t5 = *(u16 *)(t4);
        t6 <<= 16;
        t5 |= t6;
        __asm__ volatile(".word 0x488D0000" : : "r"(t5));
        __asm__ volatile(".word 0xC9810008" : : "r"(t4));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4A486012");
        __asm__ volatile("addu\t%0,%1,$0" : "=r"(t4) : "r"(vtx_addr));
        __asm__ volatile(".word 0xE9990000" : : "r"(t4));
        __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
        __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
    }

    if ((u32)(angle - 0x401) < 0x7FFu) {
        *(s32 *)(arg1 + 0x44) = *(s32 *)(arg1 + 0x44) / 8;
        *(s32 *)(arg1 + 0x48) = *(s32 *)(arg1 + 0x48) / 8;
        *(s32 *)(arg1 + 0x4C) = *(s32 *)(arg1 + 0x4C) / 8;
    } else {
        *(s32 *)(arg1 + 0x44) = *(s32 *)(arg1 + 0x44) / 4;
        *(s32 *)(arg1 + 0x48) = *(s32 *)(arg1 + 0x48) / 4;
        *(s32 *)(arg1 + 0x4C) = *(s32 *)(arg1 + 0x4C) / 4;
    }

    *(s32 *)(arg1 + 0x2C) += *(s32 *)(arg1 + 0x44) / 2;
    *(s32 *)(arg1 + 0x30) += *(s32 *)(arg1 + 0x48) / 2;
    *(s32 *)(arg1 + 0x34) += *(s32 *)(arg1 + 0x4C) / 2;
}
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
void myRobGeneiDraw2(void) {
    s32 *sp = (s32 *)0x1F8002B8;
    u8 *base = &D_80104E88;
    s32 i;
    u8 *s0;

    i = 0;
    do {
        if (*base != 0) {
            s0 = base + 4;
            s0[-2] += 1;
            *(Vec3_copy *)(s0 + 0xC) = *(Vec3_copy *)s0;
            *(s32 *)(s0 + 0x1C) += 0xD;
            sp[0] = *(s32 *)s0 + *(s32 *)(s0 + 0x18);
            sp[1] = *(s32 *)(s0 + 4) + *(s32 *)(s0 + 0x1C);
            {
                s32 arg5 = (s32)sp + 0x38;
                sp[2] = *(s32 *)(s0 + 8) + *(s32 *)(s0 + 0x20);
                arg5++;
                arg5--;
                if (func_8005344C((s32 *)s0, sp, (s32 *)((u8 *)sp + 0x10), (s32 *)((u8 *)sp + 0x30), arg5) != 0 || *(s32 *)(s0 + 0x24) < *(s32 *)(s0 + 4)) {
                    *base = 0;
                } else {
                    *(Vec3_copy *)s0 = *(Vec3_copy *)sp;
                }
            }
        }
        i++;
        s0 += 0x2C;
        base += 0x2C;
    } while (i < 4);
}
extern u8 D_8008D118;
extern void func_8005C650(s32, s32, s32);
void Pad_Prs(void) {
    register u8 *t0 asm("t0") = &D_80104E88;
    register s32 t1 asm("t1") = 0;
    register u8 *a3 asm("a3") = &D_80104E88 + 2;
    u8 *ent;
    s32 state;
    s32 a0;

loop:
    if (*t0 == 0) goto next;
    {
        s32 v1_v = (*(u8 *)(a3 + 1) == 0);
        v1_v = v1_v * 0x44C;
        ent = v1_v + &D_80101EC8;
    }
    state = *(u16 *)(ent + 0x6A);
    a0 = state & 0xFFFF;
    if (a0 == 4) goto next;
    if (a0 == 0x14) goto next;
    if (a0 == 0xF) goto next;
    if ((u32)(state - 0x1C) < 2) goto next;
    if ((u32)(state - 0x1E) < 2) goto next;
    if ((u32)(state - 0x20) < 2) goto next;
    if (a0 == 0x11) goto next;
    {
        s32 dx = *(s32 *)(ent + 0xF4) - *(s32 *)(a3 + 2);
        s32 dy = *(s32 *)(ent + 0xF8) - *(s32 *)(a3 + 6);
        s32 dz = *(s32 *)(ent + 0xFC) - *(s32 *)(a3 + 0xA);
        register u32 dist_sq asm("a0") = (u32)(dx * dx + dy * dy + dz * dz);
        u32 log2_val;
        if (dist_sq < 0x400) {
            log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
        } else {
            s32 clz = 0;
            s32 sp_tmp;
            register s32 t4_v asm("t4");
            if ((s32)dist_sq >= 0) {
                t4_v = (s32)dist_sq;
                __asm__ volatile (".word 0x488CF000" :: "r"(t4_v));
                __asm__ volatile ("nop");
                __asm__ volatile ("nop");
                t4_v = (s32)&sp_tmp;
                __asm__ volatile (".word 0xE99F0000" :: "r"(t4_v));
                clz = sp_tmp;
            }
            {
                u32 v0_m = (u32)(clz & -2);
                u32 v1_m = 0x16 - v0_m;
                u32 hi;
                u32 idx = dist_sq >> v1_m;
                hi = (u32)(u8)*(&D_8008D118 + idx);
                v0_m = 0x13 - (v1_m >> 1);
                log2_val = (hi << 16) >> v0_m;
            }
        }
        {
            s32 v1 = *a3;
            s32 v0 = v1 << 4;
            v0 = v0 - v1;
            v0 = v0 << 1;
            v0 = v0 + 0x1F4;
            if (log2_val < (u32)v0) {
                *(s16 *)(ent + 0x286) = 5;
                *t0 = 0;
            }
        }
    }
next:
    t1 += 1;
    a3 += 0x2C;
    t0 += 0x2C;
    if (t1 < 4) goto loop;
}
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
void func_800325E0(s32 arg0, s32 *arg1) {
    s32 sp_tmp;
    s32 *cam;
    s32 dx, dy, dz;
    s32 sqrt_val;
    s32 brightness;
    s32 dot;
    s32 sign;
    s32 a1_val, a2_val;
    s32 tmp;

    cam = (s32 *)D_800A36B4;
    dx = cam[8] - arg1[0];
    dy = cam[9] - arg1[1];
    dz = cam[10] - arg1[2];

    if ((u32)(dx + 0x9C40) > 0x13880u || (u32)(dz + 0x9C40) > 0x13880u) {
        sqrt_val = 0x9C40;
    } else {
        s32 dist_sq = dx * dx + dy * dy + dz * dz;
        if ((u32)dist_sq < 0x400u) {
            sqrt_val = (&D_8008D118)[dist_sq] >> 3;
        } else {
            s32 lzcr;
            s32 addr;
            register s32 t4_v asm("t4");
            __asm__ volatile(
                "addu\t$12,%1,$0\n\t"
                ".word 0x488CF000"
                : "=r"(t4_v) : "r"(dist_sq)
            );
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            addr = (s32)&sp_tmp;
            __asm__ volatile(
                "addu\t$12,%0,$0\n\t"
                ".word 0xE99F0000"
                : : "r"(addr) : "memory"
            );
            lzcr = sp_tmp;
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tval = (&D_8008D118)[(u32)dist_sq >> shift];
                s32 half = (u32)shift >> 1;
                sqrt_val = (u32)(tval << 16) >> (0x13 - half);
            }
        }
    }

    brightness = ((0x9C40 - sqrt_val) << 11) / 32000;
    if (brightness < 0) {
        brightness = 0;
    }

    {
        s16 angle = ((s16 *)(s32)D_800A36B4)[9];
        dot = (dx * (&Judge)[((angle + 0x400) & 0xFFF)] + dz * (&Judge)[(angle & 0xFFF)]) >> 12;
    }

    sign = (u32)~dot >> 31;
    if (dot < 0) {
        dot = -dot;
    }

    a1_val = ((0x7530 - dot) << 12) / 30000;
    a2_val = ((0x2710 - dot) << 12) / 10000;

    if (a1_val < 0) {
        a1_val = 0;
    }
    if (a2_val < 0) {
        a2_val = 0;
    }

    tmp = a1_val;
    if (sign != 0) {
        a1_val = a2_val;
        a2_val = tmp;
    }

    a1_val = (brightness * a1_val) >> 16;
    a2_val = (brightness * a2_val) >> 16;

    if (a1_val >= 0x80) {
        a1_val = 0x7F;
    }
    if (a2_val >= 0x80) {
        a2_val = 0x7F;
    }

    func_8005C650(arg0, a1_val, a2_val);
}
void func_80032854(s32 arg0, s32 arg1, u8 *arg2, s16 *arg3) {
    s32 s0;
    s32 s4;

    if (D_800A38DC == 3 && arg0 == 1 &&
        ((u32)(arg1 - 0x22) < 2 || (u32)(arg1 - 0x24) < 2 ||
         (u32)(arg1 - 0x26) < 2 || (u32)(arg1 - 0x28) < 2)) {
        s4 = 0;
        s0 = 0;
    } else {
        s0 = arg0 * 40;
        s4 = 0x28;
    }

    if (D_800A38DC == 3 && arg0 == 1 && (u32)(arg1 - 0x2D) < 2) {
        s0 += (&D_8008EBCC)[*(u8 *)&D_800A384C];
    }

    func_800395B4(arg0, arg1, arg2, arg3);

    switch (arg1) {
    case 4:
        if (D_800A38DC == 2 && D_800A389A == 0) {
            func_800617C8(arg2);
            break;
        }
        if (D_800A38DC == 5) {
            func_800617C8(arg2);
            break;
        }
        func_800618B4(arg2, arg3);
        break;
    case 3:
        func_80061250(arg2);
        break;
    case 2:
        func_80061658(arg2, arg0);
        break;
    case 15:
        func_80061710(arg2, arg0);
        break;
    case 1:
        func_8006156C(arg2);
        break;
    case 5:
        func_800325E0(0x3C, (s32 *)arg2);
        func_800325E0(s4 + 0x3C, (s32 *)arg2);
        D_800A3910 = func_8006133C(arg2);
        break;
    case 6:
        func_800325E0(0x3C, (s32 *)arg2);
        func_800325E0(s4 + 0x3C, (s32 *)arg2);
        D_800A3910 = func_800613C8(arg2);
        break;
    case 7:
        func_800325E0(0x3C, (s32 *)arg2);
        func_800325E0(s4 + 0x3C, (s32 *)arg2);
        D_800A3910 = func_80061454(arg2);
        break;
    case 8:
        func_800325E0(0x3C, (s32 *)arg2);
        func_800325E0(s4 + 0x3C, (s32 *)arg2);
        D_800A3910 = func_800614E0(arg2);
        break;
    case 9:
        func_800619A4(arg2);
        break;
    case 10:
        func_800619F0(arg2);
        break;
    case 11:
        func_800325E0(s0 + 0x31, (s32 *)arg2);
        func_800611A4(arg2, arg3);
        break;
    case 12:
        func_800325E0(0x78, (s32 *)arg2);
        func_80061C00(arg2, arg3[1]);
        break;
    case 13:
        func_800325E0(0x79, (s32 *)arg2);
        func_80061D74(arg2, arg3[1]);
        break;
    case 14:
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        func_80061ACC(arg2, arg3);
        break;
    case 17:
        func_800325E0(s0 + 0x39, (s32 *)arg2);
        break;
    case 18:
        func_80061EC0(arg2);
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        break;
    case 33:
        func_800325E0(0x7A, (s32 *)arg2);
        break;
    case 34:
        func_800325E0(s0 + 0x35, (s32 *)arg2);
        break;
    case 35:
        func_800325E0(s0 + 0x35, (s32 *)arg2);
        break;
    case 36:
        func_800325E0(s0 + 0x34, (s32 *)arg2);
        break;
    case 37:
        func_800325E0(s0 + 0x3A, (s32 *)arg2);
        break;
    case 38:
        func_800325E0(s0 + 0x3B, (s32 *)arg2);
        break;
    case 39:
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        break;
    case 40:
        func_800325E0(s0 + 0x3E, (s32 *)arg2);
        break;
    case 41:
        func_800325E0(s0 + 0x40, (s32 *)arg2);
        break;
    case 42:
        func_800325E0(s0 + 0x21, (s32 *)arg2);
        break;
    case 43:
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        break;
    case 44:
        func_800325E0(s0 + 0x23, (s32 *)arg2);
        break;
    case 45:
        func_800325E0(s0 + 0x29, (s32 *)arg2);
        break;
    case 46:
        func_800325E0(s0 + 0x2B, (s32 *)arg2);
        break;
    case 47:
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        break;
    case 48:
        func_800325E0(s0 + 0x22, (s32 *)arg2);
        break;
    case 49:
        func_800325E0(s0 + 0x32, (s32 *)arg2);
        break;
    case 50:
        func_800325E0(s0 + 0x33, (s32 *)arg2);
        break;
    }
}
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

void mk_leaf_newpos(void) {
    u8 *slot = &D_800A3918;
    u8 *pos = (u8 *)&D_80107850;
    u8 *tbl = &D_8008E914 + D_800A36A4 * 8;
    int new_var;

    do {
        if (*slot != 0) {
            s32 cat = func_80033498();
            u8 cur = *slot;
            s32 idx = cat * 2;
            s32 a0_val;

            if (cur == 1) {
                a0_val = (&D_8008EBF4)[cat];
                goto do_call;
            }
            if (cur == (&D_8008EBFC)[idx]) {
                a0_val = (&D_8008EBF4)[cat] + 1;
                goto do_call;
            }
            if (cur == (&D_8008EBFD)[idx]) {
                a0_val = (&D_8008EBF4)[cat] + 2;
            do_call:
                func_800325E0(a0_val, (s32 *)pos);
            }

            {
                s32 val = *slot + 1;
                *slot = val;
                idx = 0;
                new_var = cat * 2;
                if ((u32)(&D_8008EBFD)[new_var] < (u32)(val & 0xFF)) {
                    *slot = idx;
                }
            }
        }
        slot++;
        pos += 12;
    } while ((s32)slot < (s32)&D_800A391E);

    if (func_8001DB58() != 0) {
        u8 *data = tbl + 1;
        s32 counter = 0x7B;
        s16 *base = &D_800A3750;
        s16 *p = base;

        do {
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = *tbl;

            if (type == 1) goto handle_type_1;
            if (type < 2) goto skip;
            if (type < 7) goto handle_type_2_6;
            counter++;
            goto loop_bottom;

        handle_type_1:
            if (*p != 0) goto skip;
            *p = type;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 adj2 = adj * 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = *(u16 *)p + 1;
                *p = count;
                scount = (s16)count;
                thresh = (&D_8008EA44)[adj2];
                limit = thresh * 30;

                if (limit >= scount) goto skip;

                {
                    s32 diff = scount - limit;
                    s32 thresh2 = (&D_8008EA45)[adj2];
                    s32 limit2 = thresh2 * 30;
                    s32 ratio = (diff << 10) / limit2;

                    if (rnd >= ratio) goto skip;
                }

                *p = 0;
                type = *tbl;
                if ((u32)(type - 5) < 2) {
                    func_800325E0(counter, (s32 *)(&D_8008EA00 + type * 16));
                    counter++;
                    goto loop_bottom;
                }
                goto call_default;
            }

        call_default:
            {
                u8 val = *data;
                func_8005C650(counter, val, val);
            }
        skip:
            counter++;
        loop_bottom:
            p++;
            data += 2;
            tbl += 2;
        } while ((s32)p < (s32)(base + 4));
    }
}
void func_80033898(void) {
    gpu_EnableDisplay();
    D_800A37B8 = 0;
    D_800A3834 = 3;
}
void cpu_set_move_command_and_dir_for_no_action(void) {
    s32 sp[2];
    s32 count;
    s32 i;
    s32 idx1;
    u8 *ptr;
    s32 one;

    count = 0;
    i = 0;
    one = 1;
    __asm__ volatile("" : "=r"(one) : "0"(one));
    {
        s32 mask = ~(1 << (&D_8008D538)[(s8)D_8010277C]) & 0x3EF3DF;
        s32 bits = D_80106A50 & mask;
        ptr = &D_801077B0;
        do {
            if (bits & (one << i)) {
                *ptr = i;
                ptr++;
                count++;
            }
            i++;
        } while (i < 0x1B);
    }

    i = 0;
    do {
        i++;
        idx1 = func_80079154() % count;
        {
            s32 idx2 = func_80079154() % count;
            s32 tmp = (u8)(&D_801077B0)[idx1];
            (&D_801077B0)[idx1] = (u8)(&D_801077B0)[idx2];
            (&D_801077B0)[idx2] = tmp;
        }
    } while (i < 0x6C);

    if (D_80106A50 & 0x10020) {
        s32 v1 = D_80106A50 & 0x20;
        s32 v0 = D_80106A50 & 0x10000;
        sp[0] = v1;
        sp[1] = v0;
        if (v1 == 0) {
            sp[0] = v0;
            sp[1] = 0;
            goto block_12;
        }
        if (v0 != 0) {
            if (func_80079154() & 1) {
                s32 t1 = sp[1];
                s32 t2 = sp[0];
                sp[0] = t1;
                sp[1] = t2;
            }
        }
block_12:
        if (count >= 0xB) {
            i = count;
            do {
                (&D_801077B0)[i] = (&D_801077AF)[i];
                i--;
            } while (i >= 0xB);
            {
                s32 dir = 0x10;
                if (sp[0] & 0x20) {
                    dir = 5;
                }
                D_801077BA = dir;
            }
        } else {
            s32 dir = 0x10;
            if (sp[0] & 0x20) {
                dir = 5;
            }
            (&D_801077B0)[count] = dir;
        }
        count++;
        if (sp[1] != 0) {
            u8 *a1 = &(&D_801077B0)[count];
            count++;
            {
                s32 dir = 0x10;
                if (sp[1] & 0x20) {
                    dir = 5;
                }
                *a1 = dir;
            }
        }
    }
    {
        u8 lookup = (&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]];
        s32 val;
        if (lookup != 0) {
            val = 0x1A;
            if (D_80106A50 & 0x04000000) {
                goto append_last;
            }
        } else {
            val = 0x18;
            if (D_80106A50 & 0x01000000) {
append_last:
                (&D_801077B0)[count] = val;
                count++;
            }
        }
    }
    D_800A391F = count;
    D_800A3783 = 0;
    D_800A37BC = 0;
}
/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action  |  189i  |  x2 size collision */
void mottest_disp(void) {
    u8 a0 = D_800A3783;
    u8 b = D_800A391F;

    if ((a0 & 0xFF) == b) {
        D_800A3768 = 0xFF;
        D_800A36A8 = 0;
        if ((a0 & 0xFF) == 0x14) {
            u8 z = (&D_8008D9EC)[D_80101ED2];
            s32 val = 2;
            if (z != 0) val = 3;
            D_800A38A4 = val;
            D_800A3834 = 0x12;
        } else {
            D_800A3834 = 0x20;
        }
    } else {
        u8 a1;
        u8 idx;
        D_800A376B = 0;
        D_800A3783 = a0 + 1;
        a1 = (&D_801077B0)[a0 & 0xFF];
        idx = a1 & 0xFF;
        D_8010277D = (&D_8008D55C)[idx];
        if (idx == 5) {
            D_8010277F = 6;
        } else if (idx == 0x10) {
            D_8010277F = 7;
        } else {
            u8 x = D_800A37BC;
            s8 y;
            D_800A37BC = x + 1;
            y = (s8)(&D_8008E748)[x & 0xFF];
            D_8010277F = y;
            if (y == 4) {
                u8 v = (&D_8008D9EC)[idx];
                if (v != 0) {
                    D_8010277F = 5;
                }
            }
        }
        D_8010277A = 0x800;
        {
            s16 v = (&D_8008E75C)[(u8)a1];
            D_800A3834 = 0;
            D_800A36A4 = v;
        }
    }
}
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
s32 func_80033DF4(void) {
    register u8 state asm("v1");
    register s32 tableIndex asm("a1");

    state = D_800A38E2;
    tableIndex = state & 0xFF;
    if (tableIndex == 0x64) {
        D_800A36F0 = 0;
        D_800A3781 = 0;

        if (D_800A3858 < 0x6979) {
            register s32 v0 asm("v0");
            register s32 v1 asm("v1");
            register s32 *a0 asm("a0");

            v0 = (s8)D_8010277C;
            v0 = *(&D_8008D538 + v0);
            v0 = *(&D_8008D9EC + v0);
            tableIndex = 0x20;
            if (v0 != 0) {
                tableIndex = 0x10000;
            }
            a0 = &D_80106A50;
            v1 = *a0;
            D_800A36F0 = (u32)(v1 & tableIndex) < 1;
            *a0 = v1 | tableIndex;
        }

        if (D_800A380C == 0) {
            register s32 v0 asm("v0");
            register s32 v1 asm("v1");
            register s32 *a0 asm("a0");

            v0 = (s8)D_8010277C;
            v0 = *(&D_8008D538 + v0);
            v0 = *(&D_8008D9EC + v0);
            tableIndex = 0x1000000;
            if (v0 != 0) {
                tableIndex = 0x4000000;
            }
            a0 = &D_80106A50;
            v1 = *a0;
            D_800A3781 = (u32)(v1 & tableIndex) < 1;
            *a0 = v1 | tableIndex;
        }

        motutil_CheckKamaeReq();
        D_800A3834 = 4;
        return 0;
    } else {
        register s32 v0 asm("v0");
        register u8 *v1 asm("v1");
        register s32 a0 asm("a0");
        register s32 a2 asm("a2");
        register u8 *table asm("a1");

        D_800A38E2 = state + 1;
        table = &cpu_practice_honmokuroku_data_tbl + (tableIndex * 4);
        D_800A376B = 0;
        a2 = table[0];
        v0 = (s8)D_8010277C;
        *((u8 *)&D_800A384C) = a2;
        v0 = *(&D_8008D538 + v0);
        v1 = &D_8008EC24;
        v0 = *(&D_8008D9EC + v0);
        a2 &= 0xFF;
        v0 = (u32)v0 < 1;
        a0 = v0 << 2;
        a0 += v0;
        D_800A38DE = *(v1 + a0 + a2);
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        a0 = a0 + (s32)&D_8008E908;
        D_8010277D = *(u8 *)(a0 + a2);
        return 1;
    }
}
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


extern void func_8003B20C(s32);
extern void suDispMentalBar(void);
extern s32 func_8005509C(s32);
INCLUDE_RODATA("asm/rodata", jtbl_8001084C);

void DispSamnailWindow(void) {
    s32 s0;

    func_800343F0();

    switch (D_800A38DC) {
    case 6:
        D_80102786 = 1;
        D_800A3768 = 1;
        D_800A3834 = 0;
        D_800A36F6 = (D_800A38A0 != 0);
        goto skip_clear;

    case 0:
        func_8003B20C((&D_8008D538)[(s8)D_8010277C]);
        D_8010277D = 0;
        suDispMentalBar();
        func_8005509C(1);
        goto skip_clear;

    case 1:
        s0 = 1;
        D_80102781 = (u8)s0;
        cpu_set_move_command_and_dir_for_no_action();
        D_800A3768 = (u8)s0;
        mottest_disp();
        goto skip_clear;

    case 3:
        s0 = 1;
        D_80102781 = (u8)s0;
        D_8010277F = 0;
        D_8010277E = 0;
        D_800A38E2 = 0;
        D_800A38E0 = 0;
        D_800A3858 = 0;
        D_800A3728 = 0;
        D_800A36A4 = 0x22;
        func_80033DF4();
        D_800A3768 = (u8)s0;
        break;

    case 5:
        {
            s32 v1 = (D_800A38E1 & 1) ? 0x21 : 0x20;
            D_800A36A4 = (s16)v1;
        }
        D_800A3874 = 0;
        gpu_EnableDisplay();
        file_LoadOverlay();
        func_800342A0();
        goto skip_clear;

    case 2:
        {
            s32 v1 = D_800A389A;
            s32 cmp = (u32)v1 < 1u;
            D_800A3713 = (u8)(cmp << 1);
            {
                s32 da = (v1 != 0) ? 0x24 : 0x23;
                D_800A36A4 = da;
            }
            D_80102781 = 1;
            if (v1 != 0) {
                break;
            }
        }
        {
            u8 idx = (&D_8008D538)[(s8)D_8010277C];
            u8 val = (&D_8008D9EC)[idx];
            s32 tmp = (val != 0) ? 0x0E : 0x1D;
            D_8010277D = tmp;
        }
        break;

    case 4:
        D_800A3768 = 1;
        break;
    }

    D_800A3834 = 0;

skip_clear:
    if (D_80102781 != 0) {
        func_8005509C(1);
    }
}
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
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *src;
    u8 *ptr;
    s8 *base;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    src = (u8 *)p;
    base = &D_80102785;
    ptr = (u8 *)(base - 9);

    do {
        s32 lv = (&D_8008D55C)[*src];
        *ptr = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (*base == 0) {
                *ptr = *ptr - 3;
            }
        }
        tmp = src[1];
        src += 10;
        (&D_80102780)[i] = 0;
        (&D_8010277E)[i] = tmp;
        i++;
        ptr++;
    } while (i < 2);

    D_80102784 = ((u32)p[5] >> 4) & 0x3F;
    D_800A36F6 = 0;
    D_80102786 = ((u32)p[8] >> 3) & 1;

    val = D_80102785;
    if (val == 2) {
        D_800A389A = ((u32)p[5] >> 17) & 1;
        D_800A3788 = ((u32)p[5] >> 18) & 7;
    } else if (val == 5) {
        u32 idx;
        s32 sel;

        D_800A389B = (((u32)p[5] >> 10) & 3) + 3;
        idx = ((u32)p[5] >> 12) & 3;
        D_800A36CC = (&D_8008EC30)[idx];
        sel = 1;
        if ((u32)p[5] & 0x4000) {
            sel = 2;
        }
        D_800A37F8 = sel;
        D_800A38E1 = ((u32)p[5] >> 15) & 3;
        {
            u8 *dst_d = &D_801027D8;
            u8 *dst_a = &D_801027A0;
            s32 j = 0;
            do {
                s32 k = 0;
                u8 *da = dst_d;
                u8 *db = dst_a;
                s32 off = j << 1;
            loop_inner:
                {
                    u8 *pp = (u8 *)p + off;
                    *db = (&D_8008D55C)[pp[0]];
                    off += 10;
                    k++;
                    *da = pp[1];
                    db++;
                    da++;
                }
                if (k < 2) goto loop_inner;
                dst_d += 2;
                j++;
                dst_a += 2;
            } while (j < 5);
        }
    }

    DispSamnailWindow();
}

void single_game_SetWazaData(void) {
    s32 *p;
    u8 *src1;
    u8 *src2;
    int new_var;
    u8 *dst;
    s32 i;
    s32 j;
    s32 v;
    volatile u8 *pbit;

    p = func_80077D00();
    pbit = &D_80106A73;
    v = p[8];
    v = v & (~1);
    v = v | ((*pbit) & 1);
    p[8] = v;
    v = v & (~2);
    v = v | ((*pbit) & 2);
    p[8] = v;
    v = v & (~4);
    v = v | ((*pbit) & 4);
    p[8] = v;

    src1 = (&D_80106A73) - 3;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = src1[i];
        ((u8 *)p)[0x1D + i] = src1[i];
    }

    src2 = (u8 *)(&D_80106A58);
    dst = (u8 *)p;
    for (j = 0; j < 3; j++) {
        dst[0x21] = (*((s32 *)(src2 + 4))) / 1800;
        new_var = ((*((s32 *)(src2 + 4))) / 30) % 60;
        v = new_var;
        dst[0x22] = v;
        dst[0x23] = (((*((s32 *)(src2 + 4))) % 30) * 100) / 30;
        dst[0x24] = src2[0];
        dst += 4;
        src2++;
        src2--;
        src2 += 8;
        j++;
        j--;
    }
}
void func_80035430(void) {
}
