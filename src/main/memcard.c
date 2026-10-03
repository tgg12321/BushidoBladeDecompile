/* The memory-card layer: memcard_Init / memcard_Quit, the software and hardware event waits
 * (memcard_PollSwEvents..memcard_AckHwEvents), memcard_CountFiles, memcard_ReadFile and
 * memcard_WriteFile. .text 0x800375EC (ROM 0x27DEC). Start boundary: LEGACY (a tooling split, no
 * evidence either way). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include <psxsdk/libsnd.h>
#include "game.h"
#include "system.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Extern data declarations */





/* Extern function declarations */










/* This file's .rodata: the memory-card path formats. */
const char g_str_memcard_fmt[12] = "bu%1d%1d:*";
const char D_800109BC[12] = "bu%1d%1d:%s";













































































/* GP-relative extern data (for decompiled functions) */












extern s32 g_memcard_sw_event_ioe;
extern s32 g_memcard_sw_event_err;
extern s32 g_memcard_sw_event_timeout;
extern s32 g_memcard_sw_event_new;
extern s32 g_memcard_hw_event_ioe;
extern s32 g_memcard_hw_event_err;
extern s32 g_memcard_hw_event_timeout;
extern s32 g_memcard_hw_event_new;







extern s32 g_memcard_file_count;








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








extern s32 g_memcard_file_list;


/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void memcard_Init(void) {
    InitCARD(1);
    StartCARD();
    _bu_init();
    ChangeClearPAD(0);
    EnterCriticalSection();
    g_memcard_sw_event_ioe = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_memcard_sw_event_err = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_memcard_sw_event_timeout = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_memcard_sw_event_new = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_memcard_hw_event_ioe = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_memcard_hw_event_err = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_memcard_hw_event_timeout = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_memcard_hw_event_new = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    EnableEvent(g_memcard_sw_event_ioe);
    EnableEvent(g_memcard_sw_event_err);
    EnableEvent(g_memcard_sw_event_timeout);
    EnableEvent(g_memcard_sw_event_new);
    EnableEvent(g_memcard_hw_event_ioe);
    EnableEvent(g_memcard_hw_event_err);
    EnableEvent(g_memcard_hw_event_timeout);
    EnableEvent(g_memcard_hw_event_new);
}
void memcard_Quit(void) {
    EnterCriticalSection();
    CloseEvent(g_memcard_sw_event_ioe);
    CloseEvent(g_memcard_sw_event_err);
    CloseEvent(g_memcard_sw_event_timeout);
    CloseEvent(g_memcard_sw_event_new);
    CloseEvent(g_memcard_hw_event_ioe);
    CloseEvent(g_memcard_hw_event_err);
    CloseEvent(g_memcard_hw_event_timeout);
    CloseEvent(g_memcard_hw_event_new);
    ExitCriticalSection();
    StopCARD();
}
s32 memcard_PollSwEventsTimeout(void) {
    extern s32 D_800A3924;
    s32 result;
    s32 one;
    s32 temp;
    result = (TestEvent(g_memcard_sw_event_ioe) == 1);
    one = 1;
    if (TestEvent(g_memcard_sw_event_err) == one) {
        result = 2;
    }
    if (TestEvent(g_memcard_sw_event_timeout) == one) {
        result = 3;
    }
    if (TestEvent(g_memcard_sw_event_new) == one) {
        result = 4;
    }
    temp = D_800A3924;
    D_800A3924 = temp + 1;
    if (temp >= 0x78) {
        result = 2;
    }
    return result;
}
s32 memcard_PollSwEvents(void) {
    if (TestEvent(g_memcard_sw_event_ioe) == 1) {
        return 1;
    }
    if (TestEvent(g_memcard_sw_event_err) == 1) {
        return 2;
    }
    if (TestEvent(g_memcard_sw_event_timeout) == 1) {
        return 3;
    }
    return (TestEvent(g_memcard_sw_event_new) == 1) * 4;
}
void memcard_AckSwEvents(void) {
    TestEvent(g_memcard_sw_event_ioe);
    TestEvent(g_memcard_sw_event_err);
    TestEvent(g_memcard_sw_event_timeout);
    TestEvent(g_memcard_sw_event_new);
}
s32 memcard_WaitHwEvent(void) {
    s32 one = 1;
loop:
    if (TestEvent(g_memcard_hw_event_ioe) == one) { return 1; }
    if (TestEvent(g_memcard_hw_event_err) == one) { return 2; }
    if (TestEvent(g_memcard_hw_event_timeout) == one) { return 3; }
    if (TestEvent(g_memcard_hw_event_new) != one) { goto loop; }
    return 4;
}
void memcard_AckHwEvents(void) {
    TestEvent(g_memcard_hw_event_ioe);
    TestEvent(g_memcard_hw_event_err);
    TestEvent(g_memcard_hw_event_timeout);
    TestEvent(g_memcard_hw_event_new);
}
s32 memcard_CountFiles(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&g_memcard_file_list;
    sprintf(sp10, g_str_memcard_fmt, arg0, arg1);
    var_s1 = 0;
    if (firstfile(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (nextfile(var_s0) != 0);
    }
    g_memcard_file_count = var_s1;
    return var_s1;
}
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;
    s32 sh; /* FAKE: shift-amount constant-holder (SOTN cd.c new_var2 shape) -- survives
               cse past the guard join and raises var_a0's allocation priority so
               global-alloc assigns a0/v1 in target order; reload's constant-equivalence
               (update_equiv_regs) then substitutes 13 and deletes the li: zero extra bytes. */

    sh = 0xD;
    var_a1 = 0;
    var_a0 = 0;
    var_a2 = g_memcard_file_count;
    if (var_a1 < var_a2) {
        var_v1 = (s8 *)&g_memcard_file_list;
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
    if (var_t1 < g_memcard_file_count) {
        var_t3 = g_memcard_file_count;
        var_a3 = (s8 *)&g_memcard_file_list;
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
    g_memcard_fd = temp_v0;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    read(temp_v0, arg3, arg4);
    return -(memcard_WaitHwEvent() != 1);
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
    g_memcard_fd = temp_v0;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    write(temp_v0, arg3, arg5);
    return -(memcard_WaitHwEvent() != 1);
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 g_memcard_fd;
s32 g_memcard_sw_event_ioe;
s32 g_memcard_sw_event_err;
s32 g_memcard_sw_event_timeout;
s32 g_memcard_sw_event_new;
s32 g_memcard_hw_event_ioe;
s32 g_memcard_hw_event_err;
s32 g_memcard_hw_event_timeout;
s32 g_memcard_hw_event_new;
s32 g_memcard_file_count;
s32 D_800A3924;
