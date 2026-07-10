#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "system.h"
#include "psx.h"

/* Forward declarations */
extern void cdrom_ClearIrq(void);
extern s32 cpu_side_move_dir_4(s32, u8 *);
extern s32 marionation_Exec(s32, u8 *);
extern s32 cdrom_SendCmd();
extern s32 cdrom_DmaToRam();
extern s32 cdrom_DmaChain();
extern s32 irq_AcknowledgeVblank(s32, s32);
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
    "glabel bios_DeliverEvent\n"
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

/* PsyQ 4.0 LIBCD sys: CdComstr — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
void *cdrom_GetCmdName(u8 com) {
    if (com > 0x1B) {
        return &g_str_none;
    }
    return (void *)g_cd_cmd_table[com];
}

/* PsyQ 4.0 LIBCD sys: CdIntstr — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
void *cdrom_GetResultName(u8 intr) {
    if (intr > 6) {
        return &g_str_none;
    }
    return (void *)g_cd_result_table[intr];
}

/* PsyQ 4.0 LIBCD sys: CdSync — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
s32 Vu0SetLightColMatrix_800801E8(s32 mode, u8 *result) {
    return cpu_side_move_dir_4(mode, result);
}

/* PsyQ 4.0 LIBCD sys: CdReady — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
s32 Vu0SetLightColMatrix_80080208(s32 mode, u8 *result) {
    return marionation_Exec(mode, result);
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
    s32 idx;
    s32 saved;
    s32 *elem;
    s32 *base;
    s32 status;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    base = g_cd_sector_buf;
    elem = base + idx;
    count = 3;

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
        status = 0;
        goto done;
    }
next:
    count--;
    status = -1;
    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
done:
    if (status != 0) {
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

s32 Vu0SetLightColMatrix_80080640(void) {
    return cdrom_DmaChain() == 0;
}

/* PsyQ 4.0 LIBCD sys: CdDataCallback — verbatim-linked Sony object (census
   2026-07-09); returns the previous callback */
s32 tslTmlGetHeda(s32 a0) {
    return irq_AcknowledgeVblank(3, a0);
}

void Vu0SetLightColMatrix_80080684(s32 a0) {
    saEft01Init(a0);
}

/* PsyQ 4.0 LIBCD sys: CdIntToPos — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
u8 *cdrom_FramesToBcd(s32 i, u8 *p) {
    inline int ENCODE_BCD(n) { return ((n / 10) << 4) + (n % 10); }

    i += 150;
    p[2] = ENCODE_BCD(i % 75);
    p[1] = ENCODE_BCD(i / 75 % 60);
    p[0] = ENCODE_BCD(i / 75 / 60);
    return p;
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
s32 func_80080828(void) {
    return 0;
}
extern s32 sys_VSync(s32);
extern void tslTm2LoadImage_2(void *);
extern void debug_printf(void *, void *, s32, s32, s32);
extern s32 sys_GetVblankCount(void);
extern s32 func_80080828(void);
extern u8 *D_800A147C;
extern s32 D_800A11B4;
extern s32 D_800A11B8;
extern void D_800F19A0;
extern void D_800F19A8;
extern void D_800F19B0;
extern void D_80016240;
extern void D_80016248;
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
extern s32 D_800161B8;
extern s32 D_800161C8;
extern u8 D_800A11D5;
extern s32 D_800A11DC[];
extern s32 D_800A125C[];
extern u8 D_800A1494;
extern u8 D_800A1495;
s32 cpu_side_move_dir_4(s32 a0, u8 *a1)
{
  int new_var;
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt - -1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg4, arg5;
    arg4 = tbl_125c[idx_1494[0]];
    arg5 = tbl_125c[idx_1494[1]];
    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, arg5);
  }
  cdrom_ClearIrq();
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;
  if (sys_GetVblankCount() != 0)
  {
    saved = (*D_800A147C) & 3;
    poll:
    status = func_80080828();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
      }
      if (status & 2)
      {
        if (D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C = saved;
  }
  temp = (*idx_1494) & new_var;
  if (((temp == 2) || (temp == 5)) != 0)
  {
    *idx_1494 = 2;
    dst = a1;
    src = (u8 *) (&D_800F19A0);
    i = 7;
    if (a1 != 0)
    {
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
    }
    return temp;
  }
  if (a0 != 0)
  {
    return 0;
  }
  goto loop;
}
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_4  |  160i  |  x4 size collision */
extern volatile u8 *D_800A147C_2 asm("D_800A147C");
s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt + 1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 arg4;
    ;
    arg4 = tbl_125c[idx_1494[0]];
    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
  }
  cdrom_ClearIrq();
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;
  if (sys_GetVblankCount() != 0)
  {
    saved = (*D_800A147C_2) & 3;
    poll:
    status = func_80080828();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
        ;
      }
      if (status & 2)
      {
        if (D_800A11B4)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C_2 = saved;
  }
  {
    register s32 check asm("$6");
    check = *idx_1496;
    if (!check) goto check2;
    *idx_1496 = 0;
    src = (u8 *) (&D_800F19B0);
    if (a1 != 0)
    {
      dst = a1;
      i = 7;
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
      return check;
    }
    goto done;
    check2:
    check = *(idx_1496 - 1);
    if (check)
    {
      *(idx_1496 - 1) = 0;
      dst = a1;
      src = (u8 *) (&D_800F19A8);
      if (dst != 0)
      {
        i = 7;
        do
        {
          b = *src;
          src++;
          i--;
          *dst = b;
          dst++;
        }
        while (i != (-1));
      }
    }
    else
    {
      goto after_blocks;
    }
    done:
    return check;
    after_blocks:
    if (a0 != 0)
    {
      return 0;
    }
    goto loop;
  }
}
/* kengo:HIGH  |  nm_mario/marionation_Exec  |  180i  |  +1 near-exact */
s32 tslTm2LoadImage(s32 a0, void *a1, void *a2, s32 a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return 0;
}
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
extern volatile u16 *g_cd_spu_voice;
/* PsyQ 4.0 LIBCD bios.c v1.86: CD_initvol — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/bios.c */
s32 cdrom_ConfigSPU(void) {
    u8 vol[4];

    if (g_cd_spu_voice[0xDC] == 0 && g_cd_spu_voice[0xDD] == 0) {
        g_cd_spu_voice[0xC0] = 0x3FFF;
        g_cd_spu_voice[0xC1] = 0x3FFF;
    }

    g_cd_spu_voice[0xD8] = 0x3FFF;
    g_cd_spu_voice[0xD9] = 0x3FFF;
    g_cd_spu_voice[0xD5] = 0xC001;
    vol[0] = vol[2] = 0x80;
    vol[1] = vol[3] = 0;
    *g_cd_index_reg = 2;
    *g_cd_req_reg = vol[0];
    *g_cd_irq_reg = vol[1];
    *g_cd_index_reg = 3;
    *g_cd_param_fifo = vol[2];
    *g_cd_req_reg = vol[3];
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
void tslTm2LoadImage_2(void *a0) {
    (void)a0;
}
/* kengo:MED  |  tsl_tm2/tslTm2LoadImage_2  |  253i  |  -10 x2 size collision */
extern s32 D_800162EC;
extern s32 D_80016304;
extern s32 D_80082050;
extern s32 D_80082320;

/* PsyQ 4.0 LIBCD cdread.c module .data block — CD_ReadCallbackFunc followed
   by the volatile cdread state struct (SOTN psxsdk names it D_80032DBC); BB2
   links Sony's CDREAD object verbatim (census 2026-07-09), so
   D_800A14D0..D_800A1500 are one Sony data block (preceded by
   CD_ReadCallbackFunc at D_800A14CC), not separate globals. Member map
   recorded in memory/closer/sony-naming-map.md. */
typedef struct {
    /* 0x00 */ s32 sectors; /* D_800A14D0 */
    /* 0x04 */ s32 buf;     /* D_800A14D4 */
    /* 0x08 */ s32 p;       /* D_800A14D8 */
    /* 0x0C */ s32 mode;    /* D_800A14DC */
    /* 0x10 */ s32 size;    /* D_800A14E0 */
    /* 0x14 */ s32 cnt;     /* D_800A14E4 */
    /* 0x18 */ s32 t2;      /* D_800A14E8 */
    /* 0x1C */ s32 t1;      /* D_800A14EC */
    /* 0x20 */ s32 pos;     /* D_800A14F0 */
    /* 0x24 */ s32 cbsync;  /* D_800A14F4 */
    /* 0x28 */ s32 cbready; /* D_800A14F8 */
    /* 0x2C */ s32 cbdata;  /* D_800A14FC */
    /* 0x30 */ s32 tslmode; /* D_800A1500 */
} CdlREAD;
extern volatile CdlREAD D_800A14D0;

/* PsyQ 4.0 LIBCD cdread.c: cd_read_retry (static) — verbatim-linked Sony
   object (census 2026-07-09). Body below is the HEAD interim form (still
   INCOMPLETE, carries rules); the honest struct respell of this one is
   banked in memory/closer/candidates/ — see phase3-progress.md. These
   per-member externs are the HEAD-era declarations kept only for this
   function; they name the same Sony data block the CdlREAD struct spans. */
extern s32 D_800A1500;
extern s32 D_800A14EC;
extern s32 D_800A14E8;
extern s32 D_800A14E4;
extern s32 D_800A14DC;
extern s32 D_800A14D4;
extern s32 D_800A14D8;
extern s32 D_800A14F0;

s32 saEft00Add(s32 arg0) {
    u8 sp10;
    s32 temp_s0;

    cdrom_SetCallbackA(0);
    cdrom_SetCallbackB(0);
    if (*(volatile s32 *)&D_800A1500 & 1) {
        tslTmlGetHeda(0);
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
        tslTmlGetHeda((s32)&D_80082320);
    }
    D_800A14D8 = D_800A14D4;
    func_80080390(6, 0);
    D_800A14E4 = *(s32 *)&D_800A14D0.sectors; /* interim: HEAD parity needs a
        non-volatile read here; goes away with this function's honest close */
    D_800A14E8 = sys_VSync(-1);
end:
    return D_800A14E4;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadBreak — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp psxsdk shape + v1.86 hooks */
void saEft00Add_sub(void) {
    if (D_800A14D0.tslmode & 1) {
        Vu0SetLightColMatrix_80080684(0);
    }
    D_800A14D0.cnt = 0;
    cdrom_SetCallbackA(D_800A14D0.cbsync);
    cdrom_SetCallbackB(D_800A14D0.cbready);
    if (D_800A14D0.tslmode & 1) {
        tslTmlGetHeda(D_800A14D0.cbdata);
    }
    func_80080390(9, 0);
}

/* PsyQ 4.0 LIBCD cdread.c: CdRead — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
s32 func_800826CC(s32 sectors, s32 buf, s32 mode) {
    D_800A14D0.mode = mode;
    switch (D_800A14D0.mode & 0x30) {
        case 0:
            D_800A14D0.size = 0x200;
            break;
        case 0x20:
            D_800A14D0.size = 0x249;
            break;
        default:
            D_800A14D0.size = 0x246;
            break;
    }
    D_800A14D0.mode |= 0x20;
    D_800A14D0.buf = buf;
    D_800A14D0.sectors = sectors;
    D_800A14D0.cbsync = cdrom_SetCallbackA(0);
    D_800A14D0.cbready = cdrom_SetCallbackB(0);
    if (D_800A14D0.tslmode & 1) {
        D_800A14D0.cbdata = tslTmlGetHeda(0);
    }
    D_800A14D0.t1 = sys_VSync(-1);
    if (cdrom_GetMode() & 0xE0) {
        tslPolyF4Init(9, 0, 0);
    }
    return saEft00Add(0) > 0;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadSync — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
s32 func_800827D0(s32 mode, s32 result) {
    s32 var_s0;

    while (1) {
        var_s0 = -1;
        if (sys_VSync(-1) <= D_800A14D0.t1 + 1200) {
            if (D_800A14D0.cnt < 0 ||
                sys_VSync(-1) > D_800A14D0.t2 + 60) {
                saEft00Add(1);
                var_s0 = D_800A14D0.sectors;
            } else {
                var_s0 = D_800A14D0.cnt;
            }
        }
        if (mode != 0 || var_s0 <= 0) {
            Vu0SetLightColMatrix_80080208(1, (u8 *)result);
            return var_s0;
        }
    }
}
