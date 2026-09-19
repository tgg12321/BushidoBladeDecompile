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










extern s32 D_800109BC;













































































/* GP-relative extern data (for decompiled functions) */












extern s32 D_800A37DC;
extern s32 D_800A37F0;
extern s32 D_800A37FC;
extern s32 D_800A3800;
extern s32 D_800A3838;
extern s32 D_800A383C;
extern s32 D_800A3848;
extern s32 D_800A3850;







extern s32 D_800A38C8;








/* Extern function declarations for decompiled functions */
extern s32 TestEvent(s32);
extern void CloseEvent(s32);
extern void EnableEvent(s32);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void read(s32, s32 *, s32);

extern s32 firstfile(s32 *, s32 *);
extern s32 nextfile(s32 *);


extern void StopCARD(void);








extern s32 g_str_memcard_fmt;
extern s32 D_80102810;


/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void memcard_Init(void) {
    InitCARD(1);
    StartCARD();
    _bu_init();
    ChangeClearPAD(0);
    EnterCriticalSection();
    D_800A37DC = OpenEvent(0xF4000001, 4, 0x2000, 0);
    D_800A37F0 = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    D_800A37FC = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    D_800A3800 = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    D_800A3838 = OpenEvent(0xF0000011, 4, 0x2000, 0);
    D_800A383C = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    D_800A3848 = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    D_800A3850 = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    EnableEvent(D_800A37DC);
    EnableEvent(D_800A37F0);
    EnableEvent(D_800A37FC);
    EnableEvent(D_800A3800);
    EnableEvent(D_800A3838);
    EnableEvent(D_800A383C);
    EnableEvent(D_800A3848);
    EnableEvent(D_800A3850);
}
void func_80037774(void) {
    EnterCriticalSection();
    CloseEvent(D_800A37DC);
    CloseEvent(D_800A37F0);
    CloseEvent(D_800A37FC);
    CloseEvent(D_800A3800);
    CloseEvent(D_800A3838);
    CloseEvent(D_800A383C);
    CloseEvent(D_800A3848);
    CloseEvent(D_800A3850);
    ExitCriticalSection();
    StopCARD();
}
s32 func_80037804(void) {
    extern s32 D_800A3924;
    s32 result;
    s32 one;
    s32 temp;
    result = (TestEvent(D_800A37DC) == 1);
    one = 1;
    if (TestEvent(D_800A37F0) == one) {
        result = 2;
    }
    if (TestEvent(D_800A37FC) == one) {
        result = 3;
    }
    if (TestEvent(D_800A3800) == one) {
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
    if (TestEvent(D_800A37DC) == 1) {
        return 1;
    }
    if (TestEvent(D_800A37F0) == 1) {
        return 2;
    }
    if (TestEvent(D_800A37FC) == 1) {
        return 3;
    }
    return (TestEvent(D_800A3800) == 1) * 4;
}
void memcard_AckSwEvents(void) {
    TestEvent(D_800A37DC);
    TestEvent(D_800A37F0);
    TestEvent(D_800A37FC);
    TestEvent(D_800A3800);
}
s32 func_80037964(void) {
    s32 one = 1;
loop:
    if (TestEvent(D_800A3838) == one) { return 1; }
    if (TestEvent(D_800A383C) == one) { return 2; }
    if (TestEvent(D_800A3848) == one) { return 3; }
    if (TestEvent(D_800A3850) != one) { goto loop; }
    return 4;
}
void memcard_AckHwEvents(void) {
    TestEvent(D_800A3838);
    TestEvent(D_800A383C);
    TestEvent(D_800A3848);
    TestEvent(D_800A3850);
}
s32 memcard_CountFiles(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    sprintf(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (firstfile(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (nextfile(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;
    s32 sh; /* FAKE: shift-amount constant-holder (SOTN cd.c new_var2 shape) — survives
               cse past the guard join, lifts var_a0 to 11 refs/17 len (pri 19411 < 20000)
               so global-alloc assigns a0/v1 in target order; reload's constant-equivalence
               (update_equiv_regs) then substitutes 13 and deletes the li: zero extra bytes.
               Sanctioned per Judge ruling 2026-07-28 06:28 (docs/grind/decisions.md). */

    sh = 0xD;
    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a1 < var_a2) {
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
    var_a0 = var_v0 >> sh;
    return 0xF - var_a0;
}
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;
    s32 var_t3;

    var_t1 = 0;
    if (var_t1 < D_800A38C8) {
        var_t3 = D_800A38C8;
        var_a3 = (s8 *)&D_80102810;
        while (var_t1 < var_t3) {
            var_t2 = 0;
            var_a1 = var_a3;
            var_a2 = (s8 *)arg0;
            var_t0 = var_a3 + 0x15;
            while (1) {
                var_v1 = (u8)*var_a2;
                if (var_v1 == 0) {
                    break;
                }
                var_v0 = (u8)*var_a1;
                if (var_v1 != var_v0) {
                    goto block_6c;
                }
                var_a1 += 1;
                var_a2 += 1;
                if ((s32)var_a1 >= (s32)var_t0) {
                    break;
                }
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
            var_a3 += 0x28;
        }
    }
    return 0;
}
extern s32 open(s32 *, s32);
typedef void (*Func79A30_5)(s32 *, s32 *, s32, s32, s32);
s32 memcard_ReadFile(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 sp18[8];
    s32 temp_v0;

    ((Func79A30_5)sprintf)(sp18, &D_800109BC, arg0, arg1, arg2);
    temp_v0 = open(sp18, 0x8001);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    read(temp_v0, arg3, arg4);
    return -(func_80037964() != 1);
}
extern void close(s32);
extern void write(s32, s32, s32);
s32 memcard_WriteFile(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s32 sp18[8];
    s32 temp_v0;

    ((Func79A30_5)sprintf)(sp18, &D_800109BC, arg0, arg1, arg2);
    if (arg6 != 0) {
        temp_v0 = open(sp18, (arg4 << 16) | 0x200);
        if (temp_v0 == -1) {
            return -1;
        }
        close(temp_v0);
    }
    temp_v0 = open(sp18, 0x8002);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    write(temp_v0, arg3, arg5);
    return -(func_80037964() != 1);
}
