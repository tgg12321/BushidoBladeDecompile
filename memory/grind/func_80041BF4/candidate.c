/* saTan4FireDisp — GRIND candidate, s1 (2026-08-19).
 * sandbox --disable all == 22 (prior banked floor 29, HEAD form 41).
 * frame 88 / 135 insns, both exactly target's.
 *
 * THE ROTATION IS SOLVED. This form produces target's ENTIRE callee-save
 * allocation: a2 home->s2 (reused by outer), b->s3, yoff->s4, xoff->s5,
 * g->s6, r->s7, tbl->s0 (shares a0's home), idx->s1 (shares a1's home),
 * fp_ptr->fp.  The two changes vs the floor-29 form:
 *   1. outer loop written as a REAL `do { ... } while (outer < 2);`
 *      (was goto loop_outer). The loop notes make flow.c count in-loop
 *      refs at depth 2 / inner-loop refs at depth 3 (REG_N_REFS +=
 *      loop_depth), which lifts outer's allocno priority above b's:
 *      global.c priority = floor_log2(refs)*refs/live_length*10000.
 *      Measured order: outer 5192 > b 3870 > yoff(now) > xoff — target's
 *      exact allocation order.
 *   2. The `do { yoff = 0; } while (0);` wrap in the else arm REMOVED.
 *      Under the real outer loop that wrap put yoff's else-def at depth 3,
 *      inflating yoff to ~5853 (> outer) and flipping the rotation the
 *      wrong way — that is why the earlier do-while-outer attempt scored
 *      34 with permutation yoff > outer > b. Without the wrap yoff sits
 *      below b, exactly where target needs it.
 *
 * Residual 22 (all NON-rotation; see evidence.md s1 for the full diff):
 *   a. D_80094DF0 table load inside the loop: ours materializes the full
 *      symbol address in $t0 (lui+addiu) then `lw s0,0(v0+t0)`; target
 *      indexes through %hi/%lo (`lui at,%hi; addu at,at,v0; lw s0,%lo(at)`).
 *      Sibling func_80041AC8 (same TU, same expression, matched) emits the
 *      target form OUTSIDE a loop -> the divergence is loop-context
 *      (loop.c/CSE treatment of the invariant symbol address), not
 *      expression spelling.
 *   b. LoadImage arg setup (addiu a0,sp,24 + D_800A9A24 address) scheduled
 *      AFTER the lhu tbl[1] read; target has it BEFORE. Likely cascades
 *      from (a) since the a1 arg is the D_800A9A24 chain.
 *   c. v0/v1/t0 caller-save renames riding on (a)/(b): idx<<5 in a1 vs v0,
 *      the ==1 compare constant in v1 vs t0.
 */
void saTan4FireDisp(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  int new_var;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  s32 sent;
  s16 rect[4];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  new_var = 5;
  r = (a0 << 12) / 255;
  g = (a1 << 12) / 255;
  b = (a2 << 12) / 255;
  if (func_800486FC()) {
    b = func_8004881C(r, g, b);
    g = b;
    r = b;
  }
  outer = 0;
  do {
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    xoff = 0x80;
    yoff = 0;
  }
  tbl = (s16 *) D_80094DF0[D_80094E08[*(((s16 *) fp_ptr) + 4)]];
  idx = 0;
  while ((sent = tbl[0]) >= 0)
  {
    s32 off = idx << new_var;
    idx++;
    rect[0] = (*((u16 *) tbl)) + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    tbl += 2;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == 1) { func_8003E120(); }
}
