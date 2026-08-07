/* saTan4FireDisp - WIP candidate. sandbox --disable all == 29 (HEAD == 41).
 * Frame AND instruction count are both EXACTLY target's:
 *   frame 88 / vars 24 / regs 10 / args 24, 135 insns.
 * Residual 29 = a 3-cycle callee-save rotation (ours s2/s3/s4 vs target
 * s3/s4/s2) plus the scheduling that rides on it.
 *
 * THE STRUCTURAL FIX (round 7): target keeps the RED CHANNEL and the LOOP
 * SENTINEL in DIFFERENT registers - the body's colour args come from
 * $s7/$s6/$s3 while the sentinel lives in $v0, and at the loop label target
 * reads tbl[0] TWICE (`lh $v0,0($s0)` for the test, `lhu $v1,0($s0)` for the
 * next rect[0]). The previous m2c-shaped body conflated them into one `r`,
 * which put the label in the wrong place - that is what the two asmfix
 * rules were compensating for. Giving the sentinel its own variable and
 * writing the loop as `while ((sent = tbl[0]) >= 0)` puts the read at the
 * label exactly as target does, supplies the third phantom slot, and keeps
 * the instruction count at 135.
 *
 * NB this is also a SEMANTIC correction: the old body passed `r` to
 * func_80048A7C, so from the second iteration onward it passed the previous
 * tbl[0] instead of the red channel. Target always passes $s7 (red).
 *
 * If this lands, the two asmfix rules for this function should become
 * unnecessary - verify at the completion gate.
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
  loop_outer:
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    xoff = 0x80;
    do { yoff = 0; } while (0);
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
    gpu_LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    gpu_DrawSync(0);
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  if (outer < 2) { goto loop_outer; }
  if (single_game_SetStageId() == 1) { func_8003E120(); }
}
