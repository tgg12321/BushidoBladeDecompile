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
extern u16 D_80101F32;
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
extern s32 EndADRSound(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);

extern void motion_LoadPreCalcData_8005B98C(s32);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern u8 D_8008E908;
extern u8 D_8008EC24;
extern s32 D_80106A50;
extern s32 single_game_getEnemyCharId(s32, s32);
extern s32 func_80079154(void);
extern void *func_8007F87C(s32, s32 *);
extern void *func_8007FA1C(s32, s32 *);
extern void *func_8007FBBC(s32, s32 *);
extern s16 D_80101E74;
extern void motutil_CheckKamaeReq(void);

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern s32 stage_GetDataPtr(void);

extern void get_point_value(void);
extern void func_80037774(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 gnd_close_8004939C(void);
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
extern u8 D_80106A78;
extern s32 *saTan2GaugeInit_80077D00(void);
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
extern void motion_GameCalcMotion(void);
extern void saTan3MainJump(void);
extern s32 func_80029454(void);
extern void func_80031B24(void);
extern s32 D_801020D8;
extern s32 D_801020DC;
extern s32 D_801020E0;
extern s32 D_801020E4;
extern s32 D_801020E8;
extern s32 D_801020EC;
extern s32 D_801020FC;
extern s32 D_80102100;
extern s32 D_80102104;
extern s32 D_80102108;
extern s32 D_8010210C;
extern s32 D_80102110;
extern s32 D_80102314;
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_RODATA("asm/rodata", jtbl_8001042C);
void func_80026DA4(void) {
}
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
void katinuki_game_getMyWeaponId(s16 *arg0) {
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
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
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
s32 calc_teasi_loc_fw() {
    return 0;
}
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
void saTan3MainJump(void) {
}
/* kengo:HIGH  |  sa_tan3/saTan3MainJump  |  492i  |  +3 near-exact */
void func_8002906C(void) {
    s16 *ptr = snd_GetSeId();
    while (*(s16 *)ptr != 0) {
        *(s16 *)((u8 *)ptr + 2) = 0;
        ptr = (s16 *)((u8 *)ptr + 0x10);
    }
}
s32 DispPracticeMenuTex_C() {
    return 0;
}
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
s32 func_80029454(void) {
    return 0;
}
s32 func_8002A458() {
    return 0;
}
void calc_loc_mat_fw(s32 a0) {
    (void)a0;
}
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
        func_8001F860((s16 *)var_s0, single_game_getEnemyCharId(arg1, arg2));
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
void PutRobShadow(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    *(volatile s32 *)0x1F800360 = 0;
    *(volatile s32 *)0x1F800364 = 0;
    *(volatile s32 *)0x1F800368 = 0;
    *(volatile s32 *)0x1F800370 = 0;
    *(volatile s32 *)0x1F800374 = 0;
    *(volatile s32 *)0x1F800378 = 0;

    if (D_800A3824 & 1) {
        v1 = *(volatile s32 *)0x1F80004C;
        v0 = *(volatile s32 *)0x1F800048;
        a2 = *(volatile s32 *)0x1F800050;
        *(volatile s32 *)0x1F800364 = v1;
        v1 = v1 + *(volatile s32 *)0x1F800058;
        *(volatile s32 *)0x1F800360 = v0;
        v0 = v0 + *(volatile s32 *)0x1F800054;
        *(volatile s32 *)0x1F800360 = v0;
        *(volatile s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(volatile s32 *)0x1F800368 = a2;
        a2 = a2 + *(volatile s32 *)0x1F80005C;
        *(volatile s32 *)0x1F800370 = D_801020FC;
        *(volatile s32 *)0x1F800370 = D_801020FC + D_80102108;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        v1 = *(volatile s32 *)0x1F800004;
        v0 = *(volatile s32 *)0x1F800000;
        a2 = *(volatile s32 *)0x1F800008;
        *(volatile s32 *)0x1F800364 = v1;
        v1 = v1 + *(volatile s32 *)0x1F800010;
        *(volatile s32 *)0x1F800360 = v0;
        v0 = v0 + *(volatile s32 *)0x1F80000C;
        *(volatile s32 *)0x1F800360 = v0;
        *(volatile s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(volatile s32 *)0x1F800368 = a2;
        a2 = a2 + *(volatile s32 *)0x1F800014;
        *(volatile s32 *)0x1F800370 = D_801020D8;
        *(volatile s32 *)0x1F800370 = D_801020D8 + D_801020E4;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    *(volatile s32 *)0x1F800368 = a2;
    *(volatile s32 *)0x1F800374 = d_v1;
    *(volatile s32 *)0x1F800378 = d_a0;
    *(volatile s32 *)0x1F800374 = d_v1 + d_v0;
    *(volatile s32 *)0x1F800378 = d_a0 + d_a1;

    if (D_800A3824 & 2) {
        t0[0xA8/4] += *(volatile s32 *)0x1F800060;
        t0[0xAC/4] += *(volatile s32 *)0x1F800064;
        t0[0xB0/4] += *(volatile s32 *)0x1F800068;
        t0[0xB8/4] += d_tbl[0x234/4];
        t0[0xBC/4] += d_tbl[0x238/4];
        t0[0xC0/4] += d_tbl[0x23C/4];
        t0[0xA8/4] += *(volatile s32 *)0x1F80006C;
        t0[0xAC/4] += *(volatile s32 *)0x1F800070;
        t0[0xB0/4] += *(volatile s32 *)0x1F800074;
        t0[0xB8/4] += d_tbl[0x240/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x244/4];
        sum_a1_2 = d_tbl[0x248/4];
    } else {
        t0[0xA8/4] += *(volatile s32 *)0x1F800024;
        t0[0xAC/4] += *(volatile s32 *)0x1F800028;
        t0[0xB0/4] += *(volatile s32 *)0x1F80002C;
        t0[0xB8/4] += d_tbl[0x210/4];
        t0[0xBC/4] += d_tbl[0x214/4];
        t0[0xC0/4] += d_tbl[0x218/4];
        t0[0xA8/4] += *(volatile s32 *)0x1F800030;
        t0[0xAC/4] += *(volatile s32 *)0x1F800034;
        t0[0xB0/4] += *(volatile s32 *)0x1F800038;
        t0[0xB8/4] += d_tbl[0x21C/4];
        sum_v0_2 = t0[0xBC/4] + d_tbl[0x220/4];
        sum_a1_2 = d_tbl[0x224/4];
    }
    t0[0xBC/4] = sum_v0_2;
    t0[0xC0/4] += sum_a1_2;
    t0[0x13C/4] = ((t0[0xA8/4] * 3) + t0[0xB8/4]) >> 4;
    t0[0x140/4] = ((t0[0xAC/4] * 3) + t0[0xBC/4]) >> 4;
    t0[0x144/4] = ((t0[0xB0/4] * 3) + t0[0xC0/4]) >> 4;
}
/* kengo:MED  |  am_rmd/PutRobShadow  |  252i */
typedef struct { s32 x, y, z; } Vec3i;

void func_8002C61C(void) {
    u8 *s1 = (u8 *)&D_80101EC8;
    u8 *s0 = s1 + 0x44C;
    u16 mode;
    s32 t1;

    mode = D_80101F32;

    if (mode == 0xF || (u32)(mode - 0x1C) < 2 ||
        (u32)(mode - 0x1E) < 2 || (u32)(mode - 0x20) < 2) {
        func_80026DA4();
    } else if (mode == 0x11) {
        func_8002C0DC();
    } else {
        motion_GameCalcMotion();
        saTan3MainJump();
        D_800A3824 = func_80029454();
        if (D_800A3824 < 0) goto do_calc;
        PutRobShadow();
        if (D_800A3824 < 0) goto do_calc;
        if (D_80101F75 != 0 || D_801023C1 != 0) {
            saTan2KabutoWareMove(s1, (u8 *)0x1F8003F4);
            saTan2KabutoWareMove(s0, (u8 *)0x1F8003F4);
            D_801023C1 = 0;
            D_80101F75 = 0;
            goto after_calc;
        }
    do_calc:
        calc_loc_mat_fw(0);
    after_calc:

        if (*(s32 *)(s1 + 0x3C) >= 3 && *(s32 *)(s0 + 0x3C) >= 3 &&
            D_800A38A8 != 0 && *(s16 *)(s1 + 0x286) == -1 &&
            *(s16 *)(s0 + 0x286) == -1 && *(s16 *)(s1 + 0xC) != 0x1F &&
            *(s16 *)(s0 + 0xC) != 0x1F) {
            s32 diff = *(s32 *)(s1 + 0xF8) - *(s32 *)(s0 + 0xF8);
            if (diff < 0) diff = -diff;
            if (diff < 0x3E8) {
                *(s16 *)(s1 + 0x286) = 0xA;
                *(s16 *)(s0 + 0x286) = 0xA;
                *(s32 *)(s0 + 0x28C) = 0;
                *(s32 *)(s1 + 0x28C) = 0;
                D_800A3910 = 0;
                D_800A389C = 0;
            }
        }
    }

    if (D_80101F32 == 5) {
        D_800A3748 = 1;
        D_800A3834 = 0x1C;
    } else if (D_8010237E == 5) {
        D_800A3748 = 0;
        D_800A3834 = 0x1C;
    }

    {
        Vec3i *dst_a = (Vec3i *)&D_801020D8;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);
        Vec3i *sp_src = (Vec3i *)0x1F800000;
        t1 = 0;
        do {
            *dst_a = sp_src[0];
            *dst_b = sp_src[3];
            dst_b++;
            dst_a++;
            sp_src++;
            t1++;
        } while (t1 < 3);
    }

    {
        Vec3i *dst_a = (Vec3i *)&D_801020FC;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020FC + 0x44C);
        Vec3i *sp_src = (Vec3i *)0x1F800000;
        t1 = 0;
        do {
            *dst_a = sp_src[6];
            *dst_b = sp_src[8];
            dst_b++;
            dst_a++;
            sp_src++;
            t1++;
        } while (t1 < 2);
    }

    {
        s32 *a1 = (s32 *)0x1F8000EC;
        s32 a2_off = 0;
        t1 = 0;
        do {
            *(s32 *)((u8 *)&D_80102054 + a2_off) = (a1[-14] + a1[-11] + a1[-8]) / 3;
            *(s32 *)((u8 *)&D_80102058 + a2_off) = (a1[-13] + a1[-10] + a1[-7]) / 3;
            *(s32 *)((u8 *)&D_8010205C + a2_off) = (a1[-12] + a1[-9] + a1[-6]) / 3;
            *(s32 *)((u8 *)&D_8010203C + a2_off) = (a1[-5] + a1[-2]) / 2;
            t1++;
            *(s32 *)((u8 *)&D_80102040 + a2_off) = (a1[-4] + a1[-1]) / 2;
            *(s32 *)((u8 *)&D_80102044 + a2_off) = (a1[-3] + a1[0]) / 2;
            a1 += 66;
            a2_off += 0x44C;
        } while (t1 < 2);
    }

    {
        s16 saved = *(s16 *)(s1 + 0x286);
        if (saved == -1) {
            func_80031B24();
            if (*(s16 *)(s1 + 0x286) == saved) {
                Pad_Prs();
            }
        }
    }
}
void func_8002CA8C(u8 *a0, s32 a1, s32 a2) {
    (void)a0;
    (void)a1;
    (void)a2;
}
s32 special_camera_Init() {
    return 0;
}
/* kengo:HIGH  |  nm_special_cam/special_camera_Init  |  370i */
s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        __asm__ volatile("" ::: "memory");
        {
            register s32 t4 asm("t4");
            s32 ptr_tmp;
            ptr_tmp = (s32)(obj + 0xF8);
            __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
            __asm__ volatile("lwc2 $0, 0($12)" : : "r"(t4));
            __asm__ volatile("lwc2 $1, 4($12)" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            __asm__ volatile(".word 0x4A486012");
            ptr_tmp = (s32)(obj + 0x100);
            __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
            __asm__ volatile("swc2 $25, 0($12)" : : "r"(t4));
            __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
            __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
        }
    }

    {
        s32 x = *(s32 *)(obj + 0x100);
        if (x < -threshold || threshold < x) return 0;
    }
    {
        s32 z = *(s32 *)(obj + 0x104);
        if (z < -threshold || threshold < z) return 0;
    }

    {
        s32 x = *(s32 *)(obj + 0x100);
        s32 z = *(s32 *)(obj + 0x104);
        register s32 dist_sq asm("a0") = x * x + z * z;
        s32 disc;
        s32 sqrt_val;
        s32 sp_var;
        register s32 min_y asm("a2");
        register s32 max_y asm("a3");
        s32 y_low;
        s32 y_high;
        s32 y;

        if (r_sq < dist_sq) return 0;
        disc = r_sq - dist_sq;

        if ((u32)disc < 0x400) {
            sqrt_val = ((u32)*(&D_8008D118 + disc)) >> 3;
        } else {
            s32 lzcr = 0;
            if (disc >= 0) {
                register s32 t4 asm("t4") = disc;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(&D_8008D118 + ((u32)disc >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        {
            s32 result = 0;
            max_y = 0;
            min_y = 0;
            y_low = *(s32 *)(obj + 0xB0);
            if (y_low < 0) {
                min_y = y_low;
            } else if (y_low > 0) {
                max_y = y_low;
            }
            y_high = *(s32 *)(obj + 0xC0);
            if (y_high < min_y) {
                min_y = y_high;
            } else if (max_y < y_high) {
                max_y = y_high;
            }
            y = *(s32 *)(obj + 0x108);
            if (max_y >= y - sqrt_val) {
                if (y + sqrt_val >= min_y) {
                    result = 1;
                }
            }
            return result;
        }
    }
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
s32 saTan0KiWareMoveA(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        {
            register s32 t4 asm("t4");
            t4 = (s32)(obj + 0xF8);
            __asm__ volatile("lwc2 $0, 0($12)" : : "r"(t4));
            __asm__ volatile("lwc2 $1, 4($12)" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            __asm__ volatile(".word 0x4A486012");
            t4 = (s32)(obj + 0x100);
            __asm__ volatile("swc2 $25, 0($12)" : : "r"(t4));
            __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
            __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
        }
    }

    {
        s32 z_val = *(s32 *)(obj + 0x108);
        if (z_val < -threshold || threshold < z_val) {
            return 0;
        }
    }

    {
        s32 x0 = *(s32 *)(obj + 0xA8);
        s32 x2 = *(s32 *)(obj + 0xB8);
        s32 z0 = *(s32 *)(obj + 0xAC);
        s32 z2 = *(s32 *)(obj + 0xBC);
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = *(s32 *)(obj + 0x100);
        s32 pz = *(s32 *)(obj + 0x104);

        {
            s32 cross_c = z0 * cx - x0 * cz;
            s32 cross_p = z0 * px - x0 * pz;
            if ((cross_c ^ cross_p) < 0) goto edge_check;
        }

        {
            s32 cross_c = z2 * cx - x2 * cz;
            s32 cross_p = z2 * px - x2 * pz;
            if ((cross_c ^ cross_p) < 0) goto edge_check;
        }

        {
            s32 dz = z2 - z0;
            s32 dx = x2 - x0;
            s32 cross_c = dz * (cx - x0) - dx * (cz - z0);
            s32 cross_p = dz * (px - x0) - dx * (pz - z0);
            if ((cross_c ^ cross_p) >= 0) return 1;
        }
    }

edge_check:
    {
        s32 z_val = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - z_val * z_val;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;

        if ((u32)dist < 0x400) {
            sqrt_val = ((u32)*(&D_8008D118 + dist)) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist >= 0) {
                register s32 t4 asm("t4") = dist;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(&D_8008D118 + ((u32)dist >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);

        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);

        if (saTan5TakeAnim2(sqrt_val, dist, p118, p124) != 0) return 1;

        {
            s32 *p10C = (s32 *)(obj + 0x10C);
            p10C[0] = -*(s32 *)(obj + 0x100);
            p10C[1] = -*(s32 *)(obj + 0x104);

            if (saTan5TakeAnim2(sqrt_val, dist, p10C, p118) != 0) return 1;

            return saTan5TakeAnim2(sqrt_val, dist, p10C, p124) != 0;
        }
    }
}
/* kengo:MED  |  sa_tan0/saTan0KiWareMoveA  |  212i  |  x2 size collision */
s32 saTan0KiWareMoveB(u8 *obj) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saTan0KiWareMoveB.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)obj;
    return 0;
}

/* kengo:MED  |  sa_tan0/saTan0KiWareMoveB  |  212i  |  x2 size collision */
s32 func_8002DE20() {
    return 0;
}
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
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saSeInit.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
}
/* kengo:HIGH  |  sa_tan2/saTan2LinePrimInit  |  110i */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    __asm__ volatile("" ::: "memory");
    {
        register s32 t4 asm("t4");
        register s32 ptr_tmp asm("v0");
        ptr_tmp = (s32)(obj + 0xF8);
        __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
        __asm__ volatile("lwc2 $0, 0($12)" : : "r"(t4));
        __asm__ volatile("lwc2 $1, 4($12)" : : "r"(t4));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4A486012");
        ptr_tmp = (s32)(obj + 0x100);
        __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
        __asm__ volatile("swc2 $25, 0($12)" : : "r"(t4));
        __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
        __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
    }

    {
        register s32 x asm("a1") = *(s32 *)(obj + 0x100);
        register s32 neg_threshold asm("t1") = -threshold;
        if (x < neg_threshold || threshold < x) return 0;
    }
    {
        s32 z = *(s32 *)(obj + 0x104);
        register s32 neg_threshold asm("t1") = -threshold;
        if (z < neg_threshold || threshold < z) return 0;
    }

    {
        register s32 x asm("a1") = *(s32 *)(obj + 0x100);
        s32 z = *(s32 *)(obj + 0x104);
        register s32 a0_var asm("a0");
        s32 sp_var;
        register s32 min_y asm("a2");
        register s32 max_y asm("a1");
        s32 y_low;
        s32 y;

        a0_var = x * x + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            register s32 tbl_val asm("v0") = *(&D_8008D118 + a0_var);
            a0_var = (u32)tbl_val >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                register s32 t4 asm("t4") = a0_var;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                register s32 shifted asm("v0") = (u32)a0_var >> shift;
                register s32 tbl asm("a0") = *(&D_8008D118 + shifted);
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        if (y + a0_var < min_y) return 0;
        return 1;
    }
}
void DispSchoolBG(s16 *arg0, s16 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/DispSchoolBG.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
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
void func_8002F2D0(s32 *a0, s32 *a1) {
    (void)a0;
    (void)a1;
}
s32 func_8002F770() {
    return 0;
}
s32 cpu_check_tubazeri(s32 *a0, s32 *a1, s32 *a2) {
    s32 *va;
    s32 *vb;
    s32 *out;
    s32 ret;
    /* Diff (a1 - a0) into scratchpad SCR[0x60..0x68].
     * Inline-asm sw with absolute addr is the canonical scratchpad-write
     * idiom in this codebase (see code6cac.c:417); the natural-C
     * `*(volatile s32 *)0x1F800360 = expr` would compile to lui+ori+sw 0(reg)
     * which doesn't match target's lui+sw offset(at) pattern. */
    {
        volatile s32 *va0 = (volatile s32 *)a0;
        volatile s32 *va1 = (volatile s32 *)a1;
        volatile s32 *va2 = (volatile s32 *)a2;
        register s32 v0 asm("$2");
        register s32 v1 asm("$3");
        v0 = va1[0]; v1 = va0[0]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));
        v0 = va1[1]; v1 = va0[1]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800364" :: "r"(v0));
        v0 = va1[2]; v1 = va0[2]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800368" :: "r"(v0));
        v0 = va2[0]; v1 = va0[0]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800370" :: "r"(v0));
        v0 = va2[1]; v1 = va0[1]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800374" :: "r"(v0));
        v0 = va2[2]; v1 = va0[2]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800378" :: "r"(v0));
    }
    /* Load diff_a from scratchpad into GTE coef regs $0, $2, $4.
     * Pin source pointer to $a3 ($7) so lui+ori materialization uses $a3,
     * matching target's `lui $a3; ori $a3; addu $t4, $a3, $zero; ...`. */
    va = (s32 *)0x1F800360;
    {
        register s32 *q asm("$7") = va;
        register s32 *mp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        register s32 t7 asm("$15");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        t5 = *mp;
        t6 = *(mp + 1);
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        t7 = *(mp + 2);
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    /* Load diff_b from scratchpad into GTE IR regs and run OP cross product.
     * Pin source ptr to $a3 + move to $t4 to match target's lui+ori+addu pattern. */
    vb = (s32 *)0x1F800370;
    {
        register s32 *q asm("$7") = vb;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("lwc2 $11, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $9,  0(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $10, 4(%0)" :: "r"(mp));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4B70000C");  /* op 0 (cross product) */
    }
    /* Store MAC1/MAC2/MAC3 to scratchpad SCR[0x80..0x88], then call enemy-id
     * resolver with cross[0] and the scratchpad probe at 0x1F800388.
     * Out ptr pinned to $v0 so it survives until the lw $a0, 0($v0) call-arg
     * load (target reuses $v0 as both swc2 base and call-arg base). */
    {
        register s32 *q asm("$2") = (s32 *)0x1F800380;
        register s32 *mp asm("$12");
        s32 arg1_val;
        register s32 flag asm("$3");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lw %0, 0x1F800388" : "=r"(arg1_val));
        ret = single_game_getEnemyCharId(q[0], arg1_val);
        __asm__ volatile ("lw %0, 0x1F800384" : "=r"(flag));
        if (flag > 0) {
            ret += 0x800;
        }
    }
    (void)out;
    return ret;
}
/* kengo:HIGH  |  nm_cpu/cpu_check_tubazeri  |  76i  |  x2 size collision */
s32 coli_check_circle_hit_line(s32 *arg0) {
    s32 stride;
    s32 v1, v2;
    s32 ret;

    stride = (s32)((s16 *)arg0)[2] * 264;

    /* Compute (point_a - center) into scratchpad SCR[0x60..0x68] and
     * (point_b - center) into SCR[0x70..0x78].  Source vectors live at
     * stride-offset slots in scratchpad (0xB4/0xB8/0xBC = center xyz;
     * 0xC0/0xC4/0xC8 = a xyz; 0xCC/0xD0/0xD4 = b xyz).
     *
     * Natural-C `*(s32 *)(0x1F8000XX + stride)` produces target's
     * lui+addu+lw 3-insn sequence; absolute-addr stores match
     * the lui+sw idiom from cpu_check_tubazeri. */
    v1 = *(s32 *)((u8 *)0x1F8000C0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800360 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C4 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800364 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C8 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000BC + stride);
    *(s32 *)0x1F800368 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000CC + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800370 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000D0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800374 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000D4 + stride);
    {
        /* Pin v2 to $v0 ($2) — in the 6th block the diff result stays in
         * $v1 and stride goes in a different reg, so GCC normally uses $a0
         * for v2.  Target uses $v0 here. */
        register s32 v2_r asm("$2");
        v2_r = *(s32 *)((u8 *)0x1F8000BC + stride);
        /* Clobber $5 here so GCC defers q's lui+ori materialization until
         * AFTER the 6th-block loads — both insns then land in the 2nd lw's
         * load-delay slot (matching target's `lw $v0; lui $a1; ori $a1; subu`). */
        __asm__ volatile ("" ::: "$5");
        {
            register s32 *q asm("$5") = (s32 *)0x1F800360;
            register s32 *mp asm("$12");
            register s32 t5 asm("$13");
            register s32 t6 asm("$14");
            register s32 t7 asm("$15");
            v1 -= v2_r;
            *(s32 *)0x1F800378 = v1;
            __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
            t5 = *mp;
            t6 = *(mp + 1);
            __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
            t7 = *(mp + 2);
            __asm__ volatile ("ctc2 %0, $2" :: "r"(t6));
            __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
        }
    }
    /* Load diff_b (SCR[0x70..0x78]) into GTE IR regs and run OP cross product. */
    {
        register s32 *q asm("$5") = (s32 *)0x1F800370;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("lwc2 $11, 8(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $9,  0(%0)" :: "r"(mp));
        __asm__ volatile ("lwc2 $10, 4(%0)" :: "r"(mp));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4B70000C");  /* op 0 (cross product) */
    }
    /* Store MAC1/MAC2/MAC3 to scratchpad SCR[0x80..0x88]. */
    {
        register s32 *q asm("$5") = (s32 *)0x1F800380;
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(mp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(mp));
    }
    /* Read MAC2 from scratchpad and return slt(0, MAC2) — i.e. MAC2 > 0. */
    ret = *(s32 *)0x1F800384;
    return 0 < ret;
}


/* kengo:HIGH  |  is_coli/coli_check_circle_hit_line  |  92i */
void func_8002FF20(u8 *arg0, u8 arg1) {
    s32 mat_local[8];
    s32 *playerData;
    s32 *s2_ptr;
    s32 *rot_mat;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    arg0[8] = 1;
    arg0[9] = arg1;
    playerData = (s32 *)game_GetPlayerData(arg0[6] < 1);
    rot_mat = (s32 *)((u8 *)arg0 + 0xC);
    s2_ptr = (s32 *)playerData[arg0[9]];

    /* 3x3 identity matrix at arg0+0xC..arg0+0x1D (9 s16 entries).
     * The final entry (arg0+0x1C) lands in the jal func_8007F87C delay slot. */
    *(s16 *)((u8 *)arg0 + 0xC) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0xE) = 0;
    *(s16 *)((u8 *)arg0 + 0x10) = 0;
    *(s16 *)((u8 *)arg0 + 0x12) = 0;
    *(s16 *)((u8 *)arg0 + 0x14) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0x16) = 0;
    *(s16 *)((u8 *)arg0 + 0x18) = 0;
    *(s16 *)((u8 *)arg0 + 0x1A) = 0;
    *(s16 *)((u8 *)arg0 + 0x1C) = 0x1000;
    func_8007F87C(*(s16 *)((u8 *)arg0 + 0x54), rot_mat);
    func_8007FA1C(*(s16 *)((u8 *)arg0 + 0x56), rot_mat);
    func_8007FBBC(*(s16 *)((u8 *)arg0 + 0x58), rot_mat);
    func_8002EECC(s2_ptr, mat_local);
    func_8007E4DC(mat_local, rot_mat, rot_mat);

    /* Subtract opponent reference position from self position. */
    *(s32 *)((u8 *)arg0 + 0x2C) -= s2_ptr[5];
    *(s32 *)((u8 *)arg0 + 0x30) -= s2_ptr[6];
    *(s32 *)((u8 *)arg0 + 0x34) -= s2_ptr[7];

    /* Load 5 packed rotation-matrix words into GTE coef regs $0..$4. */
    {
        register s32 *mp asm("$12");
        s32 *src = mat_local;
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(src));
        t5 = mp[0];
        t6 = mp[1];
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
        t5 = mp[2];
        t6 = mp[3];
        t7 = mp[4];
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }

    /* Pack pos_diff low halves into V0.xy via mtc2 $0, load V0.z via lwc2 $1,
     * then mvmva (rotation matrix * V0, sf=1, cm=0, vm=0, tm=3). */
    {
        register s32 *vp asm("$12");
        s32 *src = (s32 *)((u8 *)arg0 + 0x2C);
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(src));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
    }
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");  /* mvmva 1, 0, 0, 3, 0 */

    /* Store MAC1/2/3 back to arg0+0x2C (rotated pos in place). */
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)((u8 *)arg0 + 0x2C);
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    /* Halve x, y, z (signed arithmetic shift). */
    *(s32 *)((u8 *)arg0 + 0x2C) >>= 1;
    *(s32 *)((u8 *)arg0 + 0x30) >>= 1;
    *(s32 *)((u8 *)arg0 + 0x34) >>= 1;
}

void func_800300B4(u8 *arg0) {
    s32 mac_result[3];
    s32 buf2[2];
    s32 buf3[8];
    s32 *playerData;
    s32 *mat;
    s32 lookup;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    playerData = (s32 *)game_GetPlayerData(arg0[6] < 1);
    mat = (s32 *)playerData[arg0[9]];

    /* Load rotation matrix into GTE cop2 control regs 0-4 */
    {
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(mat));
        t5 = mp[0];
        t6 = mp[1];
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
        t5 = mp[2];
        t6 = mp[3];
        t7 = mp[4];
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }

    /* Load V0 vector and run mvmva */
    {
        register s32 *vp asm("$12");
        s32 *vec_src = (s32 *)(arg0 + 0x2C);
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(vec_src));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
    }
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");  /* mvmva 1,0,0,3,0 */

    /* Store MAC1/2/3 to mac_result via $t4 alias */
    {
        register s32 *rp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(mac_result));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    /* Add matrix translation to MAC1-3 */
    mac_result[0] += mat[5];
    mac_result[1] += mat[6];
    mac_result[2] += mat[7];

    func_8007E4DC(mat, (s32 *)(arg0 + 0xC), buf3);
    func_8002F2D0(buf3, buf2);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, mac_result, (s16 *)buf2);
    saSeInit_2(arg0[10], lookup, mac_result, (u16 *)buf2);
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
    angle = single_game_getEnemyCharId(a1[0], a1[2]);
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
s32 *coli_hit_body_weapon(s32 *arg0, s32 arg1) {
    volatile s32 _spill[3];
    register u8 *t1 asm("t1") = (u8 *)arg0;
    register u8 *a3 asm("a3") = (u8 *)&D_80106A78;
    s32 i = 0;
    s32 v;
    u8 *p = a3 + 0xA;
    u8 *t0;
    s32 v1;
    do {
        if (((*((s16 *)(p - 8))) == (-1)) && ((*p) == 0xFF)) {
            v = 1;
            goto found;
        }
        i++;
        p += 0x64;
        a3 += 0x64;
    } while (i < 12);
    v = 1;
found:
    *((s8 *)(a3 + 0xA)) = i;
    *((s16 *)(a3 + 2)) = arg1;
    *((s8 *)(a3 + 7)) = 0;
    *((s8 *)(a3 + 8)) = 0;
    *((s8 *)(a3 + 4)) = v;
    *((s8 *)(a3 + 6)) = (s8)(*((u16 *)(t1 + 4)));
    *((s32 *)(a3 + 0x2C)) = *((s32 *)(t1 + 0xF4));
    v1 = *((s16 *)(t1 + 0x1A));
    *((s32 *)(a3 + 0x30)) = (*((s32 *)(t1 + 0xF8))) - (v1 / 32);
    *((s32 *)(a3 + 0x34)) = *((s32 *)(t1 + 0xFC));
    t0 = ((u8 *)(&D_8008E194)) + (2 * (arg1 * 7));
    *((s32 *)(a3 + 0x44)) = ((s32)((*((&Judge) + ((*((u16 *)(t1 + 0x1CA))) & 0xFFF))) * (*((s16 *)(t0 + 4))))) >> 12;
    *((s32 *)(a3 + 0x48)) = *((s16 *)(t0 + 6));
    {
        s32 tmp = (s32)((*((&Judge) + (((*((s16 *)(t1 + 0x1CA))) + 0x400) & 0xFFF))) * (*((s16 *)(t0 + 4))));
        *((volatile s32 *)(a3 + 0x2C)) += *((s32 *)(a3 + 0x44));
        *((volatile s32 *)(a3 + 0x30)) += *((s32 *)(a3 + 0x48));
        *((volatile s32 *)(a3 + 0x2C)) += ((s32)(*((s32 *)(a3 + 0x44)))) / 2;
        t1++;
        t1--;
        *((volatile s32 *)(a3 + 0x30)) += ((s32)(*((s32 *)(a3 + 0x48)))) / 2;
        *((s32 *)(a3 + 0x4C)) = tmp >> 12;
    }
    __asm__ volatile("" ::: "memory");
    *((volatile s32 *)(a3 + 0x34)) += *((s32 *)(a3 + 0x4C));
    *((volatile s32 *)(a3 + 0x34)) += ((s32)(*((s32 *)(a3 + 0x4C)))) / 2;
    __asm__ volatile("" ::: "memory");
    *((s32 *)(a3 + 0x38)) = *((s32 *)(a3 + 0x2C));
    *((s32 *)(a3 + 0x3C)) = *((s32 *)(a3 + 0x30));
    *((s32 *)(a3 + 0x40)) = *((s32 *)(a3 + 0x34));
    __asm__ volatile("" ::: "memory");
    *((s16 *)(a3 + 0x54)) = 0;
    *((u16 *)(a3 + 0x56)) = *((u16 *)(t1 + 0x1CA));
    *((s16 *)(a3 + 0x58)) = 0;
    {
        s16 sw = *((s16 *)t0);
        u16 t0_8;
        if (sw == 1) {
            goto label_1_or_3;
        } else if (sw == 2) {
            t0_8 = (*((u16 *)(a3 + 0x5C)) = *((u16 *)(t0 + 8)));
            *((s16 *)(a3 + 0x5E)) = 0;
            *((s16 *)(a3 + 0x60)) = 0;
        } else if (sw == 3) {
        label_1_or_3:
            *((s16 *)(a3 + 0x5C)) = 0;
            t0_8 = *((u16 *)(t0 + 8));
            *((s16 *)(a3 + 0x60)) = 0;
            *((u16 *)(a3 + 0x5E)) = t0_8;
        } else {
            *((s16 *)(a3 + 0x5C)) = 0;
            *((s16 *)(a3 + 0x5E)) = 0;
            *((s16 *)(a3 + 0x60)) = 0;
        }
    }
    *((s32 *)(a3 + 0x50)) = 1;
    *((s8 *)(a3 + 5)) = 0;
    *((s16 *)a3) = 0;
    return (s32 *)a3;
}
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
    s32 count = *(s16 *)(arg0 + 0x330);
    u16 c;
    if (count == 0xC) {
        return 0;
    }
    if (*(s16 *)(arg0 + 0x88) == -1) {
        goto skip_shift;
    }
    if (arg1 != *(s16 *)(arg0 + 0x14)) {
        goto skip_shift;
    }
    if (count > 0) {
        s32 i = count;
        do {
            *(u16 *)(arg0 + i * 2 + 0x332) = *(u16 *)(arg0 + i * 2 + 0x330);
            i--;
        } while (i > 0);
    }
    {
        u16 cc = *(u16 *)(arg0 + 0x330);
        *(u16 *)(arg0 + 0x332) = (u16)arg1;
        *(u16 *)(arg0 + 0x330) = cc + 1;
        goto done;
    }
skip_shift:
    c = *(u16 *)(arg0 + 0x330);
    *(u16 *)(arg0 + 0x330) = c + 1;
    *(u16 *)(arg0 + (s16)c * 2 + 0x332) = (u16)arg1;
done:
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
void func_80030D7C(void) {
}
void func_80031890(s32 arg0, s32 arg1, s32 arg2) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80031890.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
    (void)arg1;
    (void)arg2;
}
void func_80031B24(void) {
}
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
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800325E0.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
    (void)arg1;
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
void func_80032C50(s32 a0, s32 a1) {
    (void)a0;
    (void)a1;
}
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
    register u8 *t1 asm("t1") = (u8 *)&D_80106A50;
    register s32 a3 asm("a3");
    register s32 v1 asm("v1");
    s32 a0;

    a3 = 3;
    a0 = D_800A3858;
    v1 = a3 - 1;

loop1:
    if (*(s32 *)(t1 + v1 * 8 + 0xC) < a0) goto end1;
    a3 = v1;
    if (a3 > 0) {
        v1 = a3 - 1;
        goto loop1;
    }
end1:
    D_800A38E9 = (u8)a3;
    if (a3 < 3) {
        if (a3 < 2) {
            register s32 t0r asm("t0") = 2;
            register u8 *a2r asm("a2") = t1 + 0x10;
            do {
                *(s32 *)(a2r + 8) = *(s32 *)(a2r + 0);
                *(s32 *)(a2r + 0xC) = *(s32 *)(a2r + 4);
                t0r--;
                a2r -= 8;
            } while (a3 < t0r);
        }
        {
            register u8 *v3r asm("v1") = t1 + a3 * 8;
            v3r[0x8] = (u8)D_80101ED2;
            v3r[0x9] = (u8)D_80101ED6;
            *(s32 *)(v3r + 0xC) = D_800A3858;
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
void func_80034708(void) {
}
/* TABLED: -4 bytes, score 1980. Target alternates v1/a0 for g_file_flags address — unreproducible register allocation pattern */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = saTan2GaugeInit_80077D00();
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

    p = saTan2GaugeInit_80077D00();
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

    p = saTan2GaugeInit_80077D00();
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
