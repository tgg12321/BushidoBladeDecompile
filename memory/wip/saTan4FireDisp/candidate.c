/* saTan4FireDisp - WIP candidate. sandbox --disable all == 40 (HEAD == 41).
 * Reaches target's EXACT frame: 88 / vars=24 / regs=10 / args=24.
 * Cost: 141 insns vs target 135 (+6) - the `s16 r` conversion adds
 * sign-extends. Net -1 on the metric because the sp-offset cascade
 * collapses when the frame is right.
 *
 * Mechanism (measured): vars = 8 x (number of pseudos in greg's allocate
 * list that receive no hard register). HEAD has 1, this form has 3.
 *   slot 1: pre-existing (pseudo 80)
 *   slot 2: `s16 sid` naming the thrice-read *((s16 *)fp_ptr + 4) - FREE
 *   slot 3: `s16 r` - costs the +6
 * See notes.md; tmp/stf_orphan.py is the detector.
 */
void saTan4FireDisp(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s16 r;
  s32 g;
  s32 b;
  s32 outer;
  int new_var;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s16 sid;
  s32 idx;
  s16 rect[4];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  sid = *(((s16 *) fp_ptr) + 4);
  if (sid != D_800A9A20) { return; }
  if (D_80094E08[sid] == 0xFF) { return; }
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
  tbl = (s16 *) D_80094DF0[D_80094E08[sid]];
  idx = 0;
  goto inner_check;
  inner_body:
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
  r = tbl[0];
  inner_check:
  if (r >= 0) { goto inner_body; }
  outer++;
  if (outer < 2) { goto loop_outer; }
  if (single_game_SetStageId() == 1) { func_8003E120(); }
}
