/* s10 REJECTED (exactly inert, 13 at 135 on the while basin without `one`).
 * `if (func_8003E2A0() - 1 == 0)`.  One of SEVEN ordinary-C spellings of the
 * trailing `== 1` test measured this session, all exactly 13/135: subtract-and-
 * compare-zero (this), xor-and-compare-zero, `!(x != 1)`, `switch/case 1`,
 * naming the call RESULT in a reused `idx`, naming it in a reused `outer`, and
 * (s4) block-local rc / function-scope rc / Yoda.  Ten spellings total.  The
 * divergence is reload's spill-register choice for the rematerialised literal 1
 * ($v1 vs target's $t0); the SHAPE of the test is not the lever. */
void func_80041BF4(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  s16 rect[4];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
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
  while (tbl[0] >= 0)
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = (*((u16 *) tbl)) + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() - 1 == 0) { func_8003E120(); }
}
