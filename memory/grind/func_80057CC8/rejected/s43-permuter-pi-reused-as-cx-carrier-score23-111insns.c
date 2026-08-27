/* s43 REJECTED -- the one SEMANTICS-PRESERVING permuter find of the s43 campaigns
 * (permuter score 738 vs base 758, from the s42 sibling-idiom chassis a1.c).  It reuses
 * the existing local `pi` to carry `cx` ((s16)(pi = cx) == (s16)cx; *arg2 = pi == cx).
 * MEASURED: sandbox 23 @ 111 insns -- the FIRST Regime-A form at the target instruction
 * count, but the seat map degrades: the reused `pi` becomes a new call-crossing allocno
 * (pseudo 85, 5 refs / 44 insns) that takes $s2, pushing the next-address (88) up to $s1
 * and arg0 (72) down to $s3.  Worse than the score-16 candidate on every seat.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3)
{
  unsigned short prev_idx;
  s32 ang_prev;
  s32 ang_next;
  s32 ang_mid;
  s32 scale;
  s32 base;
  s32 half;
  u16 cx;
  s16 new_var;
  s32 pi;
  u16 cy;
  s32 vt;
  s16 *p;
  prev_idx = arg1 - 1;
  vt = *((s32 *) (arg0 + 4));
  cx = *((u16 *) ((vt + (arg1 * 4)) + 0));
  cy = *((u16 *) ((vt + (arg1 * 4)) + 2));
  if (((s16) prev_idx) < 0)
  {
    prev_idx = arg0[3] - 1;
  }
  {
    s32 tmp = arg1 + 1;
    s32 off = tmp * 4;
    if (((s16) tmp) >= ((s32) arg0[3]))
    {
      off = 0;
    }
    p = (s16 *) (off + vt);
  }
  pi = (s16) prev_idx;
  {
    s16 *q = (s16 *) ((pi * 4) + vt);
    ang_prev = ratan2(q[0] - ((s16) (pi = cx)), q[1] - ((s16) cy)) & 0xFFF;
  }
  ang_next = ratan2(p[0] - ((s16) cx), p[1] - ((s16) cy)) & 0xFFF;
  if (ang_next < ang_prev)
  {
    base = ang_prev + 0x800;
    half = ((s32) (ang_prev - ang_next)) / 2;
    ang_mid = base - half;
  }
  else
  {
    ang_mid = (((s32) (ang_next - ang_prev)) / 2) + ang_prev;
  }
  scale = arg0[2] * 40;
  p = &Judge;
  *arg2 = pi + ((scale * ((s32) (*(p + (ang_mid & 0xFFF))))) >> 12);
  *arg3 = cy + ((scale * ((s32) (new_var = *(p + ((((s16) ang_mid) + 0x400) & 0xFFF))))) >> 12);
}
