
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
asm(".include \"include/macro.inc\"\n");;
extern u8 g_file_disc_type;
extern u16 g_file_vram_timer;
extern void irq_DisableInterrupts(void);
extern void cdrom_ClearIrq(void);
extern s32 cpu_side_move_dir_4(s32, u8 *);
extern void marionation_Exec(void);
extern s32 cdrom_SendCmd();
extern s32 cdrom_DmaToRam();
extern s32 cdrom_DmaChain();
extern s32 func_80080660_ret(s32) asm("tslTmlGetHeda");
extern void irq_AcknowledgeVblank(s32, s32);
extern s32 saEft00Add_ret(s32) asm("saEft00Add");
extern s32 saEft01Init(s32);
extern u8 g_cd_mode;
extern u8 g_cd_param;
extern u8 g_cd_ready_flag;
extern u8 g_cd_ready_flag2;
extern s32 g_cd_callback_a;
extern s32 g_cd_callback_b;
asm(".set noreorder\n.set noat\nglabel bios_DeliverEvent\n    addiu $t2, $zero, 0xB0\n    jr    $t2\n    addiu $t1, $zero, 0x7\n    nop\n.set reorder\n.set at\n");;
u32 cdrom_GetMode(void);
u32 cdrom_GetReadyFlag(void);
u32 cdrom_GetReadyFlag2(void);
void *func_800800CC(void);
extern void cdrom_Shutdown(void);
extern s32 cdrom_Initialize(void);
extern s32 cdrom_ConfigSPU(void);
s32 cdrom_CheckReady(s32 a0);
void func_80080148(void);
extern s32 g_cd_debug_level;
extern s32 g_cd_cmd_table[];
extern s32 g_cd_result_table[];
extern char g_str_none;
s32 cdrom_SetDebugLevel(s32 a0);
void *cdrom_GetCmdName(u32 a0);
void *cdrom_GetResultName(u32 a0);
void Vu0SetLightColMatrix_800801E8(void);
void Vu0SetLightColMatrix_80080208(void);
s32 cdrom_SetCallbackA(s32 a0);
s32 cdrom_SetCallbackB(s32 a0);
extern s32 g_cd_sector_buf[];
extern s32 tslTm2LoadImage(s32, void *, void *, s32);
s32 func_80080258(s32 a0, s32 a1, s32 a2);
s32 func_80080390(s32 a0, s32 a1);
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2);
s32 func_80080600(void);
s32 func_80080620(void);
s32 Vu0SetLightColMatrix_80080640(void);
void tslTmlGetHeda(s32 a0);
void Vu0SetLightColMatrix_80080684(s32 a0);
void cdrom_FramesToBcd(s32 frames, u8 *out);
extern s32 g_cd_sector_buf[];
extern s32 tslTm2LoadImage(s32, void *, void *, s32);
extern s32 func_80080DB0_ret(s32, void *);
s32 cdrom_BcdToFrames(u8 *a0);
s32 func_80080828(void);
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
s32 cpu_side_move_dir_4(s32 a0, u8 *a1);
extern volatile u8 *D_800A147C_2 asm("D_800A147C");
s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  int new_var;
  int new_var3;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  u8 *dst2;
  s32 i;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  if ((D_800F19B8 < sys_VSync(-1)) || ((cnt = D_800F19BC, D_800F19BC = cnt + 1, 0x3C0000 < cnt)))
  {
    goto do_timeout;
  }

  goto success;
  do_timeout:
  do
  {
    tslTm2LoadImage_2(&D_800161B8);
    {
      s32 arg5;
      s32 t0;
      void **pp;
      t0 = idx_1494[0];
      pp = (void **) (&D_800F19C0);
      t0 *= 4;
      t0 = (s32) (((u8 *) tbl_125c) + t0);
      v0 = idx_1494[1];
      v0 <<= 2;
      arg5 = *((s32 *) (v0 + ((s32) tbl_125c)));
      debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *((s32 *) t0), arg5);
    }
    cdrom_ClearIrq();
  }
  while (0);

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
  new_var3 = 0xFF;
  do
  {
    if (sys_GetVblankCount() != 0)
    {
      saved = (*D_800A147C_2) & 3;
      do
      {
        status = func_80080828();
        if (status == 0)
        {
          break;
        }
        {
          if (status & 4)
          {
            if (D_800A11B8 != 0)
            {
              ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
            }
            ;
            *(idx_1496 - 1) = 0;
          }
          if (status & 2)
          {
            if (D_800A11B4)
            {
              ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
            }
          }
        }
      }
      while (1);
      *D_800A147C_2 = saved;
    }
  }
  while (0);
  {
    s32 check;
    check = (*idx_1496) & new_var;
    if (!check)
    {
      goto check2;
    }
    do
    {
      do
      {
        *idx_1496 = 0;
      }
      while (0);
    }
    while (0);
    src = (u8 *) (&D_800F19B0);
    dst = a1;
    if (a1 != 0)
    {
      i = 7;
      do
      {
        u8 bb;
        bb = *src;
        src++;
        i--;
        *dst = bb;
        dst++;
      }
      while (i != (-1));
      return check;
    }
    goto done;
    check2:
    check = (*(idx_1496 - 1)) & new_var3;

    if (!check)
    {
      goto tail;
    }
    do
    {
    }
    while (0);
    dst2 = a1;
    src = (u8 *) (&D_800F19A8);
    i = 7;
    if (dst2 != 0)
    {
      do
      {
        u8 bb;
        bb = *src;
        src++;
        i--;
        *dst2 = bb;
        dst2++;
      }
      while (i != (-1));
    }
    done:
    return check;

    do
    {
      tail:
      if (a0 == 0)
      {
        goto loop;
      }

    }
    while (0);
    return 0;
  }
}

s32 tslTm2LoadImage(s32 a0, void *a1, void *a2, s32 a3);
extern volatile u8 *g_cd_index_reg;
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_irq_reg;
extern volatile u8 *g_cd_param_fifo;
s32 cdrom_SendCmd(u8 *a0);
extern volatile u32 *g_cd_dma_madr;
extern volatile u32 *g_cd_dma_ctrl;
extern volatile u8 g_cd_status_a;
extern volatile u8 g_cd_status_b;
extern volatile u8 g_cd_status_c;
void cdrom_ClearIrq(void);
extern volatile u16 * volatile g_cd_spu_voice;
s32 cdrom_ConfigSPU(void);
extern s32 g_cd_init_flag;
extern void irq_EnableInterrupts(s32, void *);
extern u8 D_80081F1C;
void cdrom_Shutdown(void);
extern void D_800162A8;
extern void D_800162B4;
extern void D_800A1498;
s32 cdrom_Initialize(void);
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
s32 saEft01Init(s32 a0);
extern volatile u32 *g_cd_dma_madr;
extern volatile u32 *g_cd_dma_bcr;
extern volatile u32 *g_cd_dma_ctrl_b4;
extern volatile u32 *g_cd_dma_dest;
extern volatile u32 *g_cd_dma_size;
extern volatile u32 *g_cd_dma_ctrl;
s32 cdrom_DmaToRam(s32 a0, s32 a1);
s32 cdrom_DmaChain(s32 a0, s32 a1);
extern s32 D_800A1460;
void cdrom_SetErrorCallback(s32 a0);
extern volatile u8 g_cd_status_a;
extern volatile u8 g_cd_status_b;
extern s32 g_cd_callback_a;
extern s32 g_cd_callback_b;
extern void D_800F19A8;
extern void D_800F19A0;
extern s32 func_80080828(void);
asm(".set noreorder\n.set noat\nglabel D_80081F1C\n.set reorder\n.set at\n");;
void cdrom_IrqHandler(void);
void tslTm2LoadImage_2(void *a0);
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
s32 saEft00Add(s32 arg0);
void saEft00Add_sub(void);
s32 func_800826CC(s32 arg0, s32 arg1, s32 arg2);
s32 func_800827D0(s32 a0, s32 a1);
