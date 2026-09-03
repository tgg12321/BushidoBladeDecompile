/*
s70 REJECTED (score 55, build 178 vs target 179). REDERIVE probe: R1's original-libcd control
flow with the Intr triple restored to single-base pointer addressing (idx_1494 / +1 / +2,
1496 volatile). Still 55 masked and one instruction short: the natural while(1)+||
control flow is a different basin from the vAT1 goto chassis, not a perturbation of it.
*/
/* s70 R2 — REDERIVE. Same original-libcd control flow as R1 (while(1) + short-circuit `||`
 * get_alarm + natural subscripts), but retaining the ONE addressing fact the target's bytes
 * demand and R1 cannot express: the Intr triple is reached through a single base pointer
 * (idx_1494 / +1 / +2), with the 1496 poll access volatile (s60: stripping the qualifier costs
 * the target's 179th instruction). No do-while(0) wraps, no staged temporaries, no pp alias,
 * no constant-holder masks — i.e. every FAKE construct in candidate.c is absent.
 */
s32 marionation_Exec(s32 a0, u8 *a1)
{
  u8 *idx_1494;
  u8 *idx_1495;
  volatile u8 *idx_1496;
  s32 *tbl_125c;
  s32 c;
  s32 ready;
  s32 interrupt;
  s32 i;
  u8 *src;
  u8 *dst;
  u8 saved;

  tbl_125c = D_800A125C;
  idx_1494 = (u8 *)&D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;

  while (1)
  {
    if (D_800F19B8 < sys_VSync(-1) || D_800F19BC++ > 0x3C0000)
    {
      tslTm2LoadImage_2(&D_800161B8);
      debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5],
                   tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
      cdrom_ClearIrq();
      return -1;
    }
    if (sys_GetVblankCount() != 0)
    {
      saved = *D_800A147C_2 & 3;
      while (1)
      {
        interrupt = func_80080828();
        if (interrupt == 0) break;
        if ((interrupt & 4) && D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
        if ((interrupt & 2) && D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      *D_800A147C_2 = saved;
    }
    c = *idx_1496 & 0xFF;
    if (c != 0)
    {
      *idx_1496 = 0;
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
      }
      return c;
    }
    ready = *(idx_1496 - 1) & 0xFF;
    if (ready != 0)
    {
      *(idx_1496 - 1) = 0;
      src = (u8 *) (&D_800F19A8);
      dst = a1;
      i = 7;
      if (dst != 0)
      {
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
      }
      return ready;
    }
    if (a0 != 0)
    {
      return 0;
    }
  }
}
