/* REJECTED (measured 5, s1): plain form without the do-while wrap.
 * sched1 hoists the three lw's above the final `sll v0,v0,2` of the idx
 * chain (load-latency priority), adding a 1-position reordering diff on
 * top of the 4-insn a1/a3 pointer-home diff. */
void func_80033550(s32 *arg0)
{
  s32 i; s32 w0; s32 w1; s32 w2; s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  idx = i * 12;
  w0 = arg0[0];
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
