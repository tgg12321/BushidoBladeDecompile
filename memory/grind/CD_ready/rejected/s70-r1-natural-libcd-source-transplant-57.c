/*
s70 REJECTED (score 57, build 174 vs target 179). REDERIVE probe: a straight structural
transplant of the ORIGINAL PsyQ libcd bios.c source shape for CD_ready(int mode,
u_char *result) (tmp/closer/sotn_bios.c:260-286 plus the inlined set_alarm :95,
get_alarm :102 and callback :210 bodies), mapped onto BB2 symbols. while(1) loop,
short-circuit || timeout test, direct global accesses, natural array subscripts, zero FAKE
constructs, zero gotos. DEAD because it is 5 instructions SHORT of the target's 179 and
57 masked: the direct D_800A1494/95/96 accesses cost the single-base addressing (the same
effect s60 measured for the volatile-global respelling).
*/
/* s70 R1 — REDERIVE. Structural transplant of the ORIGINAL PsyQ libcd `bios.c` source shape for
 * CD_ready(int a0, u_char *a1), taken from tmp/closer/sotn_bios.c:260-286 with the inlined
 * set_alarm (:95), get_alarm (:102) and callback (:210) bodies, and mapped onto BB2 symbols.
 * ZERO FAKE constructs, no gotos, no staged temporaries, no pointer aliases: `while (1)` loop,
 * short-circuit `||` get_alarm test, natural array subscripts for every printf argument.
 * This is NOT a descendant of the vT40/vAT1 basin every prior session has ground.
 */
s32 marionation_Exec(s32 a0, u8 *a1)
{
  extern volatile u8 D_800A1496;
  s32 c;
  s32 ready;
  s32 interrupt;
  s32 i;
  u8 *src;
  u8 *dst;
  u8 saved;

  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;

  while (1)
  {
    if (D_800F19B8 < sys_VSync(-1) || D_800F19BC++ > 0x3C0000)
    {
      tslTm2LoadImage_2(&D_800161B8);
      debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5],
                   D_800A125C[D_800A1494], D_800A125C[D_800A1495]);
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
          ((void (*)(u8, void *)) D_800A11B8)(D_800A1495, &D_800F19A8);
        }
        if ((interrupt & 2) && D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(D_800A1494, &D_800F19A0);
        }
      }
      *D_800A147C_2 = saved;
    }
    c = D_800A1496;
    if (c != 0)
    {
      D_800A1496 = 0;
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
    ready = D_800A1495;
    if (ready != 0)
    {
      D_800A1495 = 0;
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
