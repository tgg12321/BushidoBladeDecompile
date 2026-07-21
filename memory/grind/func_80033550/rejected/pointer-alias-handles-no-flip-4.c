/* REJECTED s2 (score 4, no movement): second/third pointer handles to arg0
 * cannot create a1/a2 conflicts byte-free. Measured family: alias for w2
 * only, w1+w2, all three, block-local, late-init, two handles, first-two
 * (this file) — every variant either ties the extra handle to incoming a0
 * (copy no-op-deleted) or the handle simply BECOMES the single a1-homed
 * copy. The one emitted move is always the ptr copy itself; ptr stays a1.
 * Overlapping un-tied handles emit a second move (v02: 13). */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  s32 *p = arg0;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  do { idx = new_var; w0 = p[0]; } while (0);
  w1 = p[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
