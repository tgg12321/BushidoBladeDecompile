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
extern u8 D_8008D118;
extern s32 D_800F33D8;
extern u32 D_800A378C;
extern u32 D_80101E3C;
extern u32 D_80101E44;
extern s32 D_800FF580;

/* Extern function declarations */
extern s32 func_80037110(s32);
extern void game_FrameInit(void);
extern void game_FrameLoop(void);
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
extern u16 D_800A38C4;
extern s16 D_80101F32;
extern void func_80035F30(s32, s32, s32, s32);
extern void obj_InitChars(void);
extern void obj_Reset(void);
extern void obj_InitTask(void);
extern void obj_InitPair(void);
extern void player_SetCharId(s32, s32);
extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
extern s32 D_80101E70;
extern s32 D_800A3894;
extern s8 D_80102781;
extern u16 D_800A3310;


extern s8 D_8010277D;
extern s8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void cpu_get_dist_2(u8 *);
extern void func_8003553C(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);

extern void sys_Panic(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);
extern s32 func_80054434(void);
extern void DispSchoolBG(s16 *, s16 *, s32 *, s32, s32);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern u8 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

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
extern s16 D_80101ED6;
extern u8 D_800F1B18[];
extern s32 g_file_disc_size;
extern s32 replay_camera_Init(s32, s32);
extern s32 func_80079154();
extern void func_800325E0(s32, s32);
extern void func_80046BF4(s32 *, s32 *, s32);
extern s32 game_GetPlayerData(s32);
extern s32 func_80032854(s32, s32, u8 *, s16 *);
extern void func_8002EECC(s32, s32 *);
extern void func_80061064(s32 *, s32 *);
extern s32 func_8007E11C(s32);
extern void func_8007F87C(s32, s32);
extern void func_8007FA1C(s32, s32);
extern void func_8007FBBC(s32, s32);
extern s32 func_80053614(s32 *, s32 *, s32 *, s32 *, s32);
extern u16 D_8008D59C;
extern s16 D_8008EB40;
extern u8 D_800F5F68;
extern s16 Judge;
extern void func_80033BC0(void);
extern void func_8001DA2C(void);
extern void game_SetPlayerCount(s32);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8003F3D4(s16 *);
extern void func_80055138(s32, s32, s32);
extern void func_8003FFE0(s32);
extern s32 D_80101F90;
extern s32 D_801020C0;
extern s32 D_80102114;
extern s32 camera_GetBoneData(void);
extern void func_80039320(void);
extern void func_8002C61C(void);
extern void func_80030D7C(void);
extern void func_800321E8(void);
extern void func_800397A0(void);
extern void func_8003E6A0(s32, s32);
extern void game_StageInit(s32);
extern void func_800335D8(void);
extern s32 D_80102030;
extern s8 D_800A3768;
extern void myRobGeneiDraw2(void);
extern void mk_leaf_newpos(void);
extern void mottest_disp(void);
extern void func_8005C650(s32, s32, s32);
extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern s32 func_8005D814(s16 *, s32, s32, s32);
extern void func_800550E8(s32);
typedef struct { s32 f0, f1, f2, f3; } Copy16;
extern void getScreenPosition(void);
extern void cpu_check_run_attack(s32 *);
extern void func_800372C0(void);
extern void func_80023F08(s32, s32);

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_80017FA0(s32 *a0) {
    s32 _frame;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    {
        s32 val = ptr[0] << 7;
        asm volatile("" : "=r"(val) : "0"(val));
        *(volatile s32 *)0x1F8000B8 = val;
    }

    {
        s32 i = 0;
        if (ptr[1] > 0) {
            s32 *p68 = ptr;
            volatile s32 *ac_base = (volatile s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j;
                s32 data_off;
                s32 sp_inner;

                j = 0;
                data_off = i << 5;
                sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(volatile s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(volatile s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(volatile s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                } while (j < 2);

                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }

    *(volatile s32 *)0x1F800060 = ((s32 *)a0[3])[1];

end:
    asm volatile("" : : "m"(_frame));
}
void marionation_camera_Exec(s32 *arg0, s32 *arg1) {
    register s32 *p0 asm("s0") = arg0;
    register s32 *p1 asm("s1") = arg1;
    s32 sp_area[4];

    {
        register s32 mat_ptr asm("a2") = p0[1];
        __asm__ volatile(
            ".word 0x00000000\n"
            "\t.word 0x00C06021\n"
            "\t.word 0x8D8D0000\n" "\t.word 0x8D8E0004\n"
            "\t.word 0x48CD0000\n" "\t.word 0x48CE0800\n"
            "\t.word 0x8D8D0008\n" "\t.word 0x8D8E000C\n" "\t.word 0x8D8F0010\n"
            "\t.word 0x48CD1000\n" "\t.word 0x48CE1800\n" "\t.word 0x48CF2000\n"
            :: "r"(mat_ptr)
        );
    }

    __asm__ volatile(
        ".word 0x8E060004\n"
        "\t.word 0x00000000\n"
        "\t.word 0x00C06021\n"
        "\t.word 0x8D8D0014\n" "\t.word 0x8D8E0018\n"
        "\t.word 0x48CD2800\n"
        "\t.word 0x8D8F001C\n"
        "\t.word 0x48CE3000\n" "\t.word 0x48CF3800\n"
    );

    getScreenPosition();

    {
        s32 *mat;
        s32 dx, dy, dz;
        s32 sum_sq;
        s32 scale;

        mat = (s32 *)p0[1];
        dx = mat[5] - p1[10];
        *(volatile s32 *)0x1F800024 = dx;

        mat = (s32 *)p0[1];
        dy = mat[6] - p1[11];
        *(volatile s32 *)0x1F800028 = dy;

        mat = (s32 *)p0[1];
        dz = mat[7] - p1[12];
        *(volatile s32 *)0x1F80002C = dz;

        sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

        scale = 0x100;
        if (sum_sq <= 250000) {
            if (sum_sq < 0) {
                scale = 0;
            } else {
                s32 log2_val;
                if (sum_sq < 0x400) {
                    log2_val = (u32)(*(&D_8008D118 + sum_sq)) >> 3;
                } else {
                    s32 shift_a, shift_b;
                    register s32 t4_v asm("t4");
                    asm volatile("" : "=r"(t4_v));
                    t4_v = sum_sq;
                    {
                        s32 *new_var = &sp_area[0];
                        asm volatile(".word 0x488CF000" : : "r"(t4_v));
                        asm volatile("nop");
                        asm volatile("nop");
                        {
                            s32 addr_v0 = (s32)new_var;
                            t4_v = addr_v0;
                            asm volatile(".word 0xE99F0000" : : "r"(t4_v));
                        }
                    }
                    {
                        s32 lw_v1 = sp_area[0];
                        s32 li_v0 = -2;
                        li_v0 = lw_v1 & li_v0;
                        shift_a = 0x16 - li_v0;
                    }
                    shift_b = shift_a >> 1;
                    log2_val = (((u8)(*(&D_8008D118 + (sum_sq >> shift_a)))) << 16) >> (0x13 - shift_b);
                }
                scale = ((log2_val << 6) / 500) + 0xC0;
            }
        }

        {
            s32 v24 = *(volatile s32 *)0x1F800024;
            s32 v28 = *(volatile s32 *)0x1F800028;
            s32 v2c = *(volatile s32 *)0x1F80002C;
            *(volatile s32 *)0x1F800024 = (v24 * scale) >> 1;
            *(volatile s32 *)0x1F800028 = (v28 * scale) >> 1;
            *(volatile s32 *)0x1F80002C = (v2c * scale) >> 1;
        }
    }

    {
        s32 *src = (s32 *)p0[1];
        *(Copy16 *)((u8 *)p1 + 0x14) = *(Copy16 *)src;
        *(Copy16 *)((u8 *)p1 + 0x24) = *(Copy16 *)((u8 *)src + 0x10);
    }

    cpu_check_run_attack(p1);
}
/* kengo:MED  |  nm_mario_cam/marionation_camera_Exec  |  155i */
void cpu_check_run_attack(s32 *arg) {
    register s32 t4 asm("t4");
    register s32 count asm("s0");
    register s32 *out asm("s1");
    register s32 sp_addr asm("s2");
    s32 sp_buf[6];

    s32 *data;
    s32 *base;
    s32 pair, radius;
    s32 thresh, neg_thresh;
    s32 dx, dy, dz;
    s32 *nodeA, *nodeB;

    out = (s32 *)0x1F8000BC;
    data = *(s32 **)((u8 *)arg + 0x10);
    base = *(s32 **)((u8 *)arg + 0xC);
    count = *(s16 *)((u8 *)arg + 6) - 1;

    pair = data[1];
    radius = data[0];

    nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
    nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));

    thresh = radius * 2 + radius;
    neg_thresh = -thresh;

    dx = nodeB[0] - nodeA[0];
    dy = nodeB[1] - nodeA[1];
    dz = nodeB[2] - nodeA[2];

    if (count <= 0) goto post_loop;
    goto range_check;

bisect:
    {
        s32 mid_x, mid_y, mid_z;

        mid_x = (nodeB[0] + nodeA[0]) / 2;
        mid_y = (nodeB[1] + nodeA[1]) / 2;
        mid_z = (nodeB[2] + nodeA[2]) / 2;

        if (nodeB[6] < 0) {
            s32 ext;
            nodeB[0] = mid_x;
            nodeB[1] = mid_y;
            nodeB[2] = mid_z;
            ext = nodeB[3];
            nodeB[3] = ext / 4;
            ext = nodeB[4];
            nodeB[4] = ext / 4;
            ext = nodeB[5];
            nodeB[5] = ext / 4;
        } else {
            s32 ext;
            nodeA[0] = mid_x;
            nodeA[1] = mid_y;
            nodeA[2] = mid_z;
            ext = nodeA[3];
            nodeA[3] = ext / 4;
            ext = nodeA[4];
            nodeA[4] = ext / 4;
            ext = nodeA[5];
            nodeA[5] = ext / 4;
        }

        dx = nodeB[0] - nodeA[0];
        dy = nodeB[1] - nodeA[1];
        dz = nodeB[2] - nodeA[2];
    }

range_check:
    if (dx < neg_thresh) goto bisect;
    if (thresh < dx) goto bisect;
    if (dy < neg_thresh) goto bisect;
    if (thresh < dy) goto bisect;
    if (dz < neg_thresh) goto bisect;
    if (thresh < dz) goto bisect;

    {
        s32 sum_sq, sqrt_val;
        s32 tmp;

        tmp = dx >> 3;
        __asm__ volatile("sw\t%0, 0x1F800000" :: "r"(tmp));
        tmp = dy >> 3;
        __asm__ volatile("sw\t%0, 0x1F800004" :: "r"(tmp));
        sp_addr = 0x1F800000;
        tmp = dz >> 3;
        __asm__ volatile("sw\t%0, 0x1F800008" :: "r"(tmp));

        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xC9890000\n"
            ".word 0xC98A0004\n"
            ".word 0xC98B0008\n"
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        data = (s32 *)((u8 *)data + 0x10);
        count--;

        sp_addr = 0x1F80000C;
        pair = data[1];
        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xE9990000\n"
            ".word 0xE99A0004\n"
            ".word 0xE99B0008\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        {
            s32 rd0, rd1, rd2;

            __asm__ volatile("lw\t%0, 0x1F80000C" : "=r"(rd0));
            __asm__ volatile("lw\t%0, 0x1F800010" : "=r"(rd1));
            __asm__ volatile("lw\t%0, 0x1F800014" : "=r"(rd2));
            sum_sq = rd0 + rd1 + rd2;
        }

        if ((u32)sum_sq < 0x400) {
            sqrt_val = *(&D_8008D118 + sum_sq);
            nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
            nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));
        } else {
            s32 shift_a, shift_b;

            t4 = sum_sq;
            __asm__ volatile(".word 0x488CF000" :: "r"(t4));

            nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
            nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));

            t4 = (s32)&sp_buf[0];
            __asm__ volatile(".word 0xE99F0000" :: "r"(t4));

            {
                s32 lzcr = sp_buf[0];
                s32 mask = -2;

                mask = lzcr & mask;
                shift_a = 0x16 - mask;
            }
            shift_b = shift_a >> 1;
            sqrt_val = ((u32)(*(&D_8008D118 + ((u32)sum_sq >> shift_a))) << 16) >> (0x10 - shift_b);
        }

        sp_addr = 0x1F800000;
        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xC9890000\n"
            ".word 0xC98A0004\n"
            ".word 0xC98B0008\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        {
            t4 = (sqrt_val - radius) << 14;
            radius = data[0];
            {
                t4 = t4 / sqrt_val;
                thresh = radius * 2 + radius;
                neg_thresh = -thresh;

                __asm__ volatile(
                    ".word 0x488C4000\n"
                    "nop\n"
                    "nop\n"
                    ".word 0x4B98003D\n"
                    :: "r"(t4)
                );
            }
        }

        t4 = (s32)out;
        dx = nodeB[0] - nodeA[0];
        dy = nodeB[1] - nodeA[1];
        dz = nodeB[2] - nodeA[2];

        __asm__ volatile(
            ".word 0xE9890000\n"
            ".word 0xE98A0004\n"
            ".word 0xE98B0008\n"
            :: "r"(t4)
        );
        out = (s32 *)((u8 *)out + 0xC);
    }

    if (count > 0) goto range_check;

post_loop:
    {
        s32 sum_sq, sqrt_val;
        s32 tmp;

        tmp = dx >> 3;
        __asm__ volatile("sw\t%0, 0x1F800000" :: "r"(tmp));
        tmp = dy >> 3;
        __asm__ volatile("sw\t%0, 0x1F800004" :: "r"(tmp));
        sp_addr = 0x1F800000;
        tmp = dz >> 3;
        __asm__ volatile("sw\t%0, 0x1F800008" :: "r"(tmp));

        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xC9890000\n"
            ".word 0xC98A0004\n"
            ".word 0xC98B0008\n"
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        sp_addr = 0x1F80000C;
        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xE9990000\n"
            ".word 0xE99A0004\n"
            ".word 0xE99B0008\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        {
            s32 rd0, rd1, rd2;

            __asm__ volatile("lw\t%0, 0x1F80000C" : "=r"(rd0));
            __asm__ volatile("lw\t%0, 0x1F800010" : "=r"(rd1));
            __asm__ volatile("lw\t%0, 0x1F800014" : "=r"(rd2));
            sum_sq = rd0 + rd1 + rd2;
        }

        if ((u32)sum_sq < 0x400) {
            sqrt_val = *(&D_8008D118 + sum_sq);
        } else {
            s32 shift_a, shift_b;

            t4 = sum_sq;
            __asm__ volatile(
                ".word 0x488CF000\n"
                "nop\n"
                "nop\n"
                :: "r"(t4)
            );

            t4 = (s32)&sp_buf[0];
            __asm__ volatile(".word 0xE99F0000" :: "r"(t4));

            {
                s32 lzcr = sp_buf[0];
                s32 mask = -2;

                mask = lzcr & mask;
                shift_a = 0x16 - mask;
            }
            shift_b = shift_a >> 1;
            sqrt_val = ((u32)(*(&D_8008D118 + ((u32)sum_sq >> shift_a))) << 16) >> (0x10 - shift_b);
        }

        sp_addr = 0x1F800000;
        __asm__ volatile(
            "addu\t$12, %0, $0\n"
            ".word 0xC9890000\n"
            ".word 0xC98A0004\n"
            ".word 0xC98B0008\n"
            : "=r"(sp_addr) : "0"(sp_addr)
        );

        {
            t4 = (sqrt_val - radius) << 14;
            t4 = t4 / sqrt_val;

            __asm__ volatile(
                ".word 0x488C4000\n"
                "nop\n"
                "nop\n"
                ".word 0x4B98003D\n"
                :: "r"(t4)
            );
        }

        t4 = (s32)out;
        __asm__ volatile(
            ".word 0xE9890000\n"
            ".word 0xE98A0004\n"
            ".word 0xE98B0008\n"
            :: "r"(t4)
        );
    }
}
/* kengo:HIGH  |  nm_cpu/cpu_check_run_attack  |  322i  |  +5 near-exact */
INCLUDE_ASM("asm/funcs", single_game_setModeRequest);
/* kengo:HIGH  |  nm_single_game/single_game_setModeRequest  |  663i  |  +1 near-exact */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0;
    s32 pad[2];
    s32 new_var;

    i = 0;
    if (arg1 <= 0) return;

    new_var = (s32)&g_file_data_buf;
    s0 = arg0;
    do {
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            single_game_setModeRequest(s0, (s32 *)(val * 52 + new_var));
        }
        i++;
        s0 = (s16 *)((u8 *)s0 + 16);
    } while (i < arg1);
}
void func_80019310(volatile s32 *arg0, s32 *arg1) {
    s32 sp_buf[6];
    register s32 *t0 asm("t0") = arg1;
    register s32 t1 asm("t1");
    register s32 *t4 asm("t4");
    register s32 t5 asm("t5");
    register s32 t6 asm("t6");
    register s32 t7 asm("t7");

    /* SetRotMatrix */
    t1 = arg0[1];
    t4 = (s32 *)t1;
    t5 = t4[0];
    t6 = t4[1];
    __asm__ volatile (".word 0x48CD0000" :: "r"(t5));
    __asm__ volatile (".word 0x48CE0800" :: "r"(t6));
    t5 = t4[2];
    t6 = t4[3];
    t7 = t4[4];
    __asm__ volatile (".word 0x48CD1000" :: "r"(t5));
    __asm__ volatile (".word 0x48CE1800" :: "r"(t6));
    __asm__ volatile (".word 0x48CF2000" :: "r"(t7));

    /* SetTransVector - reload */
    t1 = arg0[1];
    t4 = (s32 *)t1;
    t5 = t4[5];
    t6 = t4[6];
    __asm__ volatile (".word 0x48CD2800" :: "r"(t5));
    t7 = t4[7];
    __asm__ volatile (".word 0x48CE3000" :: "r"(t6));
    __asm__ volatile (".word 0x48CF3800" :: "r"(t7));

    {
        s16 count = *(s16 *)((u8 *)t0 + 4);
        s32 *output = (s32 *)t0[3];
        s32 i = 0;
        if (count > 0) {
            do {
                s32 *base = (s32 *)t0[0];
                t4 = (s32 *)((u8 *)base + (i << 3));
                __asm__ volatile (".word 0xC9800000" :: "r"(t4));
                __asm__ volatile (".word 0xC9810004" :: "r"(t4));
                __asm__ volatile ("nop");
                __asm__ volatile ("nop");
                __asm__ volatile (".word 0x4A480012");
                t4 = sp_buf;
                __asm__ volatile (".word 0xE9990000" :: "r"(t4));
                __asm__ volatile (".word 0xE99A0004" :: "r"(t4));
                __asm__ volatile (".word 0xE99B0008" :: "r"(t4));

                output[0] = sp_buf[0] << 7;
                output[1] = sp_buf[1] << 7;
                i++;
                output[2] = sp_buf[2] << 7;
                output[3] = 0;
                output[4] = 0;
                output[5] = 0;
                output = (s32 *)((u8 *)output + 0x40);
            } while (i < *(s16 *)((u8 *)t0 + 4));
        }
    }

    {
        typedef struct { s32 a[8]; } Copy8;
        s32 *src = (s32 *)arg0[1];
        *(Copy8 *)((u8 *)t0 + 0x14) = *(Copy8 *)src;
    }
}
void func_8001945C(void) {
    D_80106A70 = 0x11;
    D_80106A71 = 0x44;
    D_80106A72 = 0x88;
}
s32 func_80019488(void) {
    return (D_80106A70 & 0xF) | ((D_80106A71 & 0xF) << 4) | ((D_80106A72 & 0xF) << 8);
}
void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}
void func_800194F4(void) {
    D_80102788 = 4;
    D_8010278A = 4;
    D_80102790 = 0;
    D_80102794 = 0;
    D_80102798 = 0;
    D_8010279C = -1;
}
void func_80019534(void) {
    func_800194F4();
    D_8010278C = 1;
    D_8010278E = 1;
}
void single_game_VoiceContorol(s32 arg0) {
    typedef struct {
        u8 b0;
        u8 b1;
        u8 b2;
        u8 b3;
    } VoicePacket;
    struct {
        s16 output[4];
        s32 voice_mask;
        s32 unk_1C;
        s32 unk_20;
        s32 unk_24;
        s32 packets[4];
    } sp;
    register s32 arg0_reg asm("s0") = arg0;
    s32 voice_mask;
    s32 old_mask;
    s32 i;
    VoicePacket *packet;

    voice_mask = 0;
    sp.packets[0] = D_800FF580;
    sp.packets[1] = D_800FF584;
    sp.packets[2] = D_800FF5A4;
    sp.packets[3] = D_800FF5A8;

    packet = (VoicePacket *)&sp.packets[0];
    for (i = 0; i < 2; i++) {
        s32 bits = 0;

        if (packet->b0 == 0) {
            s32 voice = packet->b1 >> 4;

            sp.output[i] = voice;
            sp.output[i + 2] = 1;
            voice = (s16)((u16)sp.output[i] - 1);

            if ((u32)voice < 8) {
                switch (voice) {
                case 4:
                case 6:
                    sp.output[i] = 4;
                    /* fallthrough */
                case 1:
                case 2:
                case 3:
                    bits = ~((packet->b2 << 8) | packet->b3);
                    break;
                case 0:
                case 5:
                case 7:
                default:
                    break;
                }
            }
        } else {
            sp.output[i] = 4;
            sp.output[i + 2] = 0;
        }

        voice_mask = ((u32)voice_mask >> 16) | (bits << 16);
        packet = (VoicePacket *)((u8 *)packet + 8);
    }

    sp.voice_mask = voice_mask;
    func_8001B138(&sp.voice_mask);

    if (D_800A3834 == 1 && arg0_reg == 0) {
        u16 voice_state = D_800A38DC;

        if (voice_state < 7) {
            switch (voice_state) {
            case 4:
            case 5:
                if (D_8010278E == 0) {
                    sp.voice_mask |= 0x08000800;
                }
                /* fallthrough */
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
                if (D_8010278C == 0) {
                    sp.voice_mask |= 0x08000800;
                }
                break;
            }
        }
    }

    func_8003A728((s32)&sp.output[0]);

    {
        s16 *src = &sp.output[0];
        s16 *dst0 = &D_80102788;
        s16 *dst1 = &D_8010278C;

        for (i = 0; i < 2; i++) {
            dst0[0] = src[0];
            dst0++;
            dst1[0] = src[2];
            dst1++;
            src++;
        }
    }

    old_mask = D_80102790;
    arg0_reg = (D_80102794 = sp.voice_mask & ~old_mask);
    D_80102790 = sp.voice_mask;
    D_8010279C = ~sp.voice_mask;
    D_80102798 = ~sp.voice_mask & old_mask;
}
INCLUDE_RODATA("asm/rodata", D_800100A4);
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    s32 val;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    dst = base;
    do {
        if (bits_left < 0xC) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    i = 0;
    dst = base;
    do {
        if (bits_left < 2) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    val = -2;
    i = 3;
    dst = base + 0x348;
    do {
        *(s32 *)(dst + 0x110) = val;
        i--;
        dst -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
INCLUDE_ASM("asm/funcs", func_800198D0);
void func_8001A484(u16 *arg0) {
    s32 i;
    u16 *p;
    i = 0;
    p = arg0 + 2;
    do {
        i++;
        DispSleepMenuTex((s32)&D_800100A4, arg0[0], p[-1], p[0]);
        p += 3;
        arg0 += 3;
    } while (i < 0x16);
}
s32 func_8001A4F0(s32 arg0, s32 arg1) {
    s32 v = arg0 & 0xFFF;
    if (v >= 0x800) {
        v -= 0x1000;
    }
    return v / arg1;
}
void func_8001A538(s32 *arg0, s32 *arg1) {
    s16 mat[9];
    s32 pad[2];
    mat[0] = 0x1000;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;
    mat[4] = 0x1000;
    mat[5] = 0;
    mat[6] = 0;
    mat[7] = 0;
    mat[8] = 0x1000;
    func_8007F87C(-*(s16 *)((u8 *)arg0 + 0x10), (s32)mat);
    func_8007FA1C(-*(s16 *)((u8 *)arg0 + 0x12), (s32)mat);
    func_8007FBBC(-*(s16 *)((u8 *)arg0 + 0x14), (s32)mat);
    arg1[0] = arg0[0] - ((s32)(mat[2] * arg0[6]) >> 12);
    arg1[1] = arg0[1] - ((s32)(mat[5] * arg0[6]) >> 12);
    arg1[2] = arg0[2] - ((s32)(mat[8] * arg0[6]) >> 12);
}
s32 func_8001A62C(s32 arg0) {
    if (arg0 < 0) {
        return -((0x7CF - arg0) / 2000);
    }
    return arg0 / 2000;
}
void func_8001A67C(s16 *arg0, s32 *arg1, s32 *arg2) {
    s32 *new_var;
    s32 dx;
    s32 dz;
    u32 dist_sq;
    u32 log2_val;
    s32 sp_tmp;
    dx = arg1[0] - arg2[0];
    dz = arg1[2] - arg2[2];
    while ((((u32)(dx + 0x4000)) > 0x8000U) || (((u32)(dz + 0x4000)) > 0x8000U)) {
        dx = dx / 2;
        dz = dz / 2;
    }
    dist_sq = (dx * dx) + (dz * dz);
    if (dist_sq < 0x400U) {
        log2_val = ((u32)((u8)(*((&D_8008D118) + dist_sq)))) >> 3;
    } else {
        u32 shift_a;
        u32 shift_b;
        register s32 t4_v asm("t4");
        asm volatile("" : "=r"(t4_v));
        t4_v = (s32)dist_sq;
        new_var = &sp_tmp;
        asm volatile(".word 0x488CF000" : : "r"(t4_v));
        asm volatile("nop");
        asm volatile("nop");
        {
            s32 addr_v0 = (s32)new_var;
            t4_v = addr_v0;
            asm volatile(".word 0xE99F0000" : : "r"(t4_v));
        }
        {
            s32 lw_v1 = sp_tmp;
            s32 li_v0 = -2;
            li_v0 = lw_v1 & li_v0;
            shift_a = 0x16 - li_v0;
        }
        shift_b = shift_a >> 1;
        log2_val = (((u32)((u8)(*((&D_8008D118) + (dist_sq >> shift_a))))) << 16) >> (0x13 - shift_b);
    }
    arg0[0] = (s16)func_8001A62C(arg2[0] + ((dx << 10) / ((s32)log2_val)));
    arg0[2] = (s16)func_8001A62C(arg2[2] + ((dz << 10) / ((s32)log2_val)));
}
INCLUDE_ASM("asm/funcs", func_8001A820);
void func_8001B138(s32 *arg0) {
    D_800FF5C8 = 0;
    D_800FF5CC = 0;
    D_800FF5D0 = 0;
    D_800FF5D8 = 0;
    D_800FF5DA = 0;
    D_800FF5DC = 0;
    D_800FF5E0 = 0;
    if (D_800A38BA != 0 && D_800A3834 == 1) {
        if (*arg0 & 1) {
            D_800A37E0 = 1;
            if (*arg0 & 8) {
                g_file_vram_timer = (u16)(g_file_vram_timer + 0x4CC);
            }
            if (*arg0 & 2) {
                g_file_vram_timer = (u16)(g_file_vram_timer - 0x4CC);
            }
            if ((s16)g_file_vram_timer < -0x1C00) {
                { s16 tmp = -0x1C00; g_file_vram_timer = tmp; }
            }
            if ((s16)g_file_vram_timer >= 0x7401) {
                g_file_vram_timer = 0x7400;
            }
            *arg0 = *arg0 & ~0xB;
        }
        {
            s32 v;
            v = (s16)g_file_vram_timer;
            if (v < 0) {
                v = v + 0xF;
            }
            D_800FF5E0 = v >> 4;
        }
    }
    *arg0 = *arg0 & (s32)0xFFFEFFFE;
}
void func_8001B294(s32 *a0, s32 *a1) {    s32 v0;    D_800A36FA = 0;    D_800F6638 = 0x64;    D_800F663A = 0;    D_800F663C = 0x64;    D_800F6640 = 0x64;    D_800F6642 = 0;    D_800F6644 = 0x64;    game_SetControllerPorts(0);    D_800F6608 = (*(s32 *)((u8 *)a0 + 0xF4) + *(s32 *)((u8 *)a1 + 0xF4)) / 2;    D_800F660C = (*(s32 *)((u8 *)a0 + 0xF8) + *(s32 *)((u8 *)a1 + 0xF8)) / 2;    {        s32 t1 = *(s32 *)((u8 *)a0 + 0xFC);        s32 t2 = *(s32 *)((u8 *)a1 + 0xFC);        D_800F6618 = 0;        D_800F6610 = (t1 + t2) / 2;    }    {        s32 dx = *(s32 *)((u8 *)a1 + 0xF4) - *(s32 *)((u8 *)a0 + 0xF4);        s32 dy = *(s32 *)((u8 *)a1 + 0xFC) - *(s32 *)((u8 *)a0 + 0xFC);        v0 = func_8007FD5C(dx, dy);    }    D_800F661A = 0x400 - v0;    D_800F661C = 0;    D_800F6620 = 0x1388;    D_800F6626 = 0;}
void func_8001B3C0(s32 *a0, s32 *a1) {    D_800A36FA = 0;    D_800F5358 = 0x64;    D_800F535A = 0;    D_800F535C = 0x64;    D_800F5360 = 0x64;    D_800F5362 = 0;    D_800F5364 = 0x64;    game_SetControllerPorts(0);    if (D_800A36F6 != 0) {        a0 = a1;    }    D_800F5328 = *(s32 *)((u8 *)a0 + 0x180);    D_800F5330 = *(s32 *)((u8 *)a0 + 0x188);    {        s32 v = *(s32 *)((u8 *)a0 + 0x184);        D_800F5368 = 0;        D_800F532C = v;    }}
void myRobGeneiMove(s32 arg0) {
    u8 *obj = (u8 *)arg0;
    u8 *s2 = (u8 *)&D_800F5328;
    s32 a2;
    s32 val;
    s32 far;

    game_SetControllerPorts(0);

    val = (*(s32 *)(obj + 0x19C) + *(s32 *)(obj + 0x1A8)) / 2 - *(s32 *)(obj + 0x184);
    far = val >= 0x391;

    if (*(u16 *)(obj + 0x6A) == 0x2A) {
        val = 0x200;
    } else {
        *(s32 *)s2 = *(s32 *)(obj + 0x180);
        D_800F5330 = *(s32 *)(obj + 0x188);
        val = *(s32 *)(obj + 0x184);

        if (!far) {
            s32 v = -(*(s16 *)(obj + 0x1A) * 950);
            if (v < 0) {
                v += 0xFFF;
            }
            val += v >> 12;
        }

        {
            s32 diff = val - *(s32 *)(s2 + 4);
            if (diff < 0) {
                diff += 3;
            }
            a2 = *(s32 *)(s2 + 4) + (diff >> 2);
            *(s32 *)(s2 + 4) = a2;
        }

        if (*(u16 *)(obj + 0x6A) == 0x2A) {
            val = 0x200;
        } else {
            s32 angle = (-(*(s16 *)(obj + 0x1D8)) - *(s16 *)(s2 + 0x12)) & 0xFFF;

            if (angle >= 0x800) {
                angle = 0x1000 - angle;
            }
            if (angle >= 0x400) {
                angle = 0x400;
            }

            {
                s32 base_val = *(s32 *)(*(s32 *)obj + 0xF8);
                s32 result = func_8007FD5C(base_val - a2, D_800A387C);
                val = (result * (0x400 - angle)) >> 10;
            }
        }
    }

    {
        s16 old = *(s16 *)(s2 + 0x10);
        s32 diff = val - old;
        if (diff < 0) {
            diff += 7;
        }
        *(s16 *)(s2 + 0x10) = old + (diff >> 3);
    }
    *(s16 *)(s2 + 0x14) = 0;

    {
        s32 neg_angle = -(*(s16 *)(obj + 0x1CA));
        s16 counter = D_800A36FC;

        if (counter != 0) {
            s16 old12 = *(s16 *)(s2 + 0x12);
            s32 diff = neg_angle - old12;
            if (diff < 0) {
                diff += 3;
            }
            {
                s16 cnt = counter - 1;
                *(s16 *)(s2 + 0x12) = old12 + (diff >> 2);
                D_800A36FC = cnt;
            }

            {
                s32 decay = *(s16 *)(s2 + 0x1C) * 3;
                if (decay < 0) {
                    decay += 3;
                }
                *(s16 *)(s2 + 0x1C) = decay >> 2;
            }
        } else {
            *(s16 *)(s2 + 0x12) = neg_angle;
            *(s16 *)(s2 + 0x1C) = 0;
        }
    }
    *(s32 *)(s2 + 0x18) = 0;
}

/* kengo:MED  |  my_eff/myRobGeneiMove  |  134i */
void func_8001B690(s32 arg0, s32 arg1) {
    if (D_800A38BA == 0) {
        return;
    }
    if (D_800A36F6 != arg0) {
        return;
    }
    arg1 &= 0xFFF;
    if (arg1 >= 0x800) {
        arg1 = 0x1000 - arg1;
    }
    if (arg1 >= 0x401) {
        D_800A36FC = 0x19;
        D_800F5344 = 0x800;
    }
}
void func_8001B6F4(void) {
    func_80041688(0, 0);
    func_80041688(1, 0);
    D_800A36FA = 1;
    D_800F6627 = 0;
    D_800F5347 = 0;
    game_SetControllerPorts(0);
}
void DispPracticeMenuTex_A(u8 *dst, u8 *a, u8 *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    int new_var;
    s32 dx;
    s32 dy;
    u8 *base = (u8 *)&D_80101EC8 + D_800A3748 * 0x44C;
    s32 dz;
    s32 cur;
    s32 target;
    s32 use_high;
    s32 v;
    if (dst[0x1F] == 0) {
        dst[0x1F] = 1;
        *((s32 *) (dst + 0)) = ((frac * (*((s16 *) (a + 4)))) + (inv_frac * (*((s16 *) (b + 4))))) >> 12;
        *((s32 *) (dst + 4)) = (((frac * (*((s16 *) (a + 6)))) + (inv_frac * (*((s16 *) (b + 6))))) >> 12) - 0x12C;
        new_var = (frac * (*((s16 *) (a + 8)))) + (inv_frac * (*((s16 *) (b + 8))));
        D_800A3310 = 0;
        *((s16 *) (dst + 0x12)) = val;
        cur = new_var;
        *((s16 *) (dst + 0x10)) = 0x80;
        *((s16 *) (dst + 0x14)) = 0;
        dy = cur;
        *((s32 *) (dst + 0x18)) = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        *((s32 *) (dst + 8)) = dy >> 12;
        return;
    }
    {
        s32 sum = (*((s32 *) (base + 0x19C))) + (*((s32 *) (base + (0x1A8 & 0xFFFFFFFF))));
        s32 avg = ((s32) (sum + (((u32) sum) >> 31))) >> 1;
        if ((avg - (*((s32 *) (base + 0x184)))) < 0xC8) {
            D_800A3310 += 1;
        }
    }
    use_high = ((s16) D_800A3310) >= 0xB;
    cur = *((s32 *) (dst + 0));
    inv_s1 = inv_s1;
    dx = (((frac * (*((s16 *) (a + 4)))) + (inv_frac * (*((s16 *) (b + 4))))) >> 12) - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    *((s32 *) (dst + 0)) = cur + (dx >> 4);
    cur = *((s32 *) (dst + 4));
    dy = (((frac * (*((s16 *) (a + 6)))) + (inv_frac * (*((s16 *) (b + 6))))) >> 12) - (cur + 0x12C);
    if (dy < 0) {
        dy += 0xF;
    }
    *((s32 *) (dst + 4)) = cur + (dy >> 4);
    cur = *((s32 *) (dst + 8));
    dz = (((frac * (*((s16 *) (a + 8)))) + (inv_frac * (*((s16 *) (b + 8))))) >> 12) - cur;
    if (dz < 0) {
        dz += 0xF;
    }
    *((s32 *) (dst + 8)) = cur + (dz >> 4);
    if (use_high) {
        target = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        target = 0x80 - ((frac_s1 << 8) >> 12);
    }
    *((s16 *) (dst + 0x10)) = (*((u16 *) (dst + 0x10))) + func_8001A4F0(target - (*((s16 *) (dst + 0x10))), 0x10);
    v = func_8001A4F0(val - (*((s16 *) (dst + 0x12))), 0x10);
    *((s16 *) (dst + 0x14)) = 0;
    *((s16 *) (dst + 0x12)) = (*((u16 *) (dst + 0x12))) + v;
    if (use_high) {
        target = frac_s1 * 0x7D0;
    } else {
        target = frac_s1 * 0x1F4;
    }
    cur = *((s32 *) (dst + 0x18));
    dx = ((target + (inv_s1 * 0x2EE0)) >> 12) - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    *((s32 *) (dst + 0x18)) = cur + (dx >> 4);
    *((s16 *) (dst + 0x30)) = 0x64;
    *((s16 *) (dst + 0x32)) = 0;
    *((s16 *) (dst + 0x34)) = 0x64;
    *((s16 *) (dst + 0x38)) = 0x64;
    *((s16 *) (dst + 0x3A)) = 0;
    *((s16 *) (dst + 0x3C)) = 0x64;
}
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
void func_8001BAE4(s32 *arg0, s32 *arg1, s32 arg2) {
    s32 temp_a2;
    s32 var_s3;
    s32 var_v1;
    s32 var_v0;

    if (D_800A387C < 0x2711) {
        var_s3 = (arg2 / 2) + 0x800;
    } else {
        var_s3 = 0x1000;
    }
    temp_a2 = func_8007FD5C(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4),
                             *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8));
    var_v1 = arg2;
    if (arg2 < 0) {
        var_v1 = arg2 + 3;
    }
    var_v0 = ((s16*)&Judge)[((var_v1 >> 1) & 0x1FFE) >> 1] * 3;
    if (var_v0 < 0) {
        var_v0 += 3;
    }
    DispPracticeMenuTex_A(&D_800F6608, arg0, arg1, (s32 *)arg2, var_s3, (0x500 - temp_a2) - (var_v0 >> 2));
}
void func_8001BBD8(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 temp_s0;
    temp_s0 = (D_800A387C < 0x2711) << 0xB;
    DispPracticeMenuTex_A((s32 *)&D_800F5328, arg0, arg1, arg2, temp_s0, -0x200 - func_8007FD5C(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4), *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8)));
}
void func_8001BC70(u8 *arg0, s32 arg1) {
    typedef struct { s32 x, y, z; } Vec3;
    Vec3 *dst;
    Vec3 *src;
    game_SetControllerPorts(0);
    dst = (Vec3 *)&D_800F6608;
    src = (Vec3 *)(arg0 + 0x174);
    *dst = *src;
    D_800F6618 = 0x120;
    D_800F661A = arg1;
    D_800F661C = 0;
    D_800F6620 = 0x1162;
}
void func_8001BCF0(u8 *arg0, s32 arg1) {
    typedef struct { s32 x, y, z; } Vec3;
    s32 diff = 0x1000 - arg1;

    game_SetControllerPorts(0);

    *(Vec3 *)&D_800F6608 = *(Vec3 *)(arg0 + 0xB8);

    D_800F660C -= 0x44C;

    D_800F6618 = 0x100 - (arg1 * 288) / 4096;

    {
        s32 div4 = arg1 / 4;
        s32 sum = arg1 * 3000 + diff * 8000;
        u16 lhu_val = *(u16 *)(arg0 + 0x1CA);
        s32 val;
        D_800F6620 = sum >> 12;
        val = 0xB00 - div4;
        D_800F661C = 0;
        D_800F661A = val - lhu_val;
    }
}
void func_8001BE08(s32 *arg0) {
    arg0[2] = 0;
    arg0[3] = 0;
    arg0[4] = 0;
    arg0[5] = -1;
}
INCLUDE_ASM("asm/funcs", func_8001BE20);
void func_8001C444(void) {
    D_8010277A = 0x800;
    D_80102778 = 0x800;
    D_8010277C = 1;
    D_8010277D = 0x10;
    D_80102784 = 0xC;
    D_8010277E = 0;
    D_8010277F = 0;
    D_80102781 = 0;
    D_80102780 = 0;
    D_80102785 = 4;
    D_80102786 = 0;
    D_80102787 = 0;
}
void func_8001C4C0(void) {
    u16 v = D_80101F32;
    if (v == 0x32 || v == 0x11) {
        func_800218C8(0);
        {
            s32 v0 = func_80021974(0);
            D_80101F26 = 0;
            func_80021A98(0, v0, 0);
        }
    }
}
void func_8001C51C(void) {
    s16 *s0;
    s32 v0;
    func_8001C4C0();
    if (D_800A38DC == 3 && D_800A3728 != 0) {
        func_80022580(1, D_80102781, D_8010277D, D_8010277F, 0);
    } else {
        func_80022580(1, D_80102781, D_8010277D, D_8010277F, 1);
    }
    func_80022F34();
    func_800218C8(1);
    v0 = func_80021974(1);
    s0 = &D_80102372;
    *s0 = 0;
    func_80021A98(1, v0, 0);
    D_800A382E = 0;
    D_800A3748 = -1;
    func_80030524();
    func_80030D04();
    func_8001B294((s32)((u8 *)s0 - 0x4AA), (s32)((u8 *)s0 - 0x5E));
    func_800392C8();
    func_80021280(1);
}
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 a1;
    s32 v1;
    s32 a0_val;

    v1 = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[v1], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    do {
        a1 = ((s32 *)&D_80101EC8)[0x36];
        v1 = D_80101FA4;
        a0_val = D_80101FA8;
        D_80101FB4 = -0x384;
        D_80101FB8 = 0;
        D_80101FBC = a1;
        D_80101FC0 = v1 - 0x384;
        D_80101FC4 = a0_val;
        D_80101F80 = a1;
        {
            D_80101F84 = v1;
            D_80101F88 = a0_val;
            do {
                *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
                *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
                D_80101FCC = 0;
                D_80101FD0 = 0;
            } while (0);
            D_80101FD4 = 0;
            *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
            {
                s32 t = local[0];
                s32 u = local[2];
                local[0] = t;
                local[1] -= 0x384;
                local[2] = u;
            }
            D_80101FDC = 0;
            D_80101FE0 = 0;
        }
        D_80101FE4 = 0;
        D_80101FEC = 0;
        D_80101FF0 = 0;
        D_80101FF4 = 0;
    } while (0 != 0);
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F84;
    func_8003FFE0(0);
}
void func_8001C820(void) {
    s16 *s0 = &D_80101ED2;
    s32 a0;
    if ((&D_8008D9EC)[*s0] != 0) {
        if (D_800A37A0 == 1) return;
    }
    if (D_800A38DC != 0) return;
    if (D_800A3712 != 0) return;
    a0 = 0x56;
    if (D_800A3680 != D_800A3671) {
        if (func_80079154(0x56) & 1) {
            a0 = 0x57;
        } else {
            a0 = 0x58;
        }
    }
    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));
}
INCLUDE_RODATA("asm/rodata", jtbl_800100C4);
INCLUDE_ASM("asm/funcs", func_8001C8DC);
void func_8001CD68(s16 *arg0) {
    s32 val = D_800A3858;

    if (val > 0x2BF1F) {
        *(s16 *)arg0 = 99;
        *((u8 *)arg0 + 2) = 59;
        *((u8 *)arg0 + 3) = 99;
        return;
    }
    {
        s32 minutes = val / 1800;
        s32 seconds = val / 30 - minutes * 60;
        *((u8 *)arg0 + 2) = seconds;
        {
            s32 frames;
            s32 centiseconds = (D_800A3858 % 30) * 100 / 30;
            *(s16 *)arg0 = minutes;
            *((u8 *)arg0 + 3) = centiseconds;
        }
    }
}
void camera_set_target_zoom(void) {
}
/* kengo:MED  |  nm_camera/camera_set_target_zoom  |  593i  |  +5 */
extern s8 D_800A30FC;
extern s8 D_800A30FD;
extern s32 D_800FF6A8;
void se_data_set(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        s8 *p = (s8 *)&D_8010277C;

        func_80020D38();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + *p), *(&D_8008E5A8 + D_8010277D));

        D_800A390E = (s8)(u16)D_800A36A4;
        D_800A30FC = *(&D_8008E5A8 + *p);
        D_800A30FD = *(&D_8008E5A8 + D_8010277D);

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        bb2_memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
/* kengo:HIGH  |  md_game/se_data_set  |  93i */
void func_8001D904(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_EnableDisplay();
    func_80020D38();
    obj_InitTask();
    s1 = obj_InitTaskCamera((s32)0x80190800);
    if (s1 >= 0xE81) {
        sys_Panic();
    }
    s0 = &D_80104F38;
    bb2_memcpy(s0, (s32)0x80190800, s1);
    obj_ExecTask((s32)s0 - s2);
}
void func_8001D998(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_EnableDisplay();
    func_80020D38();
    obj_InitPair();
    s1 = func_8005B8B8((s32)0x80190800);
    if (s1 >= 0x1B19) {
        sys_Panic();
    }
    s0 = &D_80104F38;
    bb2_memcpy(s0, (s32)0x80190800, s1);
    func_8005B98C((s32)s0 - s2);
}
void func_8001DA2C(void) {
    obj_InitChars();
    obj_Reset();
    if (D_800A38DC == 5) {
        obj_InitTask();
    }
    if (D_800A38DC == 3) {
        obj_InitPair();
    }
}
void func_8001DA8C(void) {
    func_80035FA8();
    if (file_GetFlag2()) {
        return;
    }
    switch (D_800A38DC) {
        case 0:
        case 1:
        default:
            break;
        case 4:
            func_80037110((&D_8008D518)[D_800A36A4]);
            break;
        case 3:
            if ((&D_8008D9EC)[D_80101ED2] != 0) {
                func_80037110(9);
            } else {
                func_80037110(8);
            }
            break;
        case 2:
            if (D_800A389A != 0) {
                func_80037110(0xA);
            } else {
                func_80037110(0xB);
            }
            break;
        case 5:
            break;
    }
    func_800371E8(1);
}
s32 func_8001DB58(void) {
    s32 v = D_800A38DC;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return file_GetFlag2();
    }
    return 1;
}
void func_8001DB9C(void) {
    seq_Start((&D_8008D9EC)[D_80101ED2] < 1, (s32)0x80190800);
    D_800A38C6 = (u16)0xFFFF;
}
void func_8001DBE4(void) {
    s32 i;

    if (g_disp_enable != DISP_ACTIVE) {
        return;
    }
    func_8003AA78();
    if (D_800A38F8 > D_800A37A0) {
    } else {
        do {
            func_8003AA48();
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            func_8003AA48();
            i += 1;
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (i < 15);
    }
    func_8003AAB0();
    gpu_InitDisplay();
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", mario_test_Exec);
/* kengo:MED  |  nm_mario_test/mario_test_Exec  |  450i  |  -19 */
typedef struct {
    s32 vx, vy, vz;
    s32 pad0;
    u16 rx, ry, rz;
    u16 pad1;
    s32 dist;
    s32 tail[10];
} CamBuf;

void func_8001E404(void) {
    s32 pre_pad[2];
    CamBuf local;
    s32 *s2;

    if (D_800A38BA != 0) {
        s32 v3 = D_800A36FA;
        if (v3 == 1) {
            if (D_80101F5E != 0 || D_801023AA != 0) {
                D_800A36FA = 2;
                v3 = D_800A36FA;
            }
        }
        if (v3 == 2) goto s2_default;
        if ((u16)D_80101F32 == 0x11 || (u16)D_8010237E == 0x11) {
            s2 = (s32 *)&D_800F6608;
            D_800A36FA = 1;
        } else {
            s2 = (s32 *)&D_800F5328;
            D_800A36FA = 0;
        }
        goto done_s2;
    s2_default:
        s2 = (s32 *)&D_800F6608;
    done_s2:

        game_SetPlayerCount(D_800A36FA < 1);

        {
            s32 fov = 0x2D;
            if (D_800A36FA == 0) {
                fov = 0x50;
            }
            func_8007EFFC(disp_CalcFov(fov));
        }

        if (D_800A36FA == 0) {
            func_80041688(D_800A36F6, 1);
            func_80041688(D_800A36F6 == 0, 0);
        } else {
            func_80041688(0, 0);
            func_80041688(1, 0);
        }
        goto common_tail;
    }
    s2 = (s32 *)&D_800F6608;
common_tail:

    if (D_800A3834 == 1) {
        local.vx = s2[0] + D_800FF5C8;
        local.vy = s2[1] + D_800FF5CC;
        local.vz = s2[2] + D_800FF5D0;
        local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
        local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
        local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;
        local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    } else {
        local = *(CamBuf *)s2;
    }

    func_80046BF4((s32 *)&local, (s32 *)&local.rx, local.dist);
    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }
    func_8003F3D4((s16 *)((u8 *)s2 + 0x30));
    func_8003F3D4((s16 *)((u8 *)s2 + 0x38));
    D_800A36B4 = (s32)s2;
}
typedef struct {
    s32 vx, vy, vz;
    s32 pad0;
    s16 rx, ry, rz;
    s16 pad1;
    s32 dist;
    s32 pad2[11];
} CamWork;

void func_8001E6E4(s32 arg0) {
    s32 pre_pad[2];
    CamWork local;
    s32 *s2;

    s2 = (s32 *)&D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = (s32 *)&D_800F6608;
    }

    local.vx = s2[0] + D_800FF5C8;
    local.vy = s2[1] + D_800FF5CC;
    local.vz = s2[2] + D_800FF5D0;
    local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
    local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
    local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;

    local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    func_80046BF4((s32 *)&local, &local.rx, local.dist);

    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }

    D_800A36B4 = (s32)s2;
}
void func_8001E800(void) {
    s32 v = D_800A36F6;
    volatile u8 *ptr = (volatile u8 *)(&D_80101EC8 + v * 1100);
    s32 a1 = -1;
    if (ptr[0x62] & 1) {
        a1 = *(s16 *)(ptr + 0xE);
    }
    {
        u32 flags = ptr[0x62] & 4;
        func_80048BA4(D_800F5344, a1, flags > 0);
    }
}
void func_8001E878(void) {
    s32 buf[6];
    s32 v0;
    s32 *a0 = &D_80102030;
    u8 *s0;
    v0 = camera_GetBoneData();
    s0 = (u8 *)a0 - 0x168;
    D_800A3778 = v0;
    func_8001A820((s32)a0, (s32)((u8 *)a0 + 0x44C), (s32)s0, (s32)((u8 *)a0 + 0x2E4));
    if (D_800A38BA != 0) {
        myRobGeneiMove((s32)(s0 + D_800A36F6 * 1100));
    }
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE20(0, (s32)buf);
    func_80023F08(0, (s32)buf);
    func_8001BE20(1, (s32)buf);
    func_80023F08(1, (s32)buf);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    if (D_800A38BA != 0 && D_800A36FA == 0) {
        func_8001E800();
    } else {
        func_8003E6A0(D_80101FBC, D_80101FC4);
        func_8003E6A0(D_80102408, D_80102410);
    }
    game_StageInit((D_800A3690 ^ 1) != 0);
    camera_set_target_zoom();
    func_800335D8();
    func_8001C8DC();
}
void func_8001EA04(void) {
    u8 v;
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    v = D_800A38D4;
    D_8010262E = 0;
    D_801021E2 = 0;
    D_800A37B8 = 0;
    D_800A3929 = 0;
    D_800A3834 = 0xD;
    D_800A3804 = v < 1;
    D_800A3817 = v < 1;
}
void cpu_get_move_pattern_table_number(void) {
    s32 sp10[6];
    s16 buf[4];
    s32 ret;
    u8 *base;

    D_800A37B8 += 1;
    D_800A3778 = camera_GetBoneData();
    base = (u8 *)&D_80101EC8;
    if (D_800A3748 == 0) {
        base += 0x44C;
    }
    func_8001BC70(base, D_800A37B8 << 3);
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE08(sp10);
    func_80023F08(0, (s32)sp10);
    func_80023F08(1, (s32)sp10);
    func_8002C61C();
    func_80030D7C();
    myRobGeneiDraw2();
    func_800397A0();
    game_StageInit(1);
    mk_leaf_newpos();
    if (D_800A38DC == 3) {
        func_8001CD68(buf);
        D_800A38B4 = D_800A38B4 + ((func_8005D814(buf, D_800A38E2, D_800A38B4, 1) / 4) * 4);
    }
    if (D_800A3929 == 0) {
        D_800A38B4 = D_800A38B4 + ((func_8005C8A8(1, D_800A3817, D_800A38B4, 0) / 4) * 4);
        if ((D_80102794 & 0x10001000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 != D_800A3804) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((D_80102794 & 0x40004000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = D_800A3804;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }
        if ((D_80102794 & 0x400040) != 0) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
            if (D_800A3817 != 0) return;
            (&D_80101F7B)[ret = (D_800A3748 == 0) * 0x44C] = 0;
            if (D_800A38DC == 3) {
                D_800A3858 = D_800A3858 + 0x384;
                if (D_800A3858 > 0x2BF20) {
                    D_800A3858 = 0x2BF20;
                }
            }
        }
        return;
    }
    if (D_800A3817 == 0) {
        ret = func_8005FA98(0, D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + ((ret / 4) * 4);
    }
    D_800A3929 = D_800A3929 + 1;
    if (((u8)D_800A3929) < 0x3C) return;
    if (D_800A3817 == 0) {
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF = func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 0x44C));
        if (D_8010231A != 0) {
            func_800550E8(1);
        }
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        func_800372C0();
        func_8001DA2C();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if ((double)(D_800A3817 == 2)) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
/* kengo:HIGH  |  nm_cpu/cpu_get_move_pattern_table_number  |  265i  |  -3 near-exact */
void func_8001EEB4(void) {
    s8 idx = D_800A3748;
    u8 *entry = (u8 *)&D_80101EC8 + idx * 0x44C;
    u16 a1 = *(u16 *)(entry + 0x6A);

    if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
        (u32)((s32)a1 - 0x17) >= 2 && *(s16 *)(entry + 0x96) == 0) {
        func_800218C8(D_800A3748);
        {
            s32 ret = func_80021A3C(D_800A3748, *(s16 *)(entry + 0xA));
            *(s16 *)(entry + 0x5E) = 1;
            func_80021A98(D_800A3748, ret, 1);
        }
        *(s16 *)(entry + 0x26C) = 1;
    }

    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x11;
}
void func_8001EFA0(void) {
    s32 sp10[6];
    s16 var_v0;

    D_800A37B8 += 1;
    D_800A3778 = camera_GetBoneData();
    func_8001BCF0((u8 *)&D_80101EC8 + D_800A3748 * 1100, (D_800A37B8 << 12) / 105);
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE08(sp10);
    func_80023F08(0, (s32)sp10);
    func_80023F08(1, (s32)sp10);
    func_8002C61C();
    func_80030D7C();
    myRobGeneiDraw2();
    func_800397A0();
    game_StageInit(1);
    mk_leaf_newpos();

    if (*(&D_80101F5E + D_800A3748 * 550) != 0 && D_800A38DC == 1) {
        D_800A37B8 = 0x69;
    }

    if (D_800A37B8 >= 0x69 || (D_80102794 & 0x400040)) {
        if (D_800A38DC == 4) {
            var_v0 = 0xC;
            goto set_val;
        }
        if (D_800A38DC < 5) {
            if (D_800A38DC == 1) goto handle_1;
            goto default_case;
        }
        if (D_800A38DC == 6) {
            var_v0 = 0xC;
            goto set_val;
        }
        goto default_case;

    handle_1:
        if (D_800A3748 == 0) {
            func_8001DA2C();
            D_800A3768 = 2;
            mottest_disp();
            return;
        }
        var_v0 = 0xC;
        goto set_val;

    default_case:
        func_8001DA2C();
        var_v0 = 2;

    set_val:
        D_800A3834 = var_v0;
    }
}
void func_8001F1C4(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3) {
    s16 temp_v1;
    if (!(*(u8 *)(arg1 + 0x18) & 0x80)) {
        func_80027334((s32 *)arg2);
        func_80027334((s32 *)arg3);
    }
    func_8002F770((s32 *)(arg2 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    func_8002F770((s32 *)(arg3 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    temp_v1 = *(s16 *)(arg0 + 0xC);
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        *(u16 *)(arg2 + 0x7E) = (u16)(*(u16 *)(arg2 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x7E) = (u16)(*(u16 *)(arg3 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
    }
    if ((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) {
        *(u16 *)(arg2 + 0x72) = (u16)(*(u16 *)(arg2 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x72) = (u16)(*(u16 *)(arg3 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
    }
}
INCLUDE_ASM("asm/funcs", md_game_rob_data_init);
/* kengo:HIGH  |  md_game/md_game_rob_data_init  |  351i */
void func_8001F860(s16 *arg0, s32 arg1) {
    arg1 = (arg1 - *(s16 *)((u8 *)arg0 + 0x1CA)) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    *(s16 *)((u8 *)arg0 + 0x14C) = arg1;
}
s32 func_8001F888(void) {
    s32 dx = D_80102408 - D_80101FBC;
    s32 dy = D_80102410 - D_80101FC4;
    s32 s0 = 0;
    while ((u32)(dx + 0x4000) > 0x8000 || (u32)(dy + 0x4000) > 0x8000) {
        s32 t;
        t = dx + ((u32)dx >> 31);
        dx = (s32)t >> 1;
        t = dy + ((u32)dy >> 31);
        dy = (s32)t >> 1;
        s0 += 1;
    }
    {
        s32 v0 = dx * dx;
        s32 v1 = dy * dy;
        s32 r = func_8007E11C(v0 + v1);
        return r << s0;
    }
}
void func_8001F938(u8 *arg0)
{
    u16 kind;
    s32 val;
    s32 a2;
    s32 idx;
    s32 factor;
    kind = *((u16 *)(arg0 + 0x6A));
    a2 = *((s16 *)(arg0 + 0x1C));
    if (kind == 0x11 || kind == 0xF ||
        ((u32)((s32)kind - 0x1C)) < 2U ||
        ((u32)((s32)kind - 0x1E)) < 2U ||
        ((u32)((s32)kind - 0x20)) < 2U ||
        kind == 0xE || kind == 0x2C || kind == 0xD ||
        kind == 0x7 || kind == 0x33 || kind == 0x14)
    {
        goto clamp;
    }
    if (kind == 0x2) { goto rangecheck; }
    if (kind == 0x1B) { goto rangecheck; }
    if (kind == 0x28) { goto rangecheck; }
    if (kind != 0x26) { goto defaultpath; }
rangecheck:
    val = *((s16 *)(arg0 + 0x40));
    if (val < ((s32)(*((u8 *)(arg0 + 0xA1))))) { goto check_outer; }
    if (val > ((s32)(*((u8 *)(arg0 + 0xA3))))) { goto check_outer; }
    goto clamp;
check_outer:
    if (val < ((s32)(*((u8 *)(arg0 + 0xA2))))) { goto multpath_start; }
    if (val > ((s32)(*((u8 *)(arg0 + 0xA4))))) { goto multpath_start; }
clamp:
    *((s16 *)(arg0 + 0x44)) = 0x1000;
    return;
multpath_start:
    if ((*((s16 *)(arg0 + 0x26C))) == 0)
    {
        s32 f = *((s16 *)(arg0 + 0x274));
        a2 = (a2 * f) >> 12;
    }
    {
        s32 probe = *((s16 *)(arg0 + 0x270));
        s32 raw_or_3 = (probe < 4) ? (s32)*((u16 *)(arg0 + 0x270)) : 3;
        idx = ((raw_or_3 << 16) >> 15);
    }
    factor = *((s16 *)((arg0 + 0x276) + idx));
    a2 = (a2 * factor) >> 12;
defaultpath:
    {
        s32 vv0 = *((s16 *)(arg0 + 0x26E));
        s32 vv1 = *((s16 *)(arg0 + 0x272));
        s32 sum = vv0 + vv1;
        s32 sum_or_3 = (sum < 4) ? sum : 3;
        idx = sum_or_3 * 2;
    }
    factor = *((s16 *)((arg0 + 0x27E) + idx));
    a2 = (a2 * factor) >> 12;
    *((s16 *)(arg0 + 0x44)) = (s16)a2;
}

s32 func_8001FAE4(s32 *arg0) {
    s32 v0;
    u16 v1;
    s32 *a0;
    char new_var;

    v1 = *(u16 *)((s32)arg0 + 0xA);
    a0 = (s32 *)((s32)arg0 + 0xA);
    if (v1 == 0) goto ret_zero;
    v0 = v1 & 0x4000;
loop:
    if (v0 != 0) {
        v0 = (s32)a0;
        goto end;
    }
    new_var = 0;
    if ((v1 & 0xC000) != new_var) {
        a0 = (s32 *)((s32)a0 + 8);
    } else {
        a0 = (s32 *)((s32)a0 + 4);
    }
    v1 = *(u16 *)a0;
    v0 = v1 & 0x4000;
    if (v1 != new_var) goto loop;
ret_zero:
    v0 = 0;
end:
    return v0;
}
s32 func_8001FB34(s32 *arg0, s32 arg1) {
    s16 v1;
    s32 v0;
    v1 = D_800A38DC;
    if (v1 == 2) return 0;
    if (v1 == 5) return 0;
    if (v1 == 3) return 0;
    if (v1 != 0) goto check2;
    if (D_800A385C != 0) return 0;
check2:
    v0 = *(s32 *)arg0;
    v1 = *(s16 *)(v0 + 0xC);
    if (v1 == 0xD) return 0;
    if (v1 == 0x1C) return 0;
    v1 = *(s16 *)((u8 *)arg0 + 0xA);
    if (v1 != 0xE) goto check3;
    if (*(s16 *)((u8 *)arg0 + 0x330) == 0) return 0;
    v1 = *(s16 *)((u8 *)arg0 + 0x332);
    if (v1 == 0xA) goto check3;
    return 0;
check3:
    v0 = 1;
    if (arg1 != 0) {
        v0 = *(s16 *)((u8 *)arg0 + 0x26C);
        v0 = (u32)0 < (u32)v0;
    }
    return v0;
}
INCLUDE_ASM("asm/funcs", single_game_CheckStatusUpDataTotalOver);
/* kengo:HIGH  |  nm_single_game/single_game_CheckStatusUpDataTotalOver  |  289i */
s32 func_8002006C(void) {
    s32 s0 = D_800A387C;
    s32 v0 = func_8001F888();
    s32 v = D_800A38DC;
    D_800A387C = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        single_game_CheckStatusUpDataTotalOver();
    }
    D_800A38A8 = 0;
}
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 dx = arg1[0] - arg0[0];
    s32 dz = arg1[2] - arg0[2];
    s32 dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy = arg1[1] - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 disc = arg2 * arg2 + arg3 * dy2;
            s32 a0;

            if (disc >= 0) {
                s32 sq = func_8007E11C(disc << 10);
                s32 a2 = arg2 << 5;
                a0 = ((a2 + sq) * dist) / dy2 / 32;

                if (a0 < 0) {
                    a0 = ((a2 - sq) * dist) / dy2 / 32;
                }
            } else {
                a0 = 300;
            }

            if (a0 >= 301) {
                a0 = 300;
            }

            arg4[0] = (a0 * dx) / dist;
            arg4[2] = (a0 * dz) / dist;
        }
    }
}
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 sp_matrix[8];
    s32 vec[3];

    *(s16 *)(arg0 + 0x350) = 1;
    *(u16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    {
        s32 ret = game_GetPlayerData(*(s16 *)(arg0 + 0x4));
        s16 idx = *(s16 *)(arg0 + 0x352);
        func_8002EECC(*(s32 *)(ret + idx * 4), sp_matrix);
    }
    /* Set GTE rotation matrix from sp_matrix */
    {
        register s32 *v0 asm("v0") = sp_matrix;
        __asm__ volatile (
            ".word 0x00406021\n"   /* addu $t4, $v0, $zero */
            "\t.word 0x8D8D0000\n" /* lw   $t5, 0($t4) */
            "\t.word 0x8D8E0004\n" /* lw   $t6, 4($t4) */
            "\t.word 0x48CD0000\n" /* ctc2 $t5, $0 */
            "\t.word 0x48CE0800\n" /* ctc2 $t6, $1 */
            "\t.word 0x8D8D0008\n" /* lw   $t5, 8($t4) */
            "\t.word 0x8D8E000C\n" /* lw   $t6, 0xC($t4) */
            "\t.word 0x8D8F0010\n" /* lw   $t7, 0x10($t4) */
            "\t.word 0x48CD1000\n" /* ctc2 $t5, $2 */
            "\t.word 0x48CE1800\n" /* ctc2 $t6, $3 */
            "\t.word 0x48CF2000\n" /* ctc2 $t7, $4 */
            :: "r"(v0)
        );
    }
    /* Load translation vector from arg2 */
    vec[0] = (s32)arg2[0];
    vec[1] = (s32)arg2[1];
    vec[2] = (s32)arg2[2];
    /* Load GTE vector (VXY0, VZ0), run MVMVA */
    __asm__ volatile (
        ".word 0x00406021\n"   /* addu $t4, $v0, $zero */
        "\t.word 0x958E0004\n" /* lhu  $t6, 4($t4) */
        "\t.word 0x958D0000\n" /* lhu  $t5, 0($t4) */
        "\t.word 0x000E7400\n" /* sll  $t6, $t6, 16 */
        "\t.word 0x01AE6825\n" /* or   $t5, $t5, $t6 */
        "\t.word 0x488D0000\n" /* mtc2 $t5, $0 (VXY0) */
        "\t.word 0xC9810008\n" /* lwc2 $1, 8($t4) (VZ0) */
        "\tnop\n"
        "\tnop\n"
        "\t.word 0x4A486012\n" /* mvmva 1,0,0,3,0 (rtv0) */
        :: "r"(vec)
    );
    /* Store GTE MAC1/MAC2/MAC3 results to arg0+0x354 */
    arg0 += 0x354;
    __asm__ volatile (
        ".word 0x02006021\n"   /* addu $t4, $s0, $zero */
        "\t.word 0xE9990000\n" /* swc2 $25, 0($t4) (MAC1) */
        "\t.word 0xE99A0004\n" /* swc2 $26, 4($t4) (MAC2) */
        "\t.word 0xE99B0008\n" /* swc2 $27, 8($t4) (MAC3) */
        :: "r"(arg0)
    );
}
void single_game_SetAbilityData(u8 *arg0) {
    long temp_s1;
    s32 sp10[3];
    s16 sp20[3];
    s32 tx, ty, tz;
    s32 mul;
    s16 idx;
    s32 pd;

    temp_s1 = *(s16 *)(arg0 + 0x4);
    if (*(s16 *)(arg0 + 0x350) != 0) {
        *(u16 *)(arg0 + 0x350) += 1;
        if ((*(u16 *)(arg0 + 0x350) & 7) == 2) {
            pd = game_GetPlayerData(temp_s1);
            idx = *(s16 *)(arg0 + 0x352);
            {
                register s32 *v1 asm("v1") = (s32 *)*(s32 *)(pd + idx * 4);
                __asm__ volatile (
                    "nop\n"
                    "\t.word 0x00606021\n"
                    "\t.word 0x8D8D0000\n"
                    "\t.word 0x8D8E0004\n"
                    "\t.word 0x48CD0000\n"
                    "\t.word 0x48CE0800\n"
                    "\t.word 0x8D8D0008\n"
                    "\t.word 0x8D8E000C\n"
                    "\t.word 0x8D8F0010\n"
                    "\t.word 0x48CD1000\n"
                    "\t.word 0x48CE1800\n"
                    "\t.word 0x48CF2000\n"
                    :: "r"(v1)
                );
            }
            {
                register s32 *v0 asm("v0") = (s32 *)(arg0 + 0x354);
                __asm__ volatile (
                    ".word 0x00406021\n"
                    "\t.word 0x958E0004\n"
                    "\t.word 0x958D0000\n"
                    "\t.word 0x000E7400\n"
                    "\t.word 0x01AE6825\n"
                    "\t.word 0x488D0000\n"
                    "\t.word 0xC9810008\n"
                    "\tnop\n"
                    "\tnop\n"
                    "\t.word 0x4A486012\n"
                    :: "r"(v0)
                );
            }
            { s32 cc = *(s16 *)(arg0 + 0x350); mul = ((0x96 - cc) << 12) / 150; do {} while(0); }
            {
                register s32 *v0 asm("v0") = sp10;
                __asm__ volatile (
                    ".word 0x00406021\n"
                    "\t.word 0xE9990000\n"
                    "\t.word 0xE99A0004\n"
                    "\t.word 0xE99B0008\n"
                    :: "r"(v0)
                );
            }
            tx = (sp10[0] * mul) / 0x1000;
            sp20[0] = tx;
            ty = (sp10[1] * mul) / 0x1000;
            sp20[1] = ty;
            tz = (sp10[2] * mul) / 0x1000;
            sp20[2] = tz;
            if ((s16)ty >= 0x801) {
                sp20[0] = -tx;
                sp20[1] = -ty;
                sp20[2] = -tz;
            }
            func_80032854(temp_s1, 4, (u8 *)0x1F8000A8 + temp_s1 * 0x108 + *(s16 *)(arg0 + 0x352) * 0xC, sp20);
        }
        if (*(s16 *)(arg0 + 0x350) >= 0x96) {
            *(s16 *)(arg0 + 0x350) = 0;
        }
        *(s16 *)(arg0 + 0x350) = 0;
    }
}

/* kengo:HIGH  |  nm_single_game/single_game_SetAbilityData  |  124i */
void func_800206B0(s32 arg0, s32 arg1) {
    u8 *a3 = (u8 *)&D_8008D59C;
    u8 *a2 = (u8 *)&D_800F5F68 + arg0 * 0x1B8;
    s32 t0 = 0;
    u8 *a0 = a2 + 0x12;
    u8 *v1 = a3 + 0x12;

loop:
    *(u16 *)a2 = *(u16 *)a3;
    *(u16 *)(a0 - 0x10) = *(u16 *)(v1 - 0x10);
    *(s16 *)(a0 - 0xE) = (s32)(*(s16 *)(v1 - 0xE) * arg1) >> 0xC;
    *(s16 *)(a0 - 0xC) = (s32)(*(s16 *)(v1 - 0xC) * arg1) >> 0xC;
    *(s16 *)(a0 - 0xA) = (s32)(*(s16 *)(v1 - 0xA) * arg1) >> 0xC;
    *(s16 *)(a0 - 6) = (s32)(*(u16 *)(v1 - 6) * arg1) >> 0xC;
    *(s16 *)(a0 - 4) = (s32)(*(u16 *)(v1 - 4) * arg1) >> 0xC;
    *(s16 *)(a0 - 2) = (s32)(*(u16 *)(v1 - 2) * arg1) >> 0xC;
    {
        s32 temp_lo = *(u16 *)v1 * arg1;
        t0 += 1;
        a3 += 0x14;
        a2 += 0x14;
        v1 += 0x14;
        *(s16 *)a0 = temp_lo >> 0xC;
        a0 += 0x14;
    }
    if (t0 < 0x16) goto loop;
}
INCLUDE_ASM("asm/funcs", func_800207C8);
void func_80020CDC(void) {
    if (D_800A38C6 == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    D_800A38C6 = 0;
    D_800A38C4 = 0;
    D_800A38C1 = 0xFF;
    D_800A38C0 = 0xFF;
}
void func_80020D38(void) {
    if (D_800A38C6 == 0xFFFF) {
        seq_Reset();
    }
    D_800A38C6 = 0;
}

void func_80020D70(void) {
    D_800A3888 = (s32)0x80118800;
    D_800A388C = (s32)0x8011C400;
    D_800A3830 = (s32)0x80120000;
    D_800A3860 = (s32)0x80148800;
    D_800A3864 = (s32)0x80190800;
    func_80020CDC();
}
void func_80020DDC(void) {    s32 v0;    s32 v1;    s32 v2;    v0 = func_80036EA8(1, 1);    replay_camera_Init(v0, D_800A3830);    game_FrameLoop();    v1 = D_800A3830;    D_80102760 = v1 + 0x14;    D_80102764 = v1 + *(s32 *)(v1 + 4);    D_80102768 = v1 + *(s32 *)(v1 + 8);    v2 = *(s32 *)(v1 + 0x10);    D_800A3880 = 1;    D_80102770 = v1 + v2;}
INCLUDE_ASM("asm/funcs", DispPracticeMenuTex_B);
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
void func_80021210(void) {
    func_8001979C(0, D_80102770);
    if (D_800A38C4) {
        func_8001979C(1, D_801027C0);
    }
    if (D_800A38C6) {
        func_8001979C(2, D_801027D4);
    }
}
void func_80021280(s32 a0) {
    s32 a1 = 0;
    u8 *a2 = (u8 *)&D_80101EC8 + a0 * 1100;
    u16 a3 = *(u16 *)(a2 + 0x48);
    u16 *v1 = (u16 *)&D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = *(u16 *)(a2 + 0x48);
        *(s16 *)(a2 + 0x4A) = a1;
        *(s16 *)(a2 + 0x4C) = 0;

        if ((u32)(val >> 12) < 2) {
            s32 i = 0;
            u16 t1 = val;
            s32 t4 = 4;
            s32 t3 = 3;
            s32 t2 = 1;
            s32 mode = D_800A38DC;
            u8 t0 = D_800A384C;

        loop2_21280:
            {
                u16 nibble = (t1 >> (i << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                *(s16 *)(a2 + 0x88) = i;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                *(u16 *)(a2 + 0x8A) = *(u16 *)(a2 + 0x26C);
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                *(s16 *)(a2 + 0x8E) = i;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) goto next_21280;
            store5_21280:
                *(u16 *)(a2 + 0x90) = *(u16 *)(a2 + 0x26C);
            }
        next_21280:
            i++;
            if (i < 3) goto loop2_21280;
        }
    }
}
void func_800213A0(s16 *arg0) {
    s16 a1 = arg0[0x86 / 2];
    if (a1 != arg0[0x88 / 2]) {
        if (a1 != arg0[0x8E / 2]) {
            return;
        }
    }
    {
        s16 *v = (s16 *)(&D_800A3860)[arg0[0x4A / 2]];
        arg0[0x86 / 2] = (s16)((a1 + 1) % v[0x14 / 2]);
    }
}
void *func_80021424(s32 arg0, u16 arg1, s32 arg2) {
}
void func_800218C8(s32 a0) {
    s32 offset = a0 * 1100;
    *(u16 *)((u8 *)&D_80101F4E + offset) = *(u16 *)((u8 *)&D_80101F4C + offset);
}
s32 func_80021904(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4E + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
s32 func_80021974(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4C + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
s32 func_800219E4(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + 0x16);
    return D_80102760 + idx * 2;
}
s32 func_80021A3C(s32 a0, s32 a1) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + a1 * 2 + 0x18);
    return D_80102760 + idx * 2;
}
void func_80021A98(s32 arg0, u8 *arg1, s32 arg2) {
    u8 *s0 = ((u8 *) (&D_80101EC8)) + (arg0 * 1100);
    s32 a3;
    if ((*((s16 *) (s0 + 0x4C))) != 0) {
        a3 = *((s16 *) ((*((s32 *) s0)) + 0x4A));
    } else {
        a3 = *((s16 *) (s0 + 0x4A));
    }
    *((s16 *) (s0 + 0x4C)) = 0;
    *((s32 *) (s0 + 0x50)) = (s32) arg1;
    {
        u16 v1 = *((u16 *) (arg1 + 4));
        *((s16 *) (s0 + 0x5C)) = v1;
        if (arg2 != 0) {
            s32 v0 = D_80102764 + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v1 = *((u16 *) (v0 + 2));
            v0 = D_80102768 + v1;
            *((s32 *) (s0 + 0x58)) = v0;
        } else {
            s32 idx = a3 * 5;
            s32 v0 = (&D_801027B4)[idx] + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v1 = *((u16 *) (v0 + 2));
            v0 = (&D_801027B8)[idx] + v1;
            *((s32 *) (s0 + 0x58)) = v0;
        }
    }
    {
        s32 v0_50 = *((s32 *) (s0 + 0x50));
        u16 old_kind = *((u16 *) (s0 + 0x6A));
        s32 a0_58 = *((s32 *) (s0 + 0x58));
        *((u8 *) (s0 + 0x60)) = (u8) arg2;
        do { } while (0);
        *((u8 *) (s0 + 0x61)) = (u8) a3;
        {
            u8 a1_val = *((u8 *) (v0_50 + 6));
            *((s16 *) (s0 + 0x6C)) = old_kind;
            {
                s32 v1_58 = *((s32 *) (s0 + 0x58));
                s32 li1 = 1;
                *((s16 *) (s0 + 0x42)) = 0;
                *((s16 *) (s0 + 0x7A)) = li1;
                *((s32 *) (s0 + 0x7C)) = 0;
                *((s16 *) (s0 + 0x46)) = 0;
                *((s16 *) (s0 + 0x40)) = a1_val;
                *((s16 *) (s0 + 0x6A)) = *((u8 *) a0_58);
                *((s16 *) (s0 + 0x6E)) = *((u8 *) (v1_58 + 2));
            }
        }
        {
            s32 v0_50b = *((s32 *) (s0 + 0x50));
            s32 kind = *((u16 *) (s0 + 0x6A));
            *((s16 *) (s0 + 0x70)) = (*((u8 *) (v0_50b + 9))) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) || (kind == 0x26)) {
                    a0_flag = 1;
                }
                *((u8 *) (s0 + 0xAD)) = a0_flag;
            }
            cpu_get_dist_2(s0);
            {
                s32 kind2 = *((u16 *) (s0 + 0x6A));
                s32 v1k = kind2 & 0xFFFF;
                if (v1k == 9) {
                    *((s16 *) (s0 + 0x152)) = 1;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1CA));
                    goto end;
                }
                if (v1k == 2) {
                    if ((*((s16 *) (s0 + 0x152))) != 0) goto clear_152;
                    if ((*((s16 *) (s0 + 0x6C))) == 0x13) goto clear_152;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                    goto clear_152;
                }
                if (((u32) (kind2 - 0x19)) >= 2U) goto not_in_range;
                if ((*((s16 *) (s0 + 0x152))) == 0) goto set_154;
                if (v1k != 0x19) goto set_152;
                if ((*((s16 *) (s0 + 0x6C))) != v1k) goto set_152;
                goto set_154;
                set_154:
                *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                goto set_152;
                not_in_range:
                if (v1k != 0x11) goto clear_152;
                set_152:
                *((s16 *) (s0 + 0x152)) = 1;
                goto end;
                clear_152:
                *((s16 *) (s0 + 0x152)) = 0;
            }
            end:
            {
                u16 v1f = *((u16 *) (s0 + 0x6A));
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) || (v1f == 0x26)) {
                    *((u8 *) (s0 + 0xAF)) = ((*((u8 *) (s0 + 0xB0))) & 0xF) != 5;
                }
            }
        }
    }
}
void func_80021D10(s32 arg0, s32 *arg1, s32 arg2) {
    s16 *temp_v0;
    temp_v0 = (s16 *)(stage_GetDataPtr() + (((D_800A36A4 * 0x18) + (arg2 * 6) + (arg0 * 3)) * 2));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}
INCLUDE_ASM("asm/funcs", func_80021DB0);
void func_80022224(s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[6];
    s16 *base;
    s16 *p;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 *r;
    s32 *w;

    base = (s16 *)(stage_GetDataPtr() + (D_800A36A4 * 3) * 0x10);
    i = 0;
    p = base;
    do {
        dx = p[3] - arg2[0];
        dz = p[5] - arg2[2];
        dists[i] = dx * dx + dz * dz;
        i++;
        p += 6;
    } while (i < 4);

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] < dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[best] < dists[i]) {
            best = i;
        }
    }
    dists[best] = -1;

    r = dists;
    w = dists;
    for (i = 0; i < 4; i++) {
        if (*r != -1) {
            w[4] = i;
            w++;
        }
        r++;
    }

    base += dists[4 + (func_80079154() & 1)] * 6 + 3;
    arg1[0] = base[0];
    arg1[1] = base[1];
    arg1[2] = base[2];
}
s32 func_80022408(s32 *arg0) {
    s16 *p;
    s32 i;
    s32 best_dist;
    s32 best;
    s32 t1;
    s32 t2;
    int new_var;
    s32 dx;
    s32 dz;
    s32 dist;
    p = (s16 *)stage_GetDataPtr();
    best_dist = 0x7FFFFFFF;
    i = 0;
    t1 = arg0[0];
    t2 = arg0[2];
    p = p + ((D_800A36A4 * 3) * 8);
loop:
    dx = ((p[0] + p[3]) / 2) - t1;
    new_var = dx * dx;
    dist = new_var;
    dz = ((p[2] + p[5]) / 2) - t2;
    dist = dist + (dz * dz);
    if (dist < best_dist) {
        best_dist = dist;
        best = i;
    }
    i++;
    if (i < 4) {
        p += 6;
        goto loop;
    }
    return best;
}
s32 func_800224E0(s32 *arg0) {
    u8 *new_var2;
    u8 *p;
    u8 *end;
    u8 *base;
    u32 pad;
    s32 val;
    s32 i;
    s32 *new_var;
    s32 *ptr;

    p = (&D_8008EB1C) + (D_800A384C * 2);
    new_var2 = &D_8008DB1C;
    new_var = (s32 *)(*arg0);
    if ((!i) && (!i) && (!i)) {
    }
    end = p + 2;
    ptr = new_var;
    base = new_var2 + (*(s16 *)((u8 *)ptr + 0xA) * 16);
    val = *(u16 *)(base + *(s16 *)((u8 *)ptr + 0xE) * 2);
loop:
    i = 0;
    do {
        if (*p == ((val >> (i * 4)) & 0xF)) {
            return i;
        }
        i++;
        if ((val && val) && val) {
        }
    } while (i < 3);
    p++;
    if ((s32)p < (s32)end) {
        goto loop;
    }
    return 0;
}
void func_80022568(s16 *arg0) {
    arg0[0x136] = 1;
    arg0[0x137] = 0;
    arg0[0x138] = 0;
    arg0[0x139] = 0;
}
void func_80022580(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
}
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(a0 + 6) != 0) {
            s32 val = D_800A38DC;

            switch (val) {
                case 0:
                    *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(a0 + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s32 val1 = (&D_801027BC)[idx1 * 5];
                a0 = *(u8 **)a0;
                {
                    s16 idx2 = *(s16 *)(a0 + 0x4A);
                    func_80055138(i, val1, (&D_801027BC)[idx2 * 5]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}

typedef struct { s32 a, b, c, d; } Quad_2304C;

void func_8002304C(u8 *obj, s32 *pos1, s32 *pos2, s32 *arg3)
{
  s32 *scratch = (s32 *) 0x1F8001B0;
  register s32 *scratch_c asm("s4") = (s32 *) 0x1F8001C0;
  register s16 *scratch_d asm("s6") = (s16 *) 0x1F8001D0;
  register s32 count asm("s5") = 0;
  loop:
  if (((pos1[0] != pos2[0]) || (pos1[1] != pos2[1])) || (pos1[2] != pos2[2]))
  {
    if (func_8005344C(pos1, pos2, scratch, scratch_c, 0x1F8002B8) == 0)
    {
      *(Quad_2304C *)pos1 = *(Quad_2304C *)pos2;
      goto done;
    }
    *((s8 *) (obj + 0xB1)) = (s8) func_80054434();
    *(Quad_2304C *)pos1 = *(Quad_2304C *)scratch;
    DispSchoolBG((s16 *) arg3, (s16 *) scratch_c, arg3, -0x40, 0xE6);
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      scratch[12] = pos1[0] + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      scratch[13] = pos1[1] + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      scratch[14] = pos1[2] + (vel / 1024);
    }
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      pos2[0] += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      pos2[1] += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      pos2[2] += vel / 1024;
    }
    if (func_8005344C(pos1, scratch + 12, scratch, scratch + 6, 0x1F8002B8) == 0)
    {
      *(Quad_2304C *)pos1 = *(Quad_2304C *)(scratch + 12);
      scratch[8] = pos2[0] - pos1[0];
      scratch[9] = pos2[1] - pos1[1];
      scratch[10] = pos2[2] - pos1[2];
      {
        s32 step;
        if ((*((u16 *) (obj + 0x6A))) == 0x15)
        {
          step = 0x80;
        }
        else
        {
          step = 0x100;
        }
        DispSchoolBG(scratch_d, (s16 *) scratch_c, (s32 *) scratch_d, 0, step);
      }
      {
        s16 vel_y = *((s16 *) (((u8 *) scratch) + 0x12));
        if (vel_y >= (-0x7FF))
        {
          u16 mode = *((u16 *) (obj + 0x6A));
          s32 m = mode & 0xFFFF;
          if (((((m != 8) && (m != 0x22)) && (((u32) (mode - 0x17)) >= 2)) && (m != 0xA)) && ((*((s16 *) (obj + 0x72))) == 0))
          {
            scratch[9] = 0;
          }
        }
      }
      pos2[0] = pos1[0] + scratch[8];
      pos2[1] = pos1[1] + scratch[9];
      count++;
      pos2[2] = pos1[2] + scratch[10];
      if (count < 4)
      {
        goto loop;
      }
    }
  }

  done:
  ;

  ;
}
typedef struct { s32 a; s32 b; } __attribute__((packed)) PackedPair_233AC;

s32 func_800233AC(u8 *arg0, s32 *arg1) {
    s32 pos[3];
    s32 off[3];
    s16 out1[4];
    s32 out2[4];
    u32 bits;
    s32 a1_idx;
    s32 a0_idx;
    s16 *judge_ptr;

    bits = *(u32 *)(arg0 + 0x2C);
    a1_idx = (bits >> 14) & 1;
    if (!(bits & 0x1000)) {
        a1_idx++;
    }
    a0_idx = (bits >> 15) & 1;
    if (!(bits & 0x2000)) {
        a0_idx++;
    }

    {
        s16 *tbl = &D_8008EB40;
        s32 px;
        s16 *row;
        s32 a1_val;

        __asm__ volatile("" : "=r"(tbl) : "0"(tbl));

        px = *(s32 *)(arg0 + 0xB8);
        row = tbl + a0_idx * 3;
        a1_val = row[a1_idx];

        pos[0] = px;
        pos[1] = *(s32 *)(arg0 + 0xBC) - 0x64;
        pos[2] = *(s32 *)(arg0 + 0xC0);

        {
            s32 angle = (*(s16 *)(arg0 + 0x1D8) + a1_val) & 0xFFF;
            s16 jv = (&Judge)[angle];
            judge_ptr = &Judge;

            off[0] = px + jv / 4;
            off[1] = pos[1];
        }

        {
            s32 angle2 = (*(s16 *)(arg0 + 0x1D8) + a1_val + 0x400) & 0xFFF;
            s16 jv2 = judge_ptr[angle2];

            off[2] = pos[2] + jv2 / 4;
        }
    }

    if (func_80053614(pos, off, out2, (s32 *)out1, (s32)0x1F8002B8) == 0) {
        return 0;
    }

    *(PackedPair_233AC *)(arg0 + 0x98) = *(PackedPair_233AC *)out1;

    {
        s32 fwd_angle = func_8007FD5C(out1[0], out1[2]);
        s32 fwd_800;

        pos[0] = *(s32 *)(arg0 + 0xB8);
        pos[1] = *(s32 *)(arg0 + 0xBC) - 0x898;
        pos[2] = *(s32 *)(arg0 + 0xC0);

        fwd_800 = fwd_angle + 0x800;

        {
            s32 angle3 = fwd_800 & 0xFFF;
            s16 jv3 = judge_ptr[angle3];
            off[0] = pos[0] + jv3 / 8;
        }

        off[1] = pos[1];

        {
            s32 angle4 = (fwd_angle + 0xC00) & 0xFFF;
            s16 jv4 = judge_ptr[angle4];
            off[2] = pos[2] + jv4 / 8;
        }

        if (func_80053614(pos, off, out2, (s32 *)out1, (s32)0x1F8002B8) != 0) {
            return 0;
        }

        pos[0] = off[0];
        pos[1] = off[1] + 0x190;
        pos[2] = off[2];

        if (func_80053614(off, pos, out2, (s32 *)out1, (s32)0x1F8002B8) == 0) {
            return 0;
        }

        *arg1 = fwd_800;
        return 1;
    }
}
void func_80023648(u8 *arg0) {
    u16 kind = *(u16 *)(arg0 + 0x6A);
    s16 *new_var;

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        u32 bits = *(u32 *)(arg0 + 0x2C);
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;
            s32 a2;
            s16 *row;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            new_var = &D_8008EB40;
            row = new_var + (a0 * 3);
            a2 = row[a1];

            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1CA) + a2 / 4);
            } else {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1D8) + a2);
            }
        } else {
            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
        }

        {
            s32 abs_val = *(s16 *)(arg0 + 0x150);
            if (abs_val < 0) {
                abs_val = -abs_val;
            }
            if (abs_val >= 0x401) {
                abs_val = 0x400;
            }

            {
                s32 sub_result = *(u16 *)(arg0 + 0x14E) - abs_val;
                s32 div16 = *(s16 *)(arg0 + 0x1A);
                s16 new_14e;
                s32 tbl_val;
                s32 mult_res;
                s32 limit;

                *(s16 *)(arg0 + 0x14E) = sub_result;
                if (div16 < 0) {
                    div16 += 15;
                }
                div16 >>= 4;
                new_14e = sub_result;
                new_14e = new_14e + div16;
                *(s16 *)(arg0 + 0x14E) = new_14e;

                tbl_val = (&D_800A310C)[(&D_8008DA08)[*(s16 *)(arg0 + 0xA)]];
                sub_result = *(s16 *)(arg0 + 0x1A);
                mult_res = sub_result * tbl_val;
                limit = (mult_res << 4) >> 12;

                if (limit < (s16)new_14e) {
                    *(s16 *)(arg0 + 0x14E) = limit;
                } else if ((s16)new_14e < 0) {
                    *(s16 *)(arg0 + 0x14E) = 0;
                }

                {
                    s32 speed_prod = *(s16 *)(arg0 + 0x14E) * *(s16 *)(arg0 + 0x44);
                    s32 speed = speed_prod >> 12;
                    s16 sin_val = (&Judge)[(*(u16 *)(arg0 + 0x1CA) & 0xFFF)];

                    *(s32 *)(arg0 + 0xD8) += (sin_val * speed) >> 16;

                    {
                        s16 cos_val = (&Judge)[((*(s16 *)(arg0 + 0x1CA) + 0x400) & 0xFFF)];
                        *(s32 *)(arg0 + 0xE0) += (cos_val * speed) >> 16;
                    }
                }
            }
        }
    } else {
        if (*(s16 *)(arg0 + 0x14E) > 0) {
            if (kind != 0x22) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
            *(s16 *)(arg0 + 0x14E) = 0;
        }
    }
}
int camera_set_zoom(u8 *arg0)
{
    s32 src[4];
    s32 dst[4];
    s32 out[4];
    s16 offsets[4];
    s16 offsets2[4];
    s32 s1;
    s32 dx_delta;
    s32 dz_delta;
    s32 scratchpad;
    s32 ok;
    if ((*((s32 *) (arg0 + 0x108))) <= 0) {
        return;
    }
    {
        s32 kind = *((u16 *) (arg0 + 0x6A));
        if (((u32) (kind - 0x17)) < 2u) {
            return;
        }
        if ((kind & 0xFFFF) == 0xA) {
            return;
        }
        if ((*((s16 *) (arg0 + 0x72))) != 0) {
            return;
        }
        if ((kind & 0xFFFF) == 1) {
            return;
        }
        if ((kind & 0xFFFF) == 0x28) {
            return;
        }
    }
    scratchpad = 0x1F8002B8;
    src[0] = *((s32 *) (arg0 + 0xB8));
    src[1] = (*((s32 *) (arg0 + 0xBC))) - 0xC8;
    src[2] = *((s32 *) (arg0 + 0xC0));
    dst[0] = *((s32 *) (arg0 + 0xB8));
    dst[1] = (*((s32 *) (arg0 + 0xBC))) + 0x514;
    dst[2] = *((s32 *) (arg0 + 0xC0));
    s1 = func_80053614(src, dst, out, (s32 *) offsets, scratchpad);
    ok = 1;
    if (s1 != 0)
    {
        if ((out[1] - src[1]) >= 0x191)
        {
            ok = (*((u16 *) (arg0 + 0x6A))) != 0x22;
            goto ok_check;
        }
        if (offsets[1] < (-0x7FF))
        {
            goto ok_zero;
        }
        {
            s32 tmp;
            tmp = offsets[0];
            if (tmp < 0) {
                tmp += 7;
            }
            dst[0] += tmp >> 3;
            tmp = offsets[2];
            if (tmp < 0) {
                tmp += 7;
            }
            dst[2] += tmp >> 3;
            ok = func_80053614(src, dst, out, (s32 *) offsets2, scratchpad) == 0;
            goto ok_check;
        }
    }
    goto ok_check;
    ok_zero:
    ok = 0;
    ok_check:
    if (!ok) {
        return;
    }
    if (s1 == 0)
    {
        dx_delta = (*((s32 *) (arg0 + 0xB8))) - (*((s32 *) (arg0 + 0xC8)));
        offsets[0] = dx_delta;
        dx_delta = (s16) dx_delta;
        offsets[2] = (*((s32 *) (arg0 + 0xC0))) - (*((s32 *) (arg0 + 0xD0)));
        if (dx_delta < (-0x40)) {
            offsets[0] = -0x40;
        }
        else if (dx_delta >= 0x41) {
            offsets[0] = 0x40;
        }
        dz_delta = (s16) offsets[2];
        if (dz_delta < (-0x40)) {
            offsets[2] = -0x40;
        }
        else if (dz_delta >= 0x41) {
            offsets[2] = 0x40;
        }
        dx_delta = offsets[0];
        dz_delta = offsets[2];
    }
    else
    {
        dx_delta = offsets[0];
        if (dx_delta < 0) {
            dx_delta += 0x3F;
        }
        dz_delta = offsets[2];
        dx_delta >>= 6;
        if (dz_delta < 0) {
            dz_delta += 0x3F;
        }
        dz_delta >>= 6;
    }
    {
        s32 kind;
        s1 = ((*((s16 *) (arg0 + 0x1CA))) - func_8007FD5C(offsets[0], offsets[2])) & 0xFFF;
        if (s1 >= 0x800) {
            s1 = 0x1000 - s1;
        }
        kind = *((u16 *) (arg0 + 0x6A));
        if (((((kind & 0xFFFF) == 0xF) || (((u32) (kind - 0x1C)) < 2u)) || (((u32) (kind - 0x1E)) < 2u)) || (((u32) (kind - 0x20)) < 2u))
        {
            if (s1 < 0x400) {
                *((s16 *) (arg0 + 0x286)) = 0;
                *((s16 *) (arg0 + 0x94)) = 0;
            }
            else {
                *((s16 *) (arg0 + 0x286)) = 1;
                *((s16 *) (arg0 + 0x94)) = 1;
            }
            *((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;
        }
        else if ((kind & 0xFFFF) == 0x11)
        {
            D_800A3769 = s1 < 0x400;
            D_800A3758 = *((u16 *) (arg0 + 4));
            goto skip_74;
        }
        else if (s1 < 0x400)
        {
            *((s16 *) (arg0 + 0x286)) = 0x12;
            *((s16 *) (arg0 + 0x94)) = 0;
        }
        else
        {
            *((s16 *) (arg0 + 0x286)) = 0x11;
            *((s16 *) (arg0 + 0x94)) = 1;
        }
        *((s32 *) (arg0 + 0x74)) = *((s32 *) (arg0 + 0xBC));
    }
    skip_74:
    *((s32 *) (arg0 + 0x104)) += dx_delta;
    *((s32 *) (arg0 + 0x10C)) += dz_delta;
}
/* kengo:HIGH  |  nm_camera/camera_set_zoom  |  219i */
void func_80023C30(s32 arg0, s32 arg1, s32 arg2, s16 *arg3) {
    arg3[0] = 0x1000;
    arg3[1] = 0;
    arg3[2] = 0;
    arg3[3] = 0;
    arg3[4] = 0x1000;
    arg3[5] = 0;
    arg3[6] = 0;
    arg3[7] = 0;
    arg3[8] = 0x1000;
    func_8007F87C(arg0, (s32)arg3);
    func_8007FA1C(arg1, (s32)arg3);
    func_8007FBBC(arg2, (s32)arg3);
}
void func_80023CB4(s16 *arg0, s16 arg1) {
    s16 v;
    *(u16 *)((u8 *)arg0 + 0x31A) += 1;
    v = *(s16 *)((u8 *)arg0 + 0x31A);
    if (v == 1) {
        *(s16 *)((u8 *)arg0 + 0x318) = arg1;
        *(s32 *)((u8 *)arg0 + 0x320) = 0;
        *(s32 *)((u8 *)arg0 + 0x328) = 0;
    }
    *(s16 *)((u8 *)arg0 + 0x31C) = arg1;
    if (*(s16 *)((u8 *)arg0 + 0x152) == 0) {
        *(s16 *)((u8 *)arg0 + 0x152) = 1;
        *(s16 *)((u8 *)arg0 + 0x154) = *(u16 *)((u8 *)arg0 + 0x1D8);
    }
}
void func_80023D08(s32 arg0) {
    func_80023CB4(arg0, 0x200);
}
void func_80023D28(u8 *arg0) {
    if (*(s32 *)(arg0 + 0x108) < 0) {
        *(s16 *)(arg0 + 0x1DC) = 0;
        return;
    }
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 0x1F4;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    *(s16 *)(arg0 + 0x1DC) = func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8);
}
s32 func_80023DB8(u8 *arg0) {
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 5;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    if (func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8) != 0) {
        if ((arg0 && arg0) && arg0) {
        }
        return *(s16 *)0x1F8001C2 < -0x800;
    }
    return 0;
}
void func_80023E40(u8 *arg0) {
    s32 *s1 = (s32 *)0x1F8001B0;
    s32 a0;
    s32 v1;
    a0 = *(u16 *)(arg0 + 0x6A);
    v1 = a0 & 0xFFFF;
    if (v1 == 8) goto done;
    if (v1 == 0x22) goto done;
    if ((u32)(a0 - 0x17) < 2 || v1 == 0x28 || v1 == 0xA) {
        s32 v0;
        s32 *v3 = (s32 *)0x1F8002B8;
        s1[0xC] = *(s32 *)(arg0 + 0xB8);
        s1[0xD] = *(s32 *)(arg0 + 0xBC) + 0x1F40;
        s1[0xE] = *(s32 *)(arg0 + 0xC0);
        v0 = func_80053614((s32 *)(arg0 + 0xB8), &s1[0xC], s1, &s1[4], (s32)v3);
        if (v0 == 0) goto done;
        *(s32 *)(arg0 + 0x148) = s1[1];
        goto done;
    }
    *(s32 *)(arg0 + 0x148) = *(s32 *)(arg0 + 0xBC);
done:;
}
void func_80023F08(s32 arg0, s32 arg1) {
}
