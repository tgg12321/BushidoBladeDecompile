/* func_80033550 — best honest form, sessions 1-2 (floor 4, zero cheat-asm).
 * s2 (structural, 24 variants): floor confirmed 4; this body is applied in
 * src/. See evidence.md s2 for the conflict-injection findings (v07 a1→a2).
 * The do-while(0) wrap is sanctioned per do-while-zero-exception (2026-07-06
 * owner ruling, any codegen effect, single-level, FAKE-annotated).
 * Residual 4 = arg0's pointer pseudo homed in $a1 (build) vs $a3 (target):
 * move + 3 lw base regs. See evidence.md for the RTL conflict analysis. */
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
  /* FAKE: single-level wrap; loop notes fence sched1 so the final sll of
   * idx stays ahead of the three lw's, matching target order. */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
