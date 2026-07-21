/* REJECTED s6: wider-signature theory, measured dead.
 * ANSI 4-param (below), 2-param, and K&R-style 4-param variants all compile
 * to BYTE-IDENTICAL asm with identical .greg census (72 conflicts {i,2,3,4,29})
 * and ptr still in a1. Unused parm copies are flow-deleted pre-RA with zero
 * residual entry liveness for a1-a3 — extra unused params cannot pressure RA.
 * Dumps: tmp/grind/func_80033550/s6/{p4,p2,pk}.c.greg + .s */
void func_80033550(s32 *arg0, s32 u1, s32 u2, s32 u3)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
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
