/* s7 REJECTED (inert, not wrong): representative of the dead-store /
 * named-local FAKE-family sweep — ALL FIVE variants byte-identical to the
 * floor-4 candidate (ds1 fake=i tail; ds2 fake=arg0[0] entry; ds3 two dead
 * stores; nl1 k0/k1/k2 keep-copies; nl2 slot=i live copy).
 * Forensic pass-naming: the dead-store pseudo (79) exists in .rtl and is
 * ALREADY GONE in .jump — jump_optimize pass 1 deletes it BEFORE cse/flow/RA
 * (earlier than s6's flow.c:1479 prediction; same conclusion: the pseudo
 * never reaches conflict construction, so it cannot occupy a1/a2).
 * Named-local copies coalesce in cse (census identical: 2 allocnos).
 * Dumps: tmp/grind/func_80033550/s7/out/{ds1,ds2,ds3,nl1,nl2}*.{greg,flow,cse}
 */
void func_80033550(s32 *arg0)
{
  s32 new_var; s32 i; s32 w0; s32 w1; s32 w2; s32 idx; s32 fake;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  fake = i; /* FAKE: dead store — deleted by jump1, RA-invisible, INERT */
  new_var = i * 12;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
