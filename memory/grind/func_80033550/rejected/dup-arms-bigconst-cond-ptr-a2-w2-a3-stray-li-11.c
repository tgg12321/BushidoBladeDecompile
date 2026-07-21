/* s7 REJECTED (honest sandbox 11, 36 insns): dupM — arms={sw3} with a
 * BIG-constant unsigned ptr compare ((u32)arg0 < 0x80200000U). The lui/li
 * constant temp + sltu result occupy v0-region seats during the tail
 * (local-alloc grabs v0 first), which pushes the locals UP to their TARGET
 * seats: w0->v1 CORRECT, w1->a0 CORRECT — and rotates ptr->a2, w2->a3,
 * idx->a1. First natural-cond form ever to move ptr off a1.
 * Cost: the branch+sltu delete post-merge but ONE stray constant li
 * survives (delete_computation stops at it), plus the 3-seat rotation
 * => 11. Sibling dupU (TWO big-const conds, 3 arms) reached ptr->a3 —
 * loads via $7! — with w0=v1, w1=a0 both correct, but idx->a1, w2->a2 and
 * ~5 residual cond bytes (partial 3-arm merge).
 * Mechanism (named passes): pre-RA the duplicated arm + cond pseudos are
 * REAL in global.c's conflict graph (byte-free conflict fuel); post-reload
 * jump_optimize cross_jump (jump2: .greg 21 insns -> .jump2 16) merges the
 * identical arms and jump.c delete_computation deletes the dead cond
 * computes. This is channel (f), MISSED by the s6 closure enumeration.
 * Dumps: tmp/grind/func_80033550/s7/out3/dupM*.{greg,sdiff},
 *        out4/dupU_sltu3_sw3.{greg,sdiff}
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
  if ((u32) arg0 < 0x80200000U) { /* FAKE: identical arms, jump2-merged */
    *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
  } else {
    *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
  }
}
