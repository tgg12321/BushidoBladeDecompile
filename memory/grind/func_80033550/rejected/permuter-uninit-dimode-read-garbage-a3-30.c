/* s5 KILLED (semantically invalid) — c5 DImode-basin best structural find
 * (output-30-1, permuter score 30 = 6 reg diffs, ZERO ins/del). Mutation
 * moved `t = i;` BELOW the `new_var = (s32)t * 12;` read, so the multiply
 * reads UNINITIALIZED t: the emitted idx chain computes from garbage
 * (build: sll v0,a3,1 / addu v0,v0,a3 where a3 = t's uninit low half;
 * target computes from v1 = i). ptr did NOT flip (build loads still via
 * a1; the move is a1,a0 vs target a3,a0). Not shippable: behavior differs.
 *
 * EVIDENTIAL VALUE (first ever): the uninit-read pseudo survived to RA
 * BYTE-FREE and occupied a real register (a3) — the def is gone, the
 * upward-exposed use keeps the pseudo in the conflict graph with zero
 * emitted instructions. This is the first zero-byte occupant observed on
 * this function; the closing requirement (s2) needs TWO of these seated
 * in a1+a2 overlapping ptr — but any semantically VALID spelling must
 * initialize the variable, which re-emits bytes. See s5 evidence. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s64 t;
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
  new_var = (s32) t * 12;   /* reads t BEFORE its def — UB, garbage idx */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  t = i;                    /* def after use; emits nothing */
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
