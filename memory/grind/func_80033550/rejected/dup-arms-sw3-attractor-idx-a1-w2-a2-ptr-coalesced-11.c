/* s7 REJECTED (honest sandbox 11): duplicated-arms family attractor.
 * Arms={sw3} with ANY opaque cond (arg0!=0, (u32)arg0<smallN sltiu, shadow
 * copies idx2/w2b, 2 or 3 arms — SIX spellings measured, ALL identical
 * census) globalizes idx+w2 and lands the rigid seat-rotation:
 *   idx->a1, w2->a2, ptr->a0 (entry move COALESCED away, 33 insns),
 *   w0->v0, w1->v1 (locals slide down into the vacated seats).
 * The cond compute + branch are FULLY deleted post-RA (sltiu present in
 * .greg, absent in final asm — jump2 cross-jump merges the identical arms,
 * jump.c delete_computation kills the dead compare). Byte-clean merge, but
 * every seat except i is wrong => sandbox 11 (1 deletion + reg diffs).
 * Musical chairs: occupying a1+a2 vacates a0, so ptr coalesces into a0 and
 * the entry copy (which target HAS, into a3) disappears.
 * Dumps: tmp/grind/func_80033550/s7/out5/dupX1_sltiu_sw3.{greg,jump2,sdiff}
 */
void func_80033550(s32 *arg0)
{
  s32 new_var; s32 i; s32 w0; s32 w1; s32 w2; s32 idx;
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
  if ((u32) arg0 < 2U) { /* FAKE: identical arms, merged by jump2 cross-jump */
    *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
  } else {
    *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
  }
}
