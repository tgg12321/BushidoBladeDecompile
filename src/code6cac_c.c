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
extern u8 D_800F33D8;
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
extern void sys_VSync(s32);
extern void gpu_LoadImage(s32, s32);
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
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern s32 func_80036D88(void);
extern void gnd_disp_loop_ctrl(void);
extern s32 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 D_800A3210;
extern void func_8008BE04(void);
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
extern u8 D_800A37D0;
extern s32 D_800A3915_ext;
extern s32 D_800A36F4_ext;

/* Extern function declarations for decompiled functions */
extern s32 func_80078998(s32);
extern void func_80078988(s32);
extern void func_800789A8(s32);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void func_800789F8(s32, s32 *, s32);
extern void func_80078A18(s32);
extern s32 func_80078A38(s32 *, s32 *);
extern s32 func_80078A48(s32 *);
extern void func_80078BA8(s32);
extern s32 func_80078B04(s32);
extern void func_8007A400(void);
extern void func_8008BE4C(void);
extern s32 func_8006BEC4(s32, s32);
extern void func_8003E22C(void);
extern void game_SetPlayerCount(s32);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8001B6F4(void);
extern void func_80022568(u8 *);
extern s32 g_str_memcard_fmt;
extern s32 D_80102810;
extern s32 D_800F34D8;

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_800375EC(void) {
    func_8007A370(1);
    func_8007A3C8();
    func_80078958();
    func_80078A58(0);
    EnterCriticalSection();
    D_800A37DC = func_80078978(0xF4000001, 4, 0x2000, 0);
    D_800A37F0 = func_80078978(0xF4000001, 0x8000, 0x2000, 0);
    D_800A37FC = func_80078978(0xF4000001, 0x100, 0x2000, 0);
    D_800A3800 = func_80078978(0xF4000001, 0x2000, 0x2000, 0);
    D_800A3838 = func_80078978(0xF0000011, 4, 0x2000, 0);
    D_800A383C = func_80078978(0xF0000011, 0x8000, 0x2000, 0);
    D_800A3848 = func_80078978(0xF0000011, 0x100, 0x2000, 0);
    D_800A3850 = func_80078978(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    func_800789A8(D_800A37DC);
    func_800789A8(D_800A37F0);
    func_800789A8(D_800A37FC);
    func_800789A8(D_800A3800);
    func_800789A8(D_800A3838);
    func_800789A8(D_800A383C);
    func_800789A8(D_800A3848);
    func_800789A8(D_800A3850);
}
void func_80037774(void) {
    EnterCriticalSection();
    func_80078988(D_800A37DC);
    func_80078988(D_800A37F0);
    func_80078988(D_800A37FC);
    func_80078988(D_800A3800);
    func_80078988(D_800A3838);
    func_80078988(D_800A383C);
    func_80078988(D_800A3848);
    func_80078988(D_800A3850);
    ExitCriticalSection();
    func_8007A400();
}
s32 func_80037804(void) {
    extern s32 D_800A3924;
    s32 result;
    s32 one;
    s32 temp;
    result = (func_80078998(D_800A37DC) == 1);
    one = 1;
    if (func_80078998(D_800A37F0) == one) {
        result = 2;
    }
    if (func_80078998(D_800A37FC) == one) {
        result = 3;
    }
    if (func_80078998(D_800A3800) == one) {
        result = 4;
    }
    temp = D_800A3924;
    D_800A3924 = temp + 1;
    if (temp >= 0x78) {
        result = 2;
    }
    return result;
}
s32 func_800378A8(void) {
    if (func_80078998(D_800A37DC) == 1) {
        return 1;
    }
    if (func_80078998(D_800A37F0) == 1) {
        return 2;
    }
    if (func_80078998(D_800A37FC) == 1) {
        return 3;
    }
    return (func_80078998(D_800A3800) == 1) * 4;
}
void func_8003791C(void) {
    func_80078998(D_800A37DC);
    func_80078998(D_800A37F0);
    func_80078998(D_800A37FC);
    func_80078998(D_800A3800);
}
s32 func_80037964(void) {
    s32 one = 1;
loop:
    if (func_80078998(D_800A3838) == one) { return 1; }
    if (func_80078998(D_800A383C) == one) { return 2; }
    if (func_80078998(D_800A3848) == one) { return 3; }
    if (func_80078998(D_800A3850) != one) { goto loop; }
    return 4;
}
void func_800379D8(void) {
    func_80078998(D_800A3838);
    func_80078998(D_800A383C);
    func_80078998(D_800A3848);
    func_80078998(D_800A3850);
}
s32 func_80037A20(s32 arg0, s32 arg1)
{
  register s32 *var_s0 asm("s0");
  register s32 var_s1 asm("s1");
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (func_80078A38(sp10, var_s0) != 0)
  {
    __asm__("" : "=r"(var_s1) : "0"(var_s1));
    var_s1++;
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
    v0_val = func_80078A48(var_s0);
    var_s1 += 1;
    if (v0_val) goto loop;
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
s32 func_80037AA4(void) {
    s32 var_a1;
    register s32 var_a2 asm("a2");
    register s32 var_a0 asm("a0");
    s8 *var_v1;
    register s32 var_v0 asm("v0");
    s32 sp_dummy[2];

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a2 > 0) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
s32 func_80037B00(u8 *arg0) {
    s32 sp_dummy[2];
    register s32 var_t1 asm("t1");
    register s32 var_t3 asm("t3");
    register s32 var_t2 asm("t2");
    register s8 *var_a3 asm("a3");
    register s8 *var_a1 asm("a1");
    register s8 *var_a2 asm("a2");
    register s8 *var_t0 asm("t0");
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");

    var_t1 = 0;
    var_v0 = D_800A38C8;
    if (var_v0 <= 0) {
        goto block_end;
    }
    var_t3 = var_v0;
    var_a3 = (s8 *)&D_80102810;
loop_outer:
    var_t2 = 0;
    var_a1 = var_a3;
    var_a2 = (s8 *)arg0;
    var_t0 = var_a3 + 0x15;
loop_inner:
    var_v1 = (u8)*var_a2;
    if (var_v1 == 0) {
        goto block_5c;
    }
    var_v0 = (u8)*var_a1;
    if (var_v1 != var_v0) {
        goto block_6c;
    }
    var_a1 += 1;
    var_a2 += 1;
    if ((s32)var_a1 < (s32)var_t0) {
        goto loop_inner;
    }
block_5c:
    var_t1 += 1;
    if (var_t2 != 0) {
        goto block_74;
    }
    return 1;
block_6c:
    var_t2 = 1;
    goto block_5c;
block_74:
    if (var_t1 < var_t3) {
        var_a3 += 0x28;
        goto loop_outer;
    }
block_end:
    return 0;
}
extern s32 func_800789E8(s32 *, s32);
typedef void (*Func79A30_5)(s32 *, s32 *, s32, s32, s32);
s32 func_80037B90(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 sp18[8];
    register s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    temp_v0 = func_800789E8(sp18, 0x8001);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_800789F8(temp_v0, arg3, arg4);
    return -(func_80037964() != 1);
}
extern void func_80078A18(s32);
extern void func_80078A08(s32, s32, s32);
s32 func_80037C34(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s32 sp18[8];
    s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    if (arg6 != 0) {
        temp_v0 = func_800789E8(sp18, (arg4 << 16) | 0x200);
        if (temp_v0 == -1) {
            return -1;
        }
        func_80078A18(temp_v0);
    }
    temp_v0 = func_800789E8(sp18, 0x8002);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_80078A08(temp_v0, arg3, arg5);
    return -(func_80037964() != 1);
}
INCLUDE_ASM("asm/funcs", func_80037D14);
void func_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    func_80079A30(buf, &D_800109C8, a0, a1);
    func_80078A28(buf);
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
            Quad *dst = (Quad *)(offset + base);
            Quad *src = (Quad *)&g_file_disc_size;
            do {
                *dst = *src;
                src++;
                dst++;
            } while (src != end);
            *(s32 *)dst = *(s32 *)src;
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

s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;
        u32 j;

        j = 0;
        bp = base + offset;
        sum = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;
        s32 k = 0;
        s32 *ap;
        u8 *a2p;

        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            do {
                *dst = *sp2;
                sp2++;
                dst++;
            } while (sp2 != end);
            k = 0;
            *(s32 *)dst = *(s32 *)sp2;
        }

        ap = (s32 *)base;
        a2p = base;
        do {
            u16 *ptr = *(u16 **)((u8 *)ap + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(a2p + 0xD0);
            }
            a2p += 2;
            k++;
            ap++;
        } while (k < 0x16);
    }

    return 1;
}
/* kengo:HIGH  |  is_damage_calc/damage_DebugDisp  |  79i */

void func_80038148(void) {
    u8 *p = &D_800F33D8;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}
INCLUDE_ASM("asm/funcs", func_80038170);
INCLUDE_ASM("asm/funcs", pad_FuncAnalog);
/* kengo:HIGH  |  is_pad/pad_FuncAnalog  |  173i */
extern s32 damage_DebugDisp(s32 *);
void func_80038658(void) {
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");
    s32 var_s0;

    var_v1 = D_800A31F4;
    if (var_v1 == 4) {
        goto block_4;
    }
    if (var_v1 == 6) {
        goto block_6;
    }
    return;
block_4:
    var_s0 = func_800378A8();
    var_v0 = 1;
    if (var_s0 == 0) {
        goto block_store;
    }
    func_80078A18(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 3;
        goto block_store_clear;
    }
    var_v0 = 2;
    goto block_store_clear;
block_6:
    var_s0 = func_800378A8();
    var_v0 = 4;
    if (var_s0 == 0) {
        goto block_store;
    }
    func_80078A18(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 6;
        goto block_store_clear;
    }
    var_v0 = 5;
    D_800A379E = (s16)var_v0;
    if (damage_DebugDisp(&D_800F34D8) != 0) {
        goto block_clear;
    }
    var_v0 = 0xF;
block_store_clear:
    D_800A379E = (s16)var_v0;
block_clear:
    D_800A31F4 = 0;
    return;
block_store:
    D_800A379E = (s16)var_v0;
}
s32 func_80038734(void) {
    if ((u32)D_800A31F4 < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    pad_FuncAnalog();
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
extern s32 func_8006BEC4(s32, s32);

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
        result = -1;
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A3318 != 0) {
        } else {
            result = 1;
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
INCLUDE_ASM("asm/funcs", func_80038988);
INCLUDE_ASM("asm/funcs", motion_SetMotion);
/* kengo:MED  |  is_motion/motion_SetMotion  |  425i  |  -23 5.4% */
s32 *func_800392B8(void) {
    return &D_800F33D8;
}
void func_800392C8(void) {
    u8 val;
    int new_var;
    s32 i;
    s32 j;

    val = 0xFF;
    i = 0x1F0;
    D_800A36EC = (u8 *)&D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
loop1:
    *(&D_80101BF0 + i) = val;
    i -= 0x10;
    if (i >= 0) goto loop1;

    new_var = -1;
    j = 0xB30;
loop2:
    *((s16 *)((u8 *)&D_800F68E0 + j)) = new_var;
    j -= (0, 0x10);
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

    q = (s16 *)&D_800F68E0;
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
INCLUDE_ASM("asm/funcs", saSeInit_2);
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
    if ((u8)val == 0x77) {
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
    func_8007EFFC(disp_CalcFov(0x2D));
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
        D_800A3738 = func_80078978(0xF000000B, 0x400, 0x2000, 0);
    } while (D_800A3738 == neg1);
    neg1 = -1;
    do {
        D_800A3810 = func_80078978(0xF000000B, 0x8000, 0x2000, 0);
    } while (D_800A3810 == neg1);
    ExitCriticalSection();
    neg1 = -1;
    sys_VSync(2);
    func_8008BE04();
    do {
        D_800A373C = func_800789E8(&D_800A3210, 2);
    } while (D_800A373C == neg1);
    neg1 = -1;
    do {
        D_800A3734 = func_800789E8(&D_800A3210, 0x8001);
    } while (D_800A3734 == neg1);
    func_8008C464(2, 0, 0);
    func_8008C464(1, 3, 0xE100);
    func_8008C464(1, 4, 1);
}
void func_8003A264(void) {
    func_80078A18(D_800A3734);
    func_80078A18(D_800A373C);
    EnterCriticalSection();
    func_80078988(D_800A3738);
    func_80078988(D_800A3810);
    ExitCriticalSection();
    sys_VSync(2);
    func_8008BE4C();
    func_8008C464(1, 1, 0);
}
s32 func_8003A2DC(void) {
    return (func_8008C464(0, 0, 0) & 0x180) == 0;
}
void func_8003A308(void) {
    if (func_8008C464(3, 1, 0) != 0) {
        D_800A38A0 = 1;
    } else {
        D_800A38A0 = 0;
    }
    func_8008C464(3, 0, 1);
}
void func_8003A360(void) {
    func_800789A8(D_800A3810);
    func_800789A8(D_800A3738);
    D_800A320C = 1;
    D_800A3730 = 0;
}
void func_8003A39C(void) {
    D_800A320C = 0;
    D_800A3730 = 0;
    func_8008C464(2, 0, 0);
    func_8008C464(1, 1, 0);
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
INCLUDE_ASM("asm/funcs", func_8003A450);
void func_8003A574(void) {
    func_800789F8(D_800A3734, &D_800A3688, 8);
}
extern s32 D_800A38D0;
s32 pad_ClearAppliBuffer(void) {
    s32 s0;
    s32 s1;
    s32 a1;
    s32 a0;
    s32 v0;

    s1 = 0;
    s0 = func_80078B04(0xF2000001);
    if (s0 >= 0x401) {
        goto overflow;
    }
    goto loop_check;
overflow:
    func_80078BA8(0xF2000001);
    s0 = 0;
loop_check:
    if (func_80078998(D_800A3738) != 0) {
        goto success;
    }
    if (func_80078998(D_800A3810) == 0) {
        goto poll;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    func_8008C464(2, 0, 0);
    s0 = 0;
    func_8003A574();
    func_80078BA8(0xF2000001);
poll:
    v0 = (func_8008C464(0, 0, 0) >> 7) & 3;
    if (v0 == 1) {
        goto loop_check;
    }
    v0 = func_80078B04(0xF2000001) - s0;
    if (v0 < 0x3C01) {
        goto loop_check;
    }
    s1 += 1;
    v0 = 0;
    if (s1 >= 5) {
        goto epilogue;
    }
    goto overflow;
success:
    a1 = D_800A3688;
    a0 = D_800A368C;
    v0 = a1 >> 16;
    v0 = v0 ^ a1;
    v0 = v0 ^ (a0 >> 16);
    v0 = v0 & 0xFFFF;
    if ((a0 & 0xFFFF) == v0) {
        goto match;
    }
    v0 = 0;
    D_800A38D0 += 1;
    goto epilogue;
ret0_tramp:
    v0 = 0;
    goto epilogue;
match:
    v0 = 1;
    D_800A36C0 = a1;
    D_800A36C4 = a0;
epilogue:
    return v0;
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
    sys_VSync(2);
    func_8003AA48();
    sys_VSync(2);
}
void func_8003AAB0(void) {
    s32 val;
    D_800A3870 = 2;
    sys_VSync(2);
    val = 2;
    do {
        func_8003AA48();
        if (D_800A320C == 0) {
            goto end;
        }
        func_80078BA8(0xF2000001);
        do {
        } while (func_80078B04(0xF2000001) < 0x100);
    } while (D_800A3870 == val);
end:
    sys_VSync(2);
    func_8003AA48();
    sys_VSync(2);
}
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern u16 func_80035E88(s32);
extern s32 func_80035EDC(s16);
extern u16 func_80019488(void);
extern void func_800194C0(s16);
extern u16 func_80079154(void);
extern void func_80019568(s32);
extern u8 D_800A38AC;
extern s32 D_800A38D0;
extern s32 D_800A3908;
extern s32 D_800A38FC;
extern u16 D_800A37C4;

INCLUDE_ASM("asm/funcs", func_8003AB44);
s32 func_8003ACB8(void) {
    s32 temp_s0;

    func_80077AE0();
    gpu_SetDispMask(0);
    D_800A37B8 = 0;
    D_800A38AC = 0;
    D_800A38D0 = 0;
    D_800A3908 = 0;
    D_800A38FC = 0;
    func_8003A174();
    do {
        func_80019568(1);
        func_8005C6D0();
        temp_s0 = func_8003AB44();
        sys_VSync(2);
    } while (temp_s0 == 0);
    func_80077B00();
    func_800194F4();
    D_800A37C4 = func_80035E88(g_file_disc_size);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    D_800A38E4 = func_80035EDC(D_800A36C6);
    D_800A37C4 = func_80019488();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    func_800194C0(D_800A36C6);
    D_800A37C4 = func_80079154();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
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
    gpu_DisableDisplay();
    func_80078BA8(0xF2000001);
    return temp_s0;
}
INCLUDE_ASM("asm/funcs", func_8003AE5C);
void func_8003AF40(s32 arg0) {
    if ((&D_8010277E)[arg0] == 0xFF) {
        (&D_8010277E)[arg0] = (&D_8010277E)[(u32)arg0 < 1u];
    }
    func_80022580(arg0, ((s8 *)&D_80102780)[arg0], ((s8 *)&D_8010277C)[arg0], ((s8 *)&D_8010277E)[arg0], 0);
    gpu_EnableDisplay();
    func_80020D38();
    func_80040510(arg0, (&D_8008D578)[(s8)(&D_8010277C)[arg0]], (s32)0x80190800);
}
void func_8003AFFC(void) {
    s32 addr = (s32)0x80190800;
    s32 s0;
    s16 *edcp;
    s32 s2;
    u8 *tbl;
    s32 v1;

    gpu_EnableDisplay();
    func_80020D38();
    func_8004939C();

    tbl = &D_8008E5CC;
    edcp = &D_80101EDC;
    s2 = 0;
    s0 = 0;
loop:
    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + s0));
    func_800494D4(s2, *(tbl + *(s16 *)((u8 *)&D_80101ED2 + s0) * 8 + *(s16 *)((u8 *)&D_80101ED6 + s0)));

    v1 = *edcp;
    if (v1 != -1) {
        func_800493E4((&D_8008EB80)[v1]);
        v1 = *edcp;
        if (v1 == 14) {
            func_800493E4(D_8008EB8E + 3);
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
    s32 s0;
    u8 *tbl;
    s32 v0;

    gpu_EnableDisplay();
    func_80020D38();
    func_8004939C();

    s0 = arg0 * 1100;
    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + s0));

    if (D_800A38DC == 5) {
        tbl = &D_8008E6A4;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + s0) * 6;
    } else {
        tbl = &D_8008E5CC;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + s0) * 8;
    }
    func_800494D4(arg0, *(tbl + v0 + *(s16 *)((u8 *)&D_80101ED6 + s0)));
    func_80049584(addr);
}
void func_8003B20C(s32 arg0) {
    u8 *new_var;
    register s32 one asm("v1") = 1;
    D_80102780 = 0;
    D_80102781 = one;
    arg0 = ((s32 *)(new_var = &D_800900EC))[arg0];
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = one;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = arg0;
    file_LoadOverlay();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = (&D_8008D538)[(s8)D_8010277C];
}
extern u8 *D_800A3894;
extern void player_SetCharId(s32, s32);
void func_8003B2C8(void) {
    u8 *p = &D_8010277C;
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
    u8 *p = &D_8010277C;
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
    func_80022568(&D_80101EC8);
}
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
s32 func_8003B484(u8 *arg0) {
    D_800A3712 = 1;
    D_8010277D = arg0[0];
    D_8010277F = arg0[1];
    D_80102783 = arg0[2];
    func_8003AF40(1);
    func_8003AFFC();
    return 3;
}
void func_8003B4DC(void) {
    D_800A3712 = 1;
    D_8010277E = 0;
    D_8010277D = 0x1F;
    D_8010277F = 0;
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
INCLUDE_ASM("asm/funcs", func_8003B5A4);
void func_8003B870(void) {
    player_SetCharId(0, D_800A376A);
    player_SetCharId(1, 0);
    obj_InitChars();
    gpu_InitDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    gpu_DisableDisplay();
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
        gpu_DrawSync(0);
        func_8003AE5C(D_800A3844);
        D_800A37C0 = 500;
        D_800A38F8 = 0;
        g_disp_enable = DISP_ACTIVE;
        se_data_set();
        func_8003B5A4();
        disp_SetFramebufferMode(1, 0, 0, 0);
        D_800A390D = 1;
    }
}
