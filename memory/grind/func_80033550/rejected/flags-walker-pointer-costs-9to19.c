/* s3 KILLED family — any LIVE pointer variable in the loop region emits
 * bytes: target's loop forms addresses via as-macros (lui $at/addu/lbu),
 * so a materialized base/walker pointer always diverges.
 * Measured: flags[] used loop+store = 19; flags[] loop-only = 10;
 * walker (*p++ + index hybrid) = 9; store via per-iter addr temp *p=1 = 11.
 * These DO inject a pseudo conflicting with ptr (75 in .greg) but never
 * byte-free. Representative body (L7, score 10). */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  u8 *flags;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  flags = &D_800A3918;
  i = 0;
  loop:;
  if (flags[i] == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
