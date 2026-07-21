/* REJECTED s2 (score 7): staging w2 through dead new_var DOES flip the
 * pointer home a1->a2 (.greg: 72 conflicts {v0,a0,a1}+74) — the only form
 * ever to move it — but the staged global pseudo grabs v1 and local-alloc
 * rotates the w-homes to w0=a0, w1=a1, w2=v1 (target needs v1,a0,a1),
 * costing 3 home diffs + the sw group displacement. All staged placements
 * measured: after-w1 (7), mid (6), in-wrap (19), pre-idx (5), stage-w1 (6).
 * Mechanism datum: conflict injection works; home preservation does not
 * coexist with it — the w's are the only available a1/a2 occupants. */
void func_80033550(s32 *arg0)
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
  new_var = arg0[2];
  w2 = new_var;
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
