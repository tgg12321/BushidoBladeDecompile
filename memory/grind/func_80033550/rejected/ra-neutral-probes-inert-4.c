/* REJECTED s2 (all byte-identical to base, score 4): three RA-bias probes
 * proven inert on this function:
 *   1. per-load do-while wraps promoting w0/w1/w2 to block-crossing
 *      pseudos (global RA instead of local-alloc) — identical output;
 *   2. found_idx = i split at the found label — copy ties to v1 and is
 *      no-op-deleted; uncoalesced copies cannot add conflicts byte-free;
 *   3. constant-holder `one = 1` for the sb immediate — identical output.
 * Also killed: wrap-content/load-order geometry (w2-in-wrap, w2-before-w1,
 * w0+w1-in-wrap, idx-only-wrap: all 4 with ptr=a1; w2-before-wrap /
 * early-stage: 5, kills the no-copy form), decl order, u32 idx typing,
 * i-borrow double stage (14), stage-w0-in-wrap (19), wider-signature
 * theory (single caller sets only a0). This file archives probe 1. */
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
  do { w1 = arg0[1]; } while (0);
  do { w2 = arg0[2]; } while (0);
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
