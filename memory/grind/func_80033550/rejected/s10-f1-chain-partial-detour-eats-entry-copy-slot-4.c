/* s10 2026-08-25 — F1 chain-extender, mode C (partial detour: arg0 feeds loads
 * 0 and 1, 's32 *p = arg0 + 2;' feeds load 2).
 * THE ONLY SPELLING THAT EVER SPLIT THE POINTER INTO TWO GLOBAL ALLOCNOS AT 34
 * INSNS: 74=$v1(i), 72=$a0(arg0, nrefs 5, livelen 16), 79=$a1(detour, nrefs 2,
 * livelen 17). arg0's hard-conflict set LOSES 4 (it now keeps $a0) — the seat
 * moved, but the wrong way, and the second occupant is NOT byte-free: it
 * consumed the entry-copy slot ('addiu a1,a0,8' instead of 'addu a3,a0,zero').
 * sandbox = 4, 34/34. Proves the 5-occupant requirement costs an insn. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  s32 *p = arg0 + 2;
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
  w2 = p[0];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
