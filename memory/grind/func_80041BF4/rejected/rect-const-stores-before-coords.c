/* REJECTED [s2 2026-08-19] - rect[2]/rect[3] constant stores moved BEFORE
 * the two coordinate stores inside the inner loop.
 * sandbox --disable all == 46 (base form 17) and 139 build insns vs target's
 * 135: hoisting the two constant stores above the coordinate computation
 * lengthens the constants' live ranges across the coordinate arithmetic, so
 * reload can no longer rematerialize them into the same scratch and emits
 * four extra instructions.  The rect[] store order in target is
 * rect[0], rect[1], rect[2], rect[3] and it is load-bearing.
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
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  while ((sent = tbl[0]) >= 0)
  {
    s32 off = idx << new_var;
    idx++;
    rect[2] = 0x10;
    rect[3] = 1;
    rect[0] = (*((u16 *) tbl)) + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    tbl += 2;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == 1) { func_8003E120(); }
}
