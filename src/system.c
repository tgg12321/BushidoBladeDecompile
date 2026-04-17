#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "system.h"
#include "psx.h"

/* Forward declarations */
extern void cdrom_ClearIrq(void);
extern void cpu_side_move_dir_4(void);
extern void marionation_Exec(void);
extern s32 cdrom_SendCmd();
extern s32 cdrom_DmaToRam();
extern s32 cdrom_DmaChain();
extern s32 func_80080660_ret(s32) asm("func_80080660");
extern void irq_AcknowledgeVblank(s32, s32);
extern s32 saEft00Add_ret(s32) asm("saEft00Add");
extern s32 saEft01Init(s32);

/* Externs for globals */
extern u8 g_cd_mode;
extern u8 g_cd_param;
extern u8 g_cd_ready_flag;
extern u8 g_cd_ready_flag2;
extern s32 g_cd_callback_a;
extern s32 g_cd_callback_b;

/* --- Functions 0x8008008C - 0x800807A8 --- */

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008008C\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x7\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);

u32 cdrom_GetMode(void) {
    return g_cd_mode;
}

u32 cdrom_GetReadyFlag(void) {
    return g_cd_ready_flag;
}

u32 cdrom_GetReadyFlag2(void) {
    return g_cd_ready_flag2;
}

void *func_800800CC(void) {
    return &g_cd_param;
}

extern void cdrom_Shutdown(void);
extern s32 cdrom_Initialize(void);
extern s32 cdrom_ConfigSPU(void);
s32 cdrom_CheckReady(s32 a0) {
    if (a0 == 2) {
        cdrom_Shutdown();
        return 1;
    }
    if (cdrom_Initialize() != 0) {
        return 0;
    }
    if (a0 == 1) {
        if (cdrom_ConfigSPU() != 0) {
            return 0;
        }
    }
    return 1;
}

void func_80080148(void) {
    cdrom_ClearIrq();
}

extern s32 g_cd_debug_level;
extern s32 g_cd_cmd_table[];
extern s32 g_cd_result_table[];
extern char g_str_none;

s32 cdrom_SetDebugLevel(s32 a0) {
    s32 old = g_cd_debug_level;
    g_cd_debug_level = a0;
    return old;
}

void *cdrom_GetCmdName(u32 a0) {
    u32 idx = a0 & 0xFF;
    void *ret;
    if (idx < 0x1C) {
        ret = (void *)g_cd_cmd_table[idx];
        goto done;
    }
    ret = &g_str_none;
done:
    return ret;
}

void *cdrom_GetResultName(u32 a0) {
    u32 idx = a0 & 0xFF;
    void *ret;
    if (idx < 0x7) {
        ret = (void *)g_cd_result_table[idx];
        goto done;
    }
    ret = &g_str_none;
done:
    return ret;
}

void func_800801E8(void) {
    cpu_side_move_dir_4();
}

void func_80080208(void) {
    marionation_Exec();
}

s32 cdrom_SetCallbackA(s32 a0) {
    s32 old = g_cd_callback_a;
    g_cd_callback_a = a0;
    return old;
}

s32 cdrom_SetCallbackB(s32 a0) {
    s32 old = g_cd_callback_b;
    g_cd_callback_b = a0;
    return old;
}

extern s32 g_cd_sector_buf[];
extern s32 tslTm2LoadImage(s32, void *, void *, s32);

s32 func_80080258(s32 a0, s32 a1, s32 a2) {
    register s32 result asm("s7");
    s32 count;
    unsigned long long new_var2;
    s32 idx;
    s32 saved;
    int new_var;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    elem = &g_cd_sector_buf[idx];
    new_var = 3;
    result = 0;
    new_var2 = new_var;
    count = new_var2;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, a2, 0) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
s32 func_80080390(s32 a0, s32 a1) {
    register s32 result asm("s6");
    s32 count;
    unsigned long long new_var2;
    s32 idx;
    s32 saved;
    int new_var;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    elem = &g_cd_sector_buf[idx];
    new_var = 3;
    result = 0;
    new_var2 = new_var;
    count = new_var2;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, 0, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, 0, 1) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count;
    unsigned long long new_var2;
    s32 idx;
    s32 saved;
    int new_var;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    elem = &g_cd_sector_buf[idx];
    new_var = 3;
    new_var2 = new_var;
    count = new_var2;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, a2, 0) == 0) {
        goto done;
    }
next:
    count--;
    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
done:
    if (count == (-1)) {
        return 0;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, a2);
        r ^= 2;
        return (u32)r < 1;
    }
}
/* kengo:MED  |  tsl_pkt/tslPolyF4Init  |  81i */

s32 func_80080600(void) {
    cdrom_SendCmd();
    return 1;
}

s32 func_80080620(void) {
    return cdrom_DmaToRam() == 0;
}

s32 func_80080640(void) {
    return cdrom_DmaChain() == 0;
}

void func_80080660(s32 a0) {
    irq_AcknowledgeVblank(3, a0);
}

void func_80080684(s32 a0) {
    saEft01Init(a0);
}

void cdrom_FramesToBcd(s32 frames, u8 *out) {
    register u8 *v0 asm("v0") = out;
    s32 total;
    register s32 secs asm("a3");
    s32 mins;
    s32 rem75;
    s32 d1;
    s32 m1;
    s32 new_var2;
    s32 d2;
    s32 m2;
    s32 new_var;
    s32 d3;
    s32 m3;
    frames++;
    frames--;
    total = frames + 150;
    secs = total / 75;
    rem75 = total % 75;
    mins = secs / 60;
    secs = secs % 60;
    d1 = rem75 / 10;
    ;
    v0[2] = (u8)((d1 << 4) + (rem75 % 10));
    d2 = secs / 10;
    new_var2 = d2 << 4;
    new_var = new_var2;
    m2 = secs % 10;
    v0[1] = (u8)(new_var + m2);
    d3 = mins / 10;
    new_var = d3 << 4;
    m3 = mins % 10;
    v0[0] = (u8)(new_var + m3);
}

extern s32 g_cd_sector_buf[];
extern s32 tslTm2LoadImage(s32, void *, void *, s32);
extern s32 func_80080DB0_ret(s32, void *);
/* --- text3 segment functions (0x800807A8-0x800827D0, 17 funcs) --- */

s32 cdrom_BcdToFrames(u8 *a0) {
    u8 b0 = a0[0];
    u8 b1 = a0[1];
    s32 min, sec, frm;
    min = (b0 >> 4) * 10 + (b0 & 0xF);
    sec = min * 60;
    sec += (b1 >> 4) * 10 + (b1 & 0xF);
    {
        s32 total = sec * 75;
        u8 b2 = a0[2];
        frm = (b2 >> 4) * 10 + (b2 & 0xF);
        total += frm;
        return total - 150;
    }
}
INCLUDE_ASM("asm/funcs", func_80080828);
INCLUDE_ASM("asm/funcs", cpu_side_move_dir_4);
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_4  |  160i  |  x4 size collision */
INCLUDE_ASM("asm/funcs", marionation_Exec);
/* kengo:HIGH  |  nm_mario/marionation_Exec  |  180i  |  +1 near-exact */
INCLUDE_ASM("asm/funcs", tslTm2LoadImage);
/* kengo:MED  |  tsl_tm2/tslTm2LoadImage  |  253i  |  -10 x2 size collision */
extern volatile u8 *g_cd_index_reg;
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_irq_reg;
extern volatile u8 *g_cd_param_fifo;

s32 cdrom_SendCmd(u8 *a0) {
    *g_cd_index_reg = 2;
    *g_cd_req_reg = a0[0];
    *g_cd_irq_reg = a0[1];
    *g_cd_index_reg = 3;
    *g_cd_param_fifo = a0[2];
    *g_cd_req_reg = a0[3];
    *g_cd_irq_reg = 0x20;
    return 0;
}
extern volatile u32 *g_cd_dma_madr;
extern volatile u32 *g_cd_dma_ctrl;
extern volatile u8 g_cd_status_a;
extern volatile u8 g_cd_status_b;
extern volatile u8 g_cd_status_c;
void cdrom_ClearIrq(void) {
    u8 v0;
    volatile u8 *p94;
    *g_cd_index_reg = 1;
    v0 = *g_cd_irq_reg & 7;
    if (v0 != 0) {
        do {
            *g_cd_index_reg = 1;
            *g_cd_irq_reg = 7;
            *g_cd_req_reg = 7;
            v0 = *g_cd_irq_reg & 7;
        } while (v0 != 0);
    }
    g_cd_status_c = 0;
    v0 = g_cd_status_c;
    p94 = &g_cd_status_a;
    g_cd_status_b = v0;
    *p94 = 2;
    *g_cd_index_reg = 0;
    *g_cd_irq_reg = 0;
    *g_cd_dma_madr = 0x1325;
}
extern volatile u16 * volatile g_cd_spu_voice;
s32 cdrom_ConfigSPU(void) {
    u8 buf[4];
    volatile u16 *v1;
    v1 = g_cd_spu_voice;
    if (v1[0xDC] == 0) {
        if (v1[0xDD] == 0) {
            v1[0xC0] = 0x3FFF;
            v1[0xC1] = 0x3FFF;
            v1 = g_cd_spu_voice;
        }
    }
    v1[0xD8] = 0x3FFF;
    v1[0xD9] = 0x3FFF;
    v1[0xD5] = 0xC001;
    buf[2] = 0x80;
    buf[0] = 0x80;
    buf[3] = 0;
    buf[1] = 0;
    *g_cd_index_reg = 2;
    *g_cd_req_reg = buf[0];
    *g_cd_irq_reg = buf[1];
    *g_cd_index_reg = 3;
    *g_cd_param_fifo = buf[2];
    *g_cd_req_reg = buf[3];
    *g_cd_irq_reg = 0x20;
    return 0;
}
extern s32 g_cd_init_flag;
extern void irq_EnableInterrupts(s32, void *);
extern u8 D_80081F1C;
void cdrom_Shutdown(void) {
    g_cd_callback_b = 0;
    g_cd_callback_a = 0;
    g_cd_init_flag = 0;
    *(s32 *)&g_cd_mode = 0;
    irq_DisableInterrupts();
    irq_EnableInterrupts(2, &D_80081F1C);
}
extern void D_800162A8;
extern void D_800162B4;
extern void D_800A1498;

s32 cdrom_Initialize(void) {
    u8 v0;
    volatile u8 *p94;

    tslTm2LoadImage_2(&D_800162A8);
    debug_printf(&D_800162B4, &D_800A1498);

    g_cd_ready_flag2 = 0;
    g_cd_ready_flag = 0;
    g_cd_callback_b = 0;
    g_cd_callback_a = 0;
    g_cd_init_flag = 0;
    *(s32 *)&g_cd_mode = 0;

    irq_DisableInterrupts();
    irq_EnableInterrupts(2, &D_80081F1C);

    *g_cd_index_reg = 1;
    v0 = *g_cd_irq_reg & 7;
    if (v0 != 0) {
        do {
            *g_cd_index_reg = 1;
            *g_cd_irq_reg = 7;
            *g_cd_req_reg = 7;
            v0 = *g_cd_irq_reg & 7;
        } while (v0 != 0);
    }

    g_cd_status_c = 0;
    v0 = g_cd_status_c;
    p94 = &g_cd_status_a;
    g_cd_status_b = v0;
    *p94 = 2;
    *g_cd_index_reg = 0;
    *g_cd_irq_reg = 0;
    *g_cd_dma_madr = 0x1325;

    tslTm2LoadImage(1, 0, 0, 0);

    if (*(s32 *)&g_cd_mode & 0x10) {
        tslTm2LoadImage(1, 0, 0, 0);
    }

    if (tslTm2LoadImage(0xA, 0, 0, 0) != 0) {
        return -1;
    }
    if (tslTm2LoadImage(0xC, 0, 0, 0) != 0) {
        return -1;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, 0);
        r ^= 2;
        return -((u32)(0 < (u32)r));
    }
}
extern s32 sys_VSync(s32);
extern void tslTm2LoadImage_2(void *);
extern void debug_printf(void *, void *, s32, s32, s32);
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
extern s32 g_str_cd_timeout;
extern s32 D_800161C8;
extern void D_800162C0;
extern u8 D_800A11D5;
extern s32 D_800A11DC[];
extern s32 D_800A125C[];
extern u8 D_800A1494;
extern u8 D_800A1495;
extern volatile u32 *D_800A14C0;
extern s32 D_800161B8;
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    tslTm2LoadImage_2(&D_800161B8);
    {
        s32 arg5, arg4;
        arg5 = tbl_125c[idx_1494[1]];
        arg4 = tbl_125c[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
/* kengo:MED  |  sa_eft/saEft01Init  |  91i */
extern volatile u32 *g_cd_dma_madr;
extern volatile u32 *g_cd_dma_bcr;
extern volatile u32 *g_cd_dma_ctrl_b4;
extern volatile u32 *g_cd_dma_dest;
extern volatile u32 *g_cd_dma_size;
extern volatile u32 *g_cd_dma_ctrl;

s32 cdrom_DmaToRam(s32 a0, s32 a1) {
    volatile u8 *v1;
    u32 v0;
    *g_cd_index_reg = 0;
    *g_cd_irq_reg = CD_IRQ_DATA_READY;
    *g_cd_dma_bcr = 0x20943;
    *g_cd_dma_madr = 0x1323;
    *g_cd_dma_ctrl_b4 = *g_cd_dma_ctrl_b4 | DMA_CD_ENABLE;
    *g_cd_dma_dest = a0;
    *g_cd_dma_size = a1 | 0x10000;
    v1 = g_cd_index_reg;
    do {
        __asm__ volatile("nop");
        v0 = *v1 & CD_STAT_DATA_REQ;
    } while (v0 == 0);
    *g_cd_dma_ctrl = DMA_CD_TO_RAM;
    if ((*g_cd_dma_ctrl & DMA_BUSY) != 0) {
        do {
            v0 = *g_cd_dma_ctrl & DMA_BUSY;
        } while (v0 != 0);
    }
    *g_cd_dma_madr = 0x1325;
    return 0;
}
s32 cdrom_DmaChain(s32 a0, s32 a1) {
    volatile u8 *v1;
    u32 v0;
    *g_cd_index_reg = 0;
    *g_cd_irq_reg = CD_IRQ_DATA_READY;
    *g_cd_dma_bcr = 0x21020843;
    *g_cd_dma_madr = 0x1325;
    *g_cd_dma_ctrl_b4 = *g_cd_dma_ctrl_b4 | DMA_CD_ENABLE;
    *g_cd_dma_dest = a0;
    *g_cd_dma_size = a1 | 0x10000;
    v1 = g_cd_index_reg;
    v0 = *v1 & CD_STAT_DATA_REQ;
    if (v0 == 0) {
        do {
            v0 = *v1 & CD_STAT_DATA_REQ;
        } while (v0 == 0);
    }
    *g_cd_dma_ctrl = DMA_CD_TO_RAM_CHAIN;
    {
        volatile s32 tmp;
        tmp = *g_cd_dma_ctrl;
    }
    return 0;
}

extern s32 D_800A1460;
void cdrom_SetErrorCallback(s32 a0) {
    D_800A1460 = a0;
}

extern volatile u8 g_cd_status_a;
extern volatile u8 g_cd_status_b;
extern s32 g_cd_callback_a;
extern s32 g_cd_callback_b;
extern void D_800F19A8;
extern void D_800F19A0;
extern s32 func_80080828(void);

__asm__(
    ".set noreorder
"
    ".set noat
"
    "glabel D_80081F1C
"
    ".set reorder
"
    ".set at
"
);

void cdrom_IrqHandler(void) {
    volatile u8 *s1 = &g_cd_status_b;
    volatile u8 *s3 = s1 - 1;
    u8 s2;
    s32 s0;
    s2 = *g_cd_index_reg & 3;
    do {
        s0 = func_80080828();
        if (s0 == 0) break;
        if (s0 & 4) {
            if (g_cd_callback_b != 0) {
                ((void (*)(u8, void *))g_cd_callback_b)(*s1, &D_800F19A8);
            }
        }
        if (!(s0 & 2)) continue;
        if (g_cd_callback_a == 0) continue;
        ((void (*)(u8, void *))g_cd_callback_a)(*s3, &D_800F19A0);
    } while (1);
    *g_cd_index_reg = s2;
}
INCLUDE_ASM("asm/funcs", tslTm2LoadImage_2);
/* kengo:MED  |  tsl_tm2/tslTm2LoadImage_2  |  253i  |  -10 x2 size collision */
extern s32 D_800A1500;
extern s32 D_800A14EC;
extern s32 D_800A14E8;
extern s32 D_800A14E4;
extern s32 D_800A14E0;
extern s32 D_800A14D0;
extern s32 D_800A14DC;
extern s32 D_800A14D4;
extern s32 D_800A14D8;
extern s32 D_800A14F0;
extern s32 D_800A14F4;
extern s32 D_800A14F8;
extern s32 D_800A14FC;
extern s32 D_800162EC;
extern s32 D_80016304;
extern s32 D_80082050;
extern s32 D_80082320;

s32 saEft00Add(s32 arg0) {
    u8 sp10;
    s32 temp_s0;

    cdrom_SetCallbackA(0);
    cdrom_SetCallbackB(0);
    if (*(volatile s32 *)&D_800A1500 & 1) {
        func_80080660(0);
    }
    if (cdrom_GetMode() & 0x10) {
        if (!(sys_VSync(-1) & 0x3F)) {
            tslTm2LoadImage_2(&D_800162EC);
        }
        func_80080390(1, 0);
        D_800A14EC = sys_VSync(-1);
        *(volatile s32 *)&D_800A14E4 = -1;
        goto end;
    }
    if (arg0 != 0) {
        tslTm2LoadImage_2(&D_80016304);
        func_80080258(9, 0, 0);
        temp_s0 = (s32)func_800800CC();
        if (func_80080258(2, temp_s0, 0) != 0) {
            goto common_path;
        }
        D_800A14E4 = -1;
        __asm__("");
        goto end;
    }
common_path:
    func_80080148();
    temp_s0 = *(volatile s32 *)&D_800A14DC;
    sp10 = temp_s0;
    temp_s0 = temp_s0 & 0xFF;
    if (temp_s0 != cdrom_GetReadyFlag() || arg0 != 0) {
        if (func_80080258(0xE, (s32)&sp10, 0) == 0) {
            D_800A14E4 = -1;
            goto end;
        }
    }
    D_800A14F0 = cdrom_BcdToFrames(func_800800CC());
    cdrom_SetCallbackB((s32)&D_80082050);
    if (D_800A1500 & 1) {
        func_80080660((s32)&D_80082320);
    }
    D_800A14D8 = D_800A14D4;
    func_80080390(6, 0);
    D_800A14E4 = D_800A14D0;
    D_800A14E8 = sys_VSync(-1);
end:
    return D_800A14E4;
}

void saEft00Add_sub(void) {
    s32 *p = &D_800A1500;

    if (*p & 1) {
        func_80080684(0);
    }
    D_800A14E4 = 0;
    cdrom_SetCallbackA(D_800A14F4);
    cdrom_SetCallbackB(D_800A14F8);
    if (*p & 1) {
        func_80080660(D_800A14FC);
    }
    func_80080390(9, 0);
}
/* kengo:HIGH  |  sa_eft/saEft00Add  |  169i  |  -3 near-exact */
s32 func_800826CC(s32 arg0, s32 arg1, s32 arg2) {
    extern volatile s32 D_800A14DC_v asm("D_800A14DC");
    extern volatile s32 D_800A1500_v asm("D_800A1500");
    register s32 temp_v0 asm("v0");
    register s32 temp_v1 asm("v1");
    register s32 saved_a0 asm("a3");
    s32 *saved_arg0;
    s32 enabled;

    D_800A14DC_v = arg2;
    temp_v0 = D_800A14DC_v;
    saved_a0 = arg0;
    temp_v1 = temp_v0 & 0x30;
    if (temp_v1) {
        temp_v0 = 0x20;
        if (temp_v1 != temp_v0) {
            temp_v0 = 0x246;
        } else {
            temp_v0 = 0x249;
        }
    } else {
        temp_v0 = 0x200;
    }
    D_800A14E0 = temp_v0;
    temp_v0 = D_800A14DC_v;
    arg0 = 0;
    D_800A14DC_v = temp_v0 | 0x20;
    saved_arg0 = &D_800A14D0;
    D_800A14D4 = arg1;
    *saved_arg0 = saved_a0;
    temp_v0 = cdrom_SetCallbackA(arg0);
    D_800A14F4 = temp_v0;
    temp_v0 = cdrom_SetCallbackB(arg0);
    D_800A14F8 = temp_v0;
    enabled = D_800A1500_v & 1;
    if (enabled) {
        temp_v0 = func_80080660_ret(arg0);
        D_800A14FC = temp_v0;
    }
    temp_v0 = sys_VSync(-1);
    D_800A14EC = temp_v0;
    temp_v0 = cdrom_GetMode();
    if (temp_v0 & 0xE0) {
        tslPolyF4Init(9, 0, 0);
    }
    temp_v0 = saEft00Add_ret(arg0);
    return temp_v0 > 0;
}

s32 func_800827D0(s32 a0, s32 a1) {
    s32 *p = &D_800A14EC;
    s32 result;

    do {
        s32 tick = sys_VSync(-1);
        if (*p + 0x4B0 < tick) {
            result = -1;
        } else {
            if (p[-2] < 0) {
                goto do_seek;
            }
            tick = sys_VSync(-1);
            if (p[-1] + 0x3C < tick) {
do_seek:
                saEft00Add(1);
                result = p[-7];
            } else {
                result = p[-2];
            }
        }
        if (a0 != 0) break;
    } while (result > 0);
    ((void (*)(s32, s32))func_80080208)(1, a1);
    return result;
}
